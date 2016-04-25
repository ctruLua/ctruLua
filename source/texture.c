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
#include <string.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>
#include <png.h>

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
@treturn[1] texture the loaded texture object
@treturn[2] nil in case of error
@treturn[2] string error message
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
		int w, h;
		char* data = (char*)stbi_load(path, &w, &h, NULL, 4);
		if (data == NULL) {
			lua_pushnil(L);
			lua_pushstring(L, "Can't open file");
			return 2;
		}
		texture->texture = sf2d_create_texture_mem_RGBA8(data, w, h, TEXFMT_RGBA8, place);
		free(data);
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
Create an empty texture.
@function new
@tparam number width Texture width
@tparam number height Texture height
@tparam[opt=PLACE_RAM] number place where to put the loaded texture
@treturn texture the loaded texture object
*/
static int texture_new(lua_State *L) {
	int w = luaL_checkinteger(L, 1);
	int h = luaL_checkinteger(L, 2);
	u8 place = luaL_checkinteger(L, 3);

	texture_userdata *texture;
	texture = (texture_userdata *)lua_newuserdata(L, sizeof(*texture));

	luaL_getmetatable(L, "LTexture");
	lua_setmetatable(L, -2);

	texture->texture = sf2d_create_texture(w, h, TEXFMT_RGBA8, place);
	sf2d_texture_tile32(texture->texture);

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
@tparam integer x X position
@tparam integer y Y position
@tparam[opt=0.0] number rad rotation of the texture around the hotspot (in radians)
@tparam[opt=0.0] number hotspotX the hostpot X coordinate
@tparam[opt=0.0] number hotspotY the hostpot Y coordinate
*/
static int texture_draw(lua_State *L) {
	texture_userdata *texture = luaL_checkudata(L, 1, "LTexture");
	int x = luaL_checkinteger(L, 2);
	int y = luaL_checkinteger(L, 3);
	float rad = luaL_optnumber(L, 4, 0.0f);
	float hotspotX = luaL_optnumber(L, 5, 0.0f);
	float hotspotY = luaL_optnumber(L, 6, 0.0f);

	if (rad == 0.0f && texture->scaleX == 1.0f && texture->scaleY == 1.0f && texture->blendColor == 0xffffffff) {
		sf2d_draw_texture(texture->texture, x - hotspotX, y - hotspotY);
	} else {
		sf2d_draw_texture_rotate_scale_hotspot_blend(texture->texture, x, y, rad, texture->scaleX, texture->scaleY, hotspotX, hotspotY, texture->blendColor);
	}

	return 0;
}

/***
Draw a part of the texture
@function :drawPart
@tparam integer x X position
@tparam integer y Y position
@tparam integer sx X position of the beginning of the part
@tparam integer sy Y position of the beginning of the part
@tparam integer w width of the part
@tparam integer h height of the part
@tparam[opt=0.0] number rad rotation of the part around the hotspot (in radians)
@tparam[opt=0.0] number hotspotX the hostpot X coordinate
@tparam[opt=0.0] number hotspotY the hostpot Y coordinate
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
	float hotspotX = luaL_optnumber(L, 9, 0.0f);
	float hotspotY = luaL_optnumber(L, 10, 0.0f);

	sf2d_draw_texture_part_rotate_scale_hotspot_blend(texture->texture, x, y, rad, sx, sy, w, h, texture->scaleX, texture->scaleY, hotspotX, hotspotY, texture->blendColor);

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
@tparam[opt=scaleX] number scaleY new scale of the height
*/
static int texture_scale(lua_State *L) {
	texture_userdata *texture = luaL_checkudata(L, 1, "LTexture");
	float sx = luaL_checknumber(L, 2);
	float sy = luaL_optnumber(L, 3, sx);

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

/***
Get the blend color of the texture.
@function :getBlendColor
@treturn number the blend color
*/
static int texture_getBlendColor(lua_State *L) {
	texture_userdata *texture = luaL_checkudata(L, 1, "LTexture");

	lua_pushinteger(L, texture->blendColor);

	return 1;
}

/***
Save a texture to a file.
@function :save
@tparam string filename path to the file to save the texture to
@tparam[opt=TYPE_PNG] number type type of the image to save. Can be TYPE_PNG or TYPE_BMP
@treturn[1] boolean true on success
@treturn[2] boolean `false` in case of error
@treturn[2] string error message
*/
static int texture_save(lua_State *L) {
	texture_userdata *texture = luaL_checkudata(L, 1, "LTexture");
	const char* path = luaL_checkstring(L, 2);
	u8 type = luaL_optinteger(L, 3, 0);

	int result = 0;
	if (type == 0) { // PNG
		FILE* file = fopen(path, "wb");
		if (file == NULL) {
			lua_pushboolean(L, false);
			lua_pushstring(L, "Can open file");
			return 2;
		}
		png_structp png = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
		png_infop infos = png_create_info_struct(png);
		setjmp(png_jmpbuf(png));
		png_init_io(png, file);

		png_set_IHDR(png, infos, texture->texture->width, texture->texture->height, 8, PNG_COLOR_TYPE_RGB_ALPHA, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
		png_write_info(png, infos);

		png_bytep row = malloc(4 * texture->texture->width * sizeof(png_byte));

		for(int y=0;y<texture->texture->height;y++) {
			for (int x=0;x<texture->texture->width;x++) {
				((u32*)row)[x] = __builtin_bswap32(sf2d_get_pixel(texture->texture, x, y));
			}
			png_write_row(png, row);
		}

		png_write_end(png, NULL);

		fclose(file);
		png_free_data(png, infos, PNG_FREE_ALL, -1);
		png_destroy_write_struct(&png, &infos);
		free(row);

		result = 1;

	} else if (type == 2) { // BMP
		u32* buff = malloc(texture->texture->width * texture->texture->height * 4);
		if (buff == NULL) {
			lua_pushboolean(L, false);
			lua_pushstring(L, "Failed to allocate buffer");
			return 2;
		}
		for (int y=0;y<texture->texture->height;y++) {
			for (int x=0;x<texture->texture->width;x++) {
				buff[x+(y*texture->texture->width)] = __builtin_bswap32(sf2d_get_pixel(texture->texture, x, y));
			}
		}
		result = stbi_write_bmp(path, texture->texture->width, texture->texture->height, 4, buff);
		free(buff);
	
	} else {
		lua_pushboolean(L, false);
		lua_pushstring(L, "Not a valid type");
		return 2;
	}

	if (result == 0) {
		lua_pushboolean(L, false);
		lua_pushstring(L, "Failed to save the texture");
		return 2;
	}

	lua_pushboolean(L, true);
	return 1;
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
	{ "getBlendColor", texture_getBlendColor },
	{ "save",          texture_save          },
	{ "__gc",          texture_unload        },
	{NULL, NULL}
};

// module
static const struct luaL_Reg texture_functions[] = {
	{"load", texture_load},
	{"new",  texture_new },
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

