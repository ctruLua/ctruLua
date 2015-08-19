#include <sf2d.h>
#include <sftd.h>

#include <lua.h>
#include <lauxlib.h>

int load_color_lib(lua_State *L);
int load_font_lib(lua_State *L);

u32 color_default;
sftd_font *font_default;

static int gfx_startFrame(lua_State *L) {
	u8 screen = luaL_checkinteger(L, 1);
	u8 eye = luaL_optinteger(L, 2, GFX_LEFT);
	
	sf2d_start_frame(screen, eye);

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

static int gfx_getFPS(lua_State *L) {
	float fps = sf2d_get_fps();

	lua_pushnumber(L, fps);

	return 1;
}

static int gfx_set3D(lua_State *L) {
  bool enable = false;
  if (lua_isboolean(L, 1))
		enable = lua_toboolean(L, 1);
	
	gfxSet3D(enable);
	
	return 1;
}

static int gfx_line(lua_State *L) {
	int x1 = luaL_checkinteger(L, 1);
	int y1 = luaL_checkinteger(L, 2);
	int x2 = luaL_checkinteger(L, 3);
	int y2 = luaL_checkinteger(L, 4);
	
	u32 color = luaL_optinteger(L, 5, color_default);
	
	sf2d_draw_line(x1, y1, x2, y2, color);

	return 0;
}

static int gfx_point(lua_State *L) {
	int x = luaL_checkinteger(L, 1);
	int y = luaL_checkinteger(L, 2);
	
	u32 color = luaL_optinteger(L, 3, color_default);
	
	sf2d_draw_rectangle(x, y, 1, 1, color); // well, it looks like a point

	return 0;
}

static int gfx_rectangle(lua_State *L) {
	int x = luaL_checkinteger(L, 1);
	int y = luaL_checkinteger(L, 2);
	int width = luaL_checkinteger(L, 3);
	int height = luaL_checkinteger(L, 4);

	float angle = luaL_optnumber(L, 5, 0);
	u32 color = luaL_optinteger(L, 6, color_default);
	
	if (angle == 0)
		sf2d_draw_rectangle(x, y, width, height, color);
	else
		sf2d_draw_rectangle_rotate(x, y, width, height, color, angle);

	return 0;
}

static int gfx_circle(lua_State *L) {
	int x = luaL_checkinteger(L, 1);
	int y = luaL_checkinteger(L, 2);
	int radius = luaL_checkinteger(L, 3);
	
	u32 color = luaL_optinteger(L, 4, color_default);
	
	sf2d_draw_fill_circle(x, y, radius, color);

	return 0;
}

static int gfx_text(lua_State *L) {
	int x = luaL_checkinteger(L, 1);
	int y = luaL_checkinteger(L, 2);
	const char *text = luaL_checkstring(L, 3);

	int size = luaL_optinteger(L, 4, 9);
	u32 color = luaL_optinteger(L, 5, color_default);
	// todo : font selection

	sftd_draw_text(font_default, x, y, color, size, text);

	return 0;
}

// Functions
static const struct luaL_Reg gfx_lib[] = {
	{ "startFrame",  gfx_startFrame},
	{ "endFrame",    gfx_endFrame  },
	{ "render",      gfx_render    },
	{ "getFPS",      gfx_getFPS    },
	{ "set3D",       gfx_set3D     },
	{ "line",        gfx_line      },
	{ "point",       gfx_point     },
	{ "rectangle",   gfx_rectangle },
	{ "circle",      gfx_circle    },
	{ "text",        gfx_text      },
	{ NULL, NULL }
};

// Constants
struct { char *name; int value; } gfx_constants[] = {
	{ "GFX_TOP",       GFX_TOP    },
	{ "GFX_BOTTOM",    GFX_BOTTOM },
	{ "GFX_LEFT",      GFX_LEFT   },
	{ "GFX_RIGHT",     GFX_RIGHT  },
	{ "TOP_HEIGHT",    240        },
	{ "TOP_WIDTH",     400        },
	{ "BOTTOM_HEIGHT", 240        },
	{ "BOTTOM_WIDTH",  320        },
	{ NULL, 0 }
};

// Subtables
struct { char *name; int (*load)(lua_State *L); } gfx_libs[] = {
	{ "color", load_color_lib },
	{ "font",  load_font_lib  },
	{ NULL, NULL }
};

int luaopen_gfx_lib(lua_State *L) {
	luaL_newlib(L, gfx_lib);
	
	for (int i = 0; gfx_constants[i].name; i++) {
		lua_pushinteger(L, gfx_constants[i].value);
		lua_setfield(L, -2, gfx_constants[i].name);
	}
	
	for (int i = 0; gfx_libs[i].name; i++) {
		gfx_libs[i].load(L);
		lua_setfield(L, -2, gfx_libs[i].name);
	}

	return 1;
}

void load_gfx_lib(lua_State *L) {
	luaL_requiref(L, "ctr.gfx", luaopen_gfx_lib, 0);
}
