#include <sf2d.h>
#include <sfil.h>

#include <lapi.h>
#include <lauxlib.h>

#include <stdlib.h>

#include "texture.h"

u8 getType(const char *name) {
	// NYI, always return the PNG type, because PNG is the best type.
	return 0;
}

// module functions
static int texture_load(lua_State *L) {
	const char *path = luaL_checkstring(L, 1);
	u8 place = luaL_optinteger(L, 2, SF2D_PLACE_RAM); //place in ram by default
	u8 type = luaL_optinteger(L, 3, 3); //type 3 is "search at the end of the filename"

	texture_userdata *texture;
	texture = (texture_userdata *)lua_newuserdata(L, sizeof(*texture));
	
	luaL_getmetatable(L, "LTexture");
	lua_setmetatable(L, -2);
	
	if (type==3) type = getType(path);
	if (type==0) { //PNG
		texture->texture = sfil_load_PNG_file(path, place);
	} else if (type==1) { //JPEG
		texture->texture = sfil_load_JPEG_file(path, place);
	} else if (type==2) { //BMP
		texture->texture = sfil_load_BMP_file(path, place);
	} else {
		lua_pushnil(L);
		lua_pushstring(L, "Bad type");
		return 2;
	}
	
	if (texture->texture == NULL) {
	  lua_pushnil(L);
	  lua_pushstring(L, "No such file");
	  return 2;
	}
	
	texture->scaleX = 1.0f;
	texture->scaleY = 1.0f;
	texture->blendColor = 0xffffffff;
	
	return 1;
}

static int texture_draw(lua_State *L) {
	texture_userdata *texture = luaL_checkudata(L, 1, "LTexture");
	int x = luaL_checkinteger(L, 2);
	int y = luaL_checkinteger(L, 3);
	float rad = luaL_optnumber(L, 4, 0.0f);
	
	if (rad == 0.0f && texture->scaleX == 1.0f && texture->scaleY == 1.0f && texture->blendColor == 0xffffffff) {
		sf2d_draw_texture(texture->texture, x, y);
	} else {
		sf2d_draw_texture_part_rotate_scale_blend(texture->texture, x, y, rad, 0, 0, texture->texture->width, texture->texture->height, texture->scaleX, texture->scaleY, texture->blendColor);
	}
	
	return 0;
}

static int texture_drawPart(lua_State *L) {
	texture_userdata *texture = luaL_checkudata(L, 1, "LTexture");
	int x = luaL_checkinteger(L, 2);
	int y = luaL_checkinteger(L, 3);
	int sx = luaL_checkinteger(L, 4);
	int sy = luaL_checkinteger(L, 5);
	int w = luaL_checkinteger(L, 6);
	int h = luaL_checkinteger(L, 7);
	int rad = luaL_optnumber(L, 8, 0.0f);
	
	sf2d_draw_texture_part_rotate_scale_blend(texture->texture, x, y, rad, sx, sy, w, h, texture->scaleX, texture->scaleY, texture->blendColor);
	
	return 0;
}

static int texture_getSize(lua_State *L) {
	texture_userdata *texture = luaL_checkudata(L, 1, "LTexture");

	lua_pushinteger(L, texture->texture->width);
	lua_pushinteger(L, texture->texture->height);

	return 2;
}

static int texture_unload(lua_State *L) {
	texture_userdata *texture = luaL_checkudata(L, 1, "LTexture");
	
	if (texture->texture == NULL) return 0;

	sf2d_free_texture(texture->texture);
	texture->texture = NULL;
	
	return 0;
}

static int texture_scale(lua_State *L) {
	texture_userdata *texture = luaL_checkudata(L, 1, "LTexture");
	float sx = luaL_checknumber(L, 2);
	float sy = luaL_checknumber(L, 3);
	
	texture->scaleX = sx;
	texture->scaleY = sy;
	
	return 0;
}

static int texture_getPixel(lua_State *L) {
	texture_userdata *texture = luaL_checkudata(L, 1, "LTexture");
	int x = luaL_checkinteger(L, 2);
	int y = luaL_checkinteger(L, 3);
	
	lua_pushinteger(L, sf2d_get_pixel(texture->texture, x, y));
	
	return 1;
}

static int texture_setPixel(lua_State *L) {
	texture_userdata *texture = luaL_checkudata(L, 1, "LTexture");
	int x = luaL_checkinteger(L, 2);
	int y = luaL_checkinteger(L, 3);
	u32 color = luaL_checkinteger(L, 4);
	
	sf2d_set_pixel(texture->texture, x, y, color);
	
	return 0;
}

static int texture_setBlendColor(lua_State *L) {
  texture_userdata *texture = luaL_checkudata(L, 1, "LTexture");
	u32 color = luaL_checkinteger(L, 2);
	
	texture->blendColor = color;
	
	return 0;
}

// object
static const struct luaL_Reg texture_methods[] = {
	{ "draw",          texture_draw          },
	{ "drawPart",      texture_drawPart      },
	{ "scale",         texture_scale         },
	{ "getSize",       texture_getSize       },
	{ "unload",        texture_unload        },
	{ "getPixel",      texture_getPixel      },
	{ "setPixel",      texture_setPixel      },
	{ "setBlendColor", texture_setBlendColor },
	{ "__gc",          texture_unload        },
	{NULL, NULL}
};

// module
static const struct luaL_Reg texture_functions[] = {
	{"load", texture_load},
	{NULL, NULL}
};

// constants
struct { char *name; int value; } texture_constants[] = {
	{"PLACE_RAM",  SF2D_PLACE_RAM },
	{"PLACE_VRAM", SF2D_PLACE_VRAM},
	{"PLACE_TEMP", SF2D_PLACE_TEMP},
	{"TYPE_PNG",   0              },
	{"TYPE_JPEG",  1              },
	{"TYPE_BMP",   2              },
	{NULL, 0}
};

int luaopen_texture_lib(lua_State *L) {
	luaL_newmetatable(L, "LTexture");
	lua_pushvalue(L, -1);
	lua_setfield(L, -2, "__index");
	luaL_setfuncs(L, texture_methods, 0);
	
	luaL_newlib(L, texture_functions);
	
	for (int i = 0; texture_constants[i].name; i++) {
		lua_pushinteger(L, texture_constants[i].value);
		lua_setfield(L, -2, texture_constants[i].name);
	}
	
	return 1;
}

void load_texture_lib(lua_State *L) {
	luaL_requiref(L, "ctr.gfx.texture", luaopen_texture_lib, false);
}

