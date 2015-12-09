/***
The `gfx.color` module
@module ctr.gfx.color
@usage local color = require("ctr.gfx.color")
*/
#include <sf2d.h>

#include <lua.h>
#include <lauxlib.h>

u32 color_default = RGBA8(255, 255, 255, 255);
u32 color_background = RGBA8(0, 0, 0, 255);

/***
Set a color as the default one.
@function setDefault
@tparam integer color the color to set as the default one.
*/
static int color_setDefault(lua_State *L) {
	color_default = luaL_checkinteger(L, 1);

	return 0;
}

/***
Return the default color.
@function getDefault
@treturn integer default color
*/
static int color_getDefault(lua_State *L) {
	lua_pushinteger(L, color_default);

	return 1;
}

/***
Set a color as the background one.
@function setBackground
@tparam integer color the color to set as the background one.
*/
static int color_setBackground(lua_State *L) {
	color_background = luaL_checkinteger(L, 1);
	sf2d_set_clear_color(color_background);

	return 0;
}

/***
Return the background color.
@function getBackground
@treturn integer background color
*/
static int color_getBackground(lua_State *L) {
	lua_pushinteger(L, color_background);

	return 1;
}

/***
Return a color from red, green, blue and alpha values.
@function RGBA8
@tparam integer r the red value (0-255)
@tparam integer g the green value (0-255)
@tparam integer b the blue value (0-255)
@tparam[opt=255] integer a the alpha (opacity) value (0-255)
@treturn integer the color
*/
static int color_RGBA8(lua_State *L) {
	int r = luaL_checkinteger(L, 1);
	int g = luaL_checkinteger(L, 2);
	int b = luaL_checkinteger(L, 3);
	int a = luaL_optinteger(L, 4, 255);

	lua_pushinteger(L, RGBA8(r, g, b, a));

	return 1;
}

/***
Return a color from a hexadecimal value.
@function hex
@tparam integer hex the hexadecimal color code: 0xRRGGBBAA
@treturn integer the color
*/
static int color_hex(lua_State *L) {
	u32 hex = luaL_checkinteger(L, 1);
	
	u8 r = (hex & 0xFF000000) >> 24;
	u8 g = (hex & 0x00FF0000) >> 16;
	u8 b = (hex & 0x0000FF00) >> 8;
	u8 a = (hex & 0x000000FF);
	
	lua_pushinteger(L, RGBA8(r, g, b, a));
	
	return 1;
}

static const struct luaL_Reg color_lib[] = {
	{ "setDefault",    color_setDefault    },
	{ "getDefault",    color_getDefault    },
	{ "setBackground", color_setBackground },
	{ "getBackground", color_getBackground },
	{ "RGBA8",         color_RGBA8         },
	{ "hex",           color_hex           },
	{ NULL, NULL }
};

int luaopen_color_lib(lua_State *L) {
	luaL_newlib(L, color_lib);
	return 1;
}

void load_color_lib(lua_State *L) {
	luaL_requiref(L, "ctr.gfx.color", luaopen_color_lib, false);
}
