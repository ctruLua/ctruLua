/***
The `map` module.
Tile coordinates start at x=0,y=0.
@module ctr.gfx.map
@usage local map = require("ctr.gfx.map")
*/

#include <sf2d.h>

#include <lapi.h>
#include <lauxlib.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "texture.h"

typedef struct {
	texture_userdata *texture;
	u8 tileSizeX;
	u8 tileSizeY;
	int tilesetSizeX; // in tiles
	int tilesetSizeY; // in tiles
	u16 *data;
	int width;
	int height;
	int spaceX; // in pixels
	int spaceY; // in pixels
} map_userdata;

void getTilePos(map_userdata *map, u16 tile, int *texX, int *texY) {
	*texX = (tile%map->tilesetSizeX)*map->tileSizeX;
	*texY = (tile/map->tilesetSizeX)*map->tileSizeY;
}

u16 getTile(map_userdata *map, int x, int y) {
	return map->data[x+(y*map->width)];
}

// module functions

/***
Load a map from a file.
@function load
@tparam string/table map path to the .map or a 2D table containing tile data (`{ [Y1]={[X1]=tile1, [X2]=tile2}, [Y2]=..., ... }`)
@tparam texture tileset containing the tileset
@tparam number tileWidth tile width
@tparam number tileHeight tile height
@treturn map loaded map object
*/
static int map_load(lua_State *L) {
	texture_userdata *texture = luaL_checkudata(L, 2, "LTexture");
	u8 tileSizeX = luaL_checkinteger(L, 3);
	u8 tileSizeY = luaL_checkinteger(L, 4);
	
	map_userdata *map = lua_newuserdata(L, sizeof(map_userdata));
	luaL_getmetatable(L, "LMap");
	lua_setmetatable(L, -2);
	
	map->texture = texture;
	map->tileSizeX = tileSizeX;
	map->tileSizeY = tileSizeY;
	map->tilesetSizeX = (map->texture->texture->width/tileSizeX);
	map->tilesetSizeY = (map->texture->texture->height/tileSizeY);
	map->spaceX = 0;
	map->spaceY = 0;
	
	// read the map file
	if (lua_isstring(L, 1)) {
		const char *mapPath = luaL_checkstring(L, 1);

		FILE *mapFile = fopen(mapPath, "r");
		if (mapFile == NULL) {
			lua_pushnil(L);
			lua_pushfstring(L, "no such file \"%s\"", mapPath);
			return 2;
		}
		fseek(mapFile, 0L, SEEK_END);
		int fileSize = ftell(mapFile);
		fseek(mapFile, 0L, SEEK_SET);
		char *buffer = (char *)malloc(sizeof(char)*fileSize);
		fread(buffer, 1, fileSize, mapFile);
		fclose(mapFile);

		int width = 0;
		for (int i=0; buffer[i]; i++) {
			if (buffer[i] == ',') {
				width++;
			} else if (buffer[i] == '\n') {
				width++;
				break;
			}
		}
		int height = 0;
		for (int i=0; buffer[i]; i++) { // this should do
			if (buffer[i] == '\n' && (buffer[i+1] != '\n' || !buffer[i+1])) height++;
		}

		map->width = width;
		map->height = height;

		map->data = malloc(sizeof(u16)*width*height);
		int i = 0;
		char *token = strtok(buffer, ",\n");
		while (token != NULL) {
			map->data[i] = (u16)atoi(token);
			i++;
			token = strtok(NULL, ",\n");
		}
		free(buffer);

		return 1;

	} else if (lua_istable(L, 1)) {
		int height = luaL_len(L, 1);
		if (height < 1) luaL_error(L, "map height must be greater or equal to 1");

		lua_geti(L, 1, 1);
		int width = luaL_len(L, -1);
		if (width < 1) luaL_error(L, "map width must be greater or equal to 1");
		lua_pop(L, 1);

		map->width = width;
		map->height = height;

		map->data = malloc(sizeof(u16)*width*height);
		for (int y=1; y<=height; y++) {
			if (lua_geti(L, 1, y) != LUA_TTABLE) luaL_error(L, "map table must be an array of tables");
			if (luaL_len(L, -1) < width) luaL_error(L, "table line y=%d is shorter than the map width", y);

			for (int x=1; x<=width; x++) {
				lua_geti(L, -1, x);

				bool isnum;
				map->data[(x-1)+((y-1)*map->width)] = (u16)lua_tointegerx(L, -1, (int *)&isnum);
				if (!isnum) luaL_error(L, "tiles must be integers");

				lua_pop(L, 1);
			}

			lua_pop(L, 1);
		}
		
		return 1;

	} else {
		luaL_error(L, "map (first argument) must be a string or a table");
		return 0;
	}
}

