#include <sf2d.h>

#include <lua.h>
#include <lauxlib.h>

u32 color_default = RGBA8(255, 255, 255, 255);

static int color_setDefault(lua_State *L) {
	color_default = luaL_checkinteger(L, 1);

	return 0;
}

static int color_getDefault(lua_State *L) {
	lua_pushinteger(L, color_default);

	return 1;
}

static const struct luaL_Reg color_lib[] = {
	{ "setDefault", color_setDefault },
	{ "getDefault", color_getDefault },
	{ NULL, NULL }
};

int luaopen_color_lib(lua_State *L) {
	luaL_newlib(L, color_lib);
	return 1;
}

void load_color_lib(lua_State *L) {
	luaL_requiref(L, "ctr.gfx.color", luaopen_color_lib, false);
}