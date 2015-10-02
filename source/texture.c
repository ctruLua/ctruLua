/***
The `gfx.texture` module.
@module ctr.gfx.texture
@usage local texture = require("ctr.gfx.texture")
*/
#include <sf2d.h>
#include <sfil.h>

#include <lapi.h>
#include <lauxlib.h>

#include <stdlib.h>

#include "texture.h"

int getType(const char *name) {
	const char *dot = strrchr(name, '.');
	if(!dot || dot == name) dot = "";
	const char *ext = dot + 1;
	if (strncmp(ext, "png", 3) == 0) {
		return 0;
	} else if (strncmp(ext, "jpeg", 4) == 0 || strncmp(ext, "jpg", 3) == 0) {
		return 1;
	} else if (strncmp(ext, "bmp", 3) == 0) {
		return 2;
	} else {
		return 4;
	}
}

// module functions

/***
Load a texture from a file. Supported formats: PNG, JPEG, BMP.
@function load
@tparam string path path to the image file
@tparam[opt=PLACE_RAM] number place where to put the loaded texture
@tparam[opt=auto] number type type of the image
@treturn texture the loaded texture object
*/
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
		texture->texture = sfil_load_BMP_file(path, place); //appears to be broken right now.
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

/***
Texture object
@section Methods
*/

/***
Draw a texture.
@function :draw
@tparam number x X position
@tparam number y Y position
@tparam[opt=0.0] number rad rotation of the texture (in radians)
*/
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

/***
Draw a part of the texture
@function :drawPart
@tparam number x X position
@tparam number y Y position
@tparam number sx X position of the beginning of the part
@tparam number sy Y position of the beginning of the part
@tparam number w width of the part
@tparam number h height of the part
@tparam[opt=0.0] number rad rotation of the part (in radians)
*/
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

/***
Return the size of the texture.
@function :getSize
@treturn number width of the texture
@treturn number height of the texture
*/
static int texture_getSize(lua_State *L) {
	texture_userdata *texture = luaL_checkudata(L, 1, "LTexture");

	lua_pushinteger(L, texture->texture->width);
	lua_pushinteger(L, texture->texture->height);

	return 2;
}

/***
Unload a texture.
@function :unload
*/
static int texture_unload(lua_State *L) {
	texture_userdata *texture = luaL_checkudata(L, 1, "LTexture");
	
	if (texture->texture == NULL) return 0;

	sf2d_free_texture(texture->texture);
	texture->texture = NULL;
	
	return 0;
}

/***
Rescale the texture. The default scale is `1.0`.
@function :scale
@tparam number scaleX new scale of the width
@tparam number scaleY new scale of the height
*/
static int texture_scale(lua_State *L) {
	texture_userdata *texture = luaL_checkudata(L, 1, "LTexture");
	float sx = luaL_checknumber(L, 2);
	float sy = luaL_checknumber(L, 3);
	
	texture->scaleX = sx;
	texture->scaleY = sy;
	
	return 0;
}

/***
Return the color of a pixel.
@function :getPixel
@tparam number x X position of the pixel
@tparam number y Y position of the pixel
@treturn number color of the pixel.
*/
static int texture_getPixel(lua_State *L) {
	texture_userdata *texture = luaL_checkudata(L, 1, "LTexture");
	int x = luaL_checkinteger(L, 2);
	int y = luaL_checkinteger(L, 3);
	
	lua_pushinteger(L, sf2d_get_pixel(texture->texture, x, y));
	
	return 1;
}

/***
Set the color of a pixel.
@function :setPixel
@tparam number x X position of the pixel
@tparam number y Y position of the pixel
@tparam number color New color of the pixel
*/
static int texture_setPixel(lua_State *L) {
	texture_userdata *texture = luaL_checkudata(L, 1, "LTexture");
	int x = luaL_checkinteger(L, 2);
	int y = luaL_checkinteger(L, 3);
	u32 color = luaL_checkinteger(L, 4);
	
	sf2d_set_pixel(texture->texture, x, y, color);
	
	return 0;
}

/***
Set the blend color of the texture.
@function :setBlendColor
@tparam number color new blend color
*/
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

/***
Fields
@section Fields
*/

// constants
struct { char *name; int value; } texture_constants[] = {
	/***
	Constant used to select the RAM.
	@field PLACE_RAM
	*/
	{"PLACE_RAM",  SF2D_PLACE_RAM },
	/***
	Constant used to select the VRAM.
	@field PLACE_VRAM
	*/
	{"PLACE_VRAM", SF2D_PLACE_VRAM},
	/***
	Constant used to select a temporary RAM pool. Don't use it.
	@field PLACE_TEMP
	*/
	{"PLACE_TEMP", SF2D_PLACE_TEMP},
	/***
	Constant used to select the PNG type.
	@field TYPE_PNG
	*/
	{"TYPE_PNG",   0              },
	/***
	Constant used to select the JPEG type.
	@field TYPE_JPEG
	*/
	{"TYPE_JPEG",  1              },
	/***
	Constant used to select the BMP type.
	@field TYPE_BMP
	*/
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

