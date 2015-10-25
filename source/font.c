/***
The `font` module
@module ctr.gfx.font
@usage local font = require("ctr.gfx.font")
*/

#include <unistd.h>
#include <stdlib.h>

#include <sftd.h>
#include "vera_ttf.h"

#include <lua.h>
#include <lauxlib.h>

#include "font.h"

/***
Load a TTF font.
@function load
@tparam string path path to the file
@treturn font the loaded font.
*/
static int font_load(lua_State *L) {
	const char *path = luaL_checkstring(L, 1);

	font_userdata *font = lua_newuserdata(L, sizeof(*font));
	luaL_getmetatable(L, "LFont");
	lua_setmetatable(L, -2);

	font->font = sftd_load_font_file(path);

	// SFTD doesn't actually check if the file exist, so we have to do this ourselves.
	if (font->font == NULL || access(path, F_OK) != 0) {
		lua_pushnil(L);
		lua_pushfstring(L, "No valid font file at %s", path);
		return 2;
	}

	return 1;
}

/***
Set a font as the default one.
@function setDefault
@tparam font font the font to set as the default one.
*/
static int font_setDefault(lua_State *L) {
	if (luaL_testudata(L, 1, "LFont") == NULL) {
		font_userdata *font = lua_newuserdata(L, sizeof(*font));
		luaL_getmetatable(L, "LFont");
		lua_setmetatable(L, -2);

		font->font = sftd_load_font_mem(vera_ttf, vera_ttf_size);
	}

	lua_setfield(L, LUA_REGISTRYINDEX, "LFontDefault");

	return 0;
}

/***
Return the default font.
@function getDefault
@treturn font default font
*/
static int font_getDefault(lua_State *L) {
	lua_getfield(L, LUA_REGISTRYINDEX, "LFontDefault");

	return 1;
}

/***
font object
@section Methods
*/

/***
Return the width of a string with a font.
@function :width
@tparam string text the text to test
@treturn number the width of the text (in pixels)
*/
static int font_object_width(lua_State *L) {
	font_userdata *font = luaL_checkudata(L, 1, "LFont");
	if (font->font == NULL) luaL_error(L, "The font object was unloaded");

	size_t len;
	const char *text = luaL_checklstring(L, 2, &len);

	int size = luaL_optinteger(L, 3, 9);

	// Wide caracters support. (wchar = UTF32 on 3DS.)
	wchar_t wtext[len+1];
	len = mbstowcs(wtext, text, len);
	*(wtext+len) = 0x0; // text end

	lua_pushinteger(L, sftd_width_wtext(font->font, size, wtext));

	return 1;
}

/***
Unload a font.
@function :unload
*/
static int font_object_unload(lua_State *L) {
	font_userdata *font = luaL_checkudata(L, 1, "LFont");
	if (font->font == NULL) return 0;

	sftd_free_font(font->font);
	font->font = NULL;

	return 0;
}

// Font object methods
static const struct luaL_Reg font_object_methods[] = {
	{ "width",  font_object_width  },
	{ "unload", font_object_unload },
	{ "__gc",   font_object_unload },
	{ NULL, NULL }
};

// Library functions
static const struct luaL_Reg font_lib[] = {
	{ "load",       font_load       },
	{ "setDefault", font_setDefault },
	{ "getDefault", font_getDefault },
	{ NULL, NULL }
};

int luaopen_font_lib(lua_State *L) {
	luaL_newmetatable(L, "LFont");
	lua_pushvalue(L, -1);
	lua_setfield(L, -2, "__index");
	luaL_setfuncs(L, font_object_methods, 0);

	luaL_newlib(L, font_lib);

	return 1;
}

void load_font_lib(lua_State *L) {
	// Load default font
	font_userdata *font = lua_newuserdata(L, sizeof(*font));
	luaL_getmetatable(L, "LFont");
	lua_setmetatable(L, -2);

	font->font = sftd_load_font_mem(vera_ttf, vera_ttf_size);

	lua_setfield(L, LUA_REGISTRYINDEX, "LFontDefault");

	// Load lib
	luaL_requiref(L, "ctr.gfx.font", luaopen_font_lib, false);
}

void unload_font_lib(lua_State *L) {
	lua_getfield(L, LUA_REGISTRYINDEX, "LFontDefault");

	if (luaL_testudata(L, -1, "LFont") != NULL)
		sftd_free_font(((font_userdata *)lua_touserdata(L, -1))->font); // Unload current font
}
