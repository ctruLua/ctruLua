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
} map_userdata;

void getTilePos(map_userdata *map, u16 tile, int *texX, int *texY) {
	*texX = (tile%map->tilesetSizeX)*map->tileSizeX;
	*texY = (tile/map->tilesetSizeX)*map->tileSizeY;
}

u16 getTile(map_userdata *map, int x, int y) {
	return map->data[x+(y*map->width)];
}

// module functions
static int map_load(lua_State *L) {
	const char *mapPath = luaL_checkstring(L, 1);
	texture_userdata *texture = luaL_checkudata(L, 2, "LTexture");
	u8 tileSizeX = luaL_checkinteger(L, 3);
	u8 tileSizeY = luaL_checkinteger(L, 4);
	
	map_userdata *map;
	map = (map_userdata*)lua_newuserdata(L, sizeof(map_userdata));
	luaL_getmetatable(L, "LMap");
	lua_setmetatable(L, -2);
	
	map->texture = texture;
	map->tileSizeX = tileSizeX;
	map->tileSizeY = tileSizeY;
	map->tilesetSizeX = (map->texture->texture->width/tileSizeX);
	map->tilesetSizeY = (map->texture->texture->height/tileSizeY);
	
	// read the map file
	FILE *mapFile = fopen(mapPath, "r");
	if (mapFile == NULL) {
		lua_pushnil(L);
		lua_pushstring(L, "No such file");
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
		if (buffer[i] == '|') {
			width++;
		} else if (buffer[i] == '\n') {
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
	char *token;
	token = strtok(buffer, "|");
	while (token != NULL) {
		map->data[i] = (u16)atoi(token);
		i++;
		token = strtok(NULL, "|");
	}
	free(buffer);
	
	return 1;
}

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
				sf2d_draw_texture_part(map->texture->texture, (x+(map->tileSizeX*xp)), (y+(map->tileSizeY*yp)), texX, texY, map->tileSizeX, map->tileSizeY);
			}
		}
	} else {
		for (int xp=0; xp<map->width; xp++) {
			for (int yp=0; yp<map->height; yp++) {
				u16 tile = getTile(map, xp, yp);
				getTilePos(map, tile, &texX, &texY);
				sf2d_draw_texture_part_blend(map->texture->texture, (x+(map->tileSizeX*xp)), (y+(map->tileSizeY*yp)), texX, texY, map->tileSizeX, map->tileSizeY, map->texture->blendColor);
			}
		}
	}
	
	return 0;
}

static int map_unload(lua_State *L) {
	map_userdata *map = luaL_checkudata(L, 1, "LMap");
	free(map->data);
	free(map);
	return 0;
}

static int map_getSize(lua_State *L) {
	map_userdata *map = luaL_checkudata(L, 1, "LMap");
	
	lua_pushinteger(L, map->width);
	lua_pushinteger(L, map->height);
	
	return 2;
}

static int map_getTile(lua_State *L) {
	map_userdata *map = luaL_checkudata(L, 1, "LMap");
	int x = luaL_checkinteger(L, 2);
	int y = luaL_checkinteger(L, 3);
	
	lua_pushinteger(L, getTile(map, x, y));
	return 1;
}

static int map_setTile(lua_State *L) {
	map_userdata *map = luaL_checkudata(L, 1, "LMap");
	int x = luaL_checkinteger(L, 2);
	int y = luaL_checkinteger(L, 3);
	u16 tile = luaL_checkinteger(L, 4);
	
	map->data[x+(y*map->width)] = tile;
	
	return 0;
}

// object
static const struct luaL_Reg map_methods[] = {
	{"draw",          map_draw         },
	{"unload",        map_unload       },
	{"getSize",       map_getSize      },
	{"getTile",       map_getTile      },
	{"setTile",       map_setTile      },
	{"__gc",          map_unload       },
	{NULL, NULL}
};

// module
static const struct luaL_Reg map_functions[] = {
	{"load",   map_load},
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

