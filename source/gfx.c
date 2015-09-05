#include <stdlib.h>

#include <sf2d.h>
#include <sftd.h>

#include <3ds/vram.h>

#include <lua.h>
#include <lauxlib.h>

#include "font.h"

bool isGfxInitialised = false;

void load_color_lib(lua_State *L);
void load_font_lib(lua_State *L);
void load_texture_lib(lua_State *L);
void load_map_lib(lua_State *L);

void unload_font_lib(lua_State *L);

u32 color_default;

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
	bool enable = lua_toboolean(L, 1);
	
	sf2d_set_3D(enable);
	
	return 0;
}

static int gfx_vramSpaceFree(lua_State *L) {
  lua_pushinteger(L, vramSpaceFree());
  
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
	size_t len;
	const char *text = luaL_checklstring(L, 3, &len);

	int size = luaL_optinteger(L, 4, 9);
	u32 color = luaL_optinteger(L, 5, color_default);
	font_userdata *font = luaL_testudata(L, 6, "LFont");
	if (font == NULL) {
		lua_getfield(L, LUA_REGISTRYINDEX, "LFontDefault");
		font = luaL_testudata(L, -1, "LFont");
		if (font == NULL) luaL_error(L, "No default font set and no font object passed");
	}
	if (font->font == NULL) luaL_error(L, "The font object was unloaded");

	// Wide caracters support. (wchar = UTF32 on 3DS.)
	wchar_t wtext[len];
	len = mbstowcs(wtext, text, len);
	*(wtext+len) = 0x0; // text end

	sftd_draw_wtext(font->font, x, y, color, size, wtext);

	return 0;
}

// Functions
static const struct luaL_Reg gfx_lib[] = {
	{ "startFrame",    gfx_startFrame    },
	{ "endFrame",      gfx_endFrame      },
	{ "render",        gfx_render        },
	{ "getFPS",        gfx_getFPS        },
	{ "set3D",         gfx_set3D         },
	{ "vramSpaceFree", gfx_vramSpaceFree },
	{ "line",          gfx_line          },
	{ "point",         gfx_point         },
	{ "rectangle",     gfx_rectangle     },
	{ "circle",        gfx_circle        },
	{ "text",          gfx_text          },
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
struct { char *name; void (*load)(lua_State *L); void (*unload)(lua_State *L); } gfx_libs[] = {
	{ "color",   load_color_lib,   NULL            },
	{ "font",    load_font_lib,    unload_font_lib },
	{ "texture", load_texture_lib, NULL            },
	{ "map",     load_map_lib,     NULL            },
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
	sf2d_init();
	sftd_init();

	isGfxInitialised = true;

	luaL_requiref(L, "ctr.gfx", luaopen_gfx_lib, 0);
}

void unload_gfx_lib(lua_State *L) {
	for (int i = 0; gfx_libs[i].name; i++) {
		if (gfx_libs[i].unload) gfx_libs[i].unload(L);
	}

	sftd_fini();
	sf2d_fini();
}
