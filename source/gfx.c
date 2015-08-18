#include <sf2d.h>

#include <lua.h>
#include <lauxlib.h>

u32 defaultColor = RGBA8(255, 255, 255, 255);

static int gfx_startFrame(lua_State *L) {
	sf2d_start_frame(GFX_TOP, GFX_LEFT);

	return 0;
}

static int gfx_endFrame(lua_State *L) {
	sf2d_end_frame();

	return 0;
}

static int gfx_render(lua_State *L) {
	sf2d_swapbuffers();

	return 0;
}

static int gfx_rectangle(lua_State *L) {
	int x = luaL_checkinteger(L, 1);
	int y = luaL_checkinteger(L, 2);
	int width = luaL_checkinteger(L, 3);
	int height = luaL_checkinteger(L, 4);
	u32 color = luaL_optinteger(L, 5, defaultColor);
	
	sf2d_draw_rectangle(x, y, width, height, color);

	return 0;
}

static const struct luaL_Reg gfx_lib[] = {
	{ "startFrame",  gfx_startFrame},
	{ "endFrame",    gfx_endFrame  },
	{ "render",      gfx_render    },
	{ "rectangle",   gfx_rectangle },
	{ NULL, NULL }
};

int luaopen_gfx_lib(lua_State *L) {
	luaL_newlib(L, gfx_lib);
	return 1;
}

void load_gfx_lib(lua_State *L) {
	luaL_requiref(L, "ctr.gfx", luaopen_gfx_lib, 0);
}