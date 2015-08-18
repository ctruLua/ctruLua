#include <sf2d.h>

#include <lua.h>
#include <lauxlib.h>

int load_color_lib(lua_State *L);

u32 color_default;

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
	u32 color = luaL_optinteger(L, 5, color_default);
	
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

struct { char *name; int (*load)(lua_State *L); } gfx_libs[] = {
	{ "color", load_color_lib },
	{ NULL, NULL }
};

int luaopen_gfx_lib(lua_State *L) {
	luaL_newlib(L, gfx_lib);

	for (int i = 0; gfx_libs[i].name; i++) {
		gfx_libs[i].load(L);
		lua_setfield(L, -2, gfx_libs[i].name);
	}

	return 1;
}

void load_gfx_lib(lua_State *L) {
	luaL_requiref(L, "ctr.gfx", luaopen_gfx_lib, 0);
}