/***
Map object
@section Methods
*/

/***
Draw a map.
@function :draw
@tparam number x X position
@tparam number y Y position
@within Methods
*/
static int map_draw(lua_State *L) {
	map_userdata *map = luaL_checkudata(L, 1, "LMap");
	int x = luaL_checkinteger(L, 2);
	int y = luaL_checkinteger(L, 3);
	int texX = 0;
	int texY = 0;
	
	if (map->texture->blendColor == 0xffffffff) {
		for (int xp=0; xp<map->width; xp++) {
			for (int yp=0; yp<map->height; yp++) {
				u16 tile = getTile(map, xp, yp);
				getTilePos(map, tile, &texX, &texY);
				sf2d_draw_texture_part(map->texture->texture, (x+(map->tileSizeX*xp)+(xp*map->spaceX)), (y+(map->tileSizeY*yp)+(yp*map->spaceY)), texX, texY, map->tileSizeX, map->tileSizeY);
			}
		}
	} else {
		for (int xp=0; xp<map->width; xp++) {
			for (int yp=0; yp<map->height; yp++) {
				u16 tile = getTile(map, xp, yp);
				getTilePos(map, tile, &texX, &texY);
				sf2d_draw_texture_part_blend(map->texture->texture, (x+(map->tileSizeX*xp)+(xp*map->spaceX)), (y+(map->tileSizeY*yp)+(yp*map->spaceY)), texX, texY, map->tileSizeX, map->tileSizeY, map->texture->blendColor);
			}
		}
	}
	
	return 0;
}

/***
Unload a map.
@function :unload
@within Methods
*/
static int map_unload(lua_State *L) {
	map_userdata *map = luaL_checkudata(L, 1, "LMap");

	free(map->data);

	return 0;
}

/***
Return the size of a map.
@function :getSize
@treturn number width of the map, in tiles
@treturn number height of the map, in tiles
@within Methods
*/
static int map_getSize(lua_State *L) {
	map_userdata *map = luaL_checkudata(L, 1, "LMap");
	
	lua_pushinteger(L, map->width);
	lua_pushinteger(L, map->height);
	
	return 2;
}

/***
Return the value of a tile.
@function :getTile
@tparam number x X position of the tile (in tiles)
@tparam number y Y position of the tile (in tiles)
@treturn number value of the tile
@within Methods
*/
static int map_getTile(lua_State *L) {
	map_userdata *map = luaL_checkudata(L, 1, "LMap");
	int x = luaL_checkinteger(L, 2);
	int y = luaL_checkinteger(L, 3);
	
	lua_pushinteger(L, getTile(map, x, y));

	return 1;
}

/***
Set the value of a tile.
@function :setTile
@tparam number x X position of the tile (in tiles)
@tparam number y Y position of the tile (in tiles)
@tparam number value new value for the tile
@within Methods
*/
static int map_setTile(lua_State *L) {
	map_userdata *map = luaL_checkudata(L, 1, "LMap");
	int x = luaL_checkinteger(L, 2);
	int y = luaL_checkinteger(L, 3);
	u16 tile = luaL_checkinteger(L, 4);
	
	map->data[x+(y*map->width)] = tile;
	
	return 0;
}

/***
Set the space between draw tiles (in pixels).
@function :setSpace
@tparam number x X space (in pixels)
@tparam number y Y space (in pixels)
*/
static int map_setSpace(lua_State *L) {
	map_userdata *map = luaL_checkudata(L, 1, "LMap");
	int x = luaL_optinteger(L, 2, map->spaceX);
	int y = luaL_optinteger(L, 3, map->spaceY);
	
	map->spaceX = x;
	map->spaceY = y;
	
	return 0;
}

// object
static const struct luaL_Reg map_methods[] = {
	{"draw",          map_draw         },
	{"unload",        map_unload       },
	{"getSize",       map_getSize      },
	{"getTile",       map_getTile      },
	{"setTile",       map_setTile      },
	{"setSpace",      map_setSpace     },
	{"__gc",          map_unload       },
	{NULL, NULL}
};

// module
static const struct luaL_Reg map_functions[] = {
	{ "load",   map_load },
	{NULL, NULL}
};

int luaopen_map_lib(lua_State *L) {
	luaL_newmetatable(L, "LMap");
	lua_pushvalue(L, -1);
	lua_setfield(L, -2, "__index");
	luaL_setfuncs(L, map_methods, 0);
	
	luaL_newlib(L, map_functions);
	
	return 1;
}

void load_map_lib(lua_State *L) {
	luaL_requiref(L, "ctr.gfx.map", luaopen_map_lib, false);
}

