#include <sf2d.h>
#include <sfil.h>

#include <lapi.h>
#include <lauxlib.h>

typedef struct {
	sf2d_texture *texture;
	//u32 blendColor = 0xffffffff;
} texture_userdata;

u8 getType(const char *name) {
	// NYI, always return the PNG type, because PNG is the best type.
	return 0;
}

// module functions
static int texture_load(lua_State *L) {
	const char *path = luaL_checkstring(L, 1);
	u8 place = luaL_checkinteger(L, 2);
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
	
	return 1;
}

static int texture_draw(lua_State *L) {
	texture_userdata *texture = luaL_checkudata(L, 1, "LTexture");
	int x = luaL_checkinteger(L, 2);
	int y = luaL_checkinteger(L, 3);
	
	sf2d_draw_texture(texture->texture, x, y);
	
	return 0;
}

static int texture_unload(lua_State *L) {
	texture_userdata *texture = luaL_checkudata(L, 1, "LTexture");
	
	if (texture->texture != NULL) sf2d_free_texture(texture->texture);
	texture->texture = NULL;
	
	return 0;
}

// object
static const struct luaL_Reg texture_methods[] = {
	{"draw",    texture_draw  },
	{"destroy", texture_unload},
	{"__gc",    texture_unload},
	{NULL, NULL}
};

// module
static const struct luaL_Reg texture_functions[] = {
	{"load", texture_load},
	{NULL, NULL}
};

// constants
struct { char *name; int value; } texture_constants[] = {
	{"PLACE_RAM",  0},
	{"PLACE_VRAM", 1},
	{"PLACE_TEMP", 2},
	{"TYPE_PNG",   0},
	{"TYPE_JPEG",  1},
	{"TYPE_BMP",   2},
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

