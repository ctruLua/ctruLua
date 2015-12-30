/***
The `gfx` module.
@module ctr.gfx
@usage local gfx = require("ctr.gfx")
*/
#include <stdlib.h>

#include <sf2d.h>
#include <sftd.h>

#include <3ds/vram.h>
//#include <3ds/services/gsp.h>

#include <lua.h>
#include <lauxlib.h>

#include "font.h"

bool isGfxInitialized = false;
bool is3DEnabled = false; //TODO: add a function for this in the ctrulib/sf2dlib.

/***
The `ctr.gfx.color` module.
@table color
@see ctr.gfx.color
*/
void load_color_lib(lua_State *L);
u32 color_default;

/***
The `ctr.gfx.font` module.
@table font
@see ctr.gfx.font
*/
void load_font_lib(lua_State *L);
void unload_font_lib(lua_State *L);

/***
The `ctr.gfx.texture` module.
@table texture
@see ctr.gfx.texture
*/
void load_texture_lib(lua_State *L);

/***
The `ctr.gfx.map` module.
@table map
@see ctr.gfx.map
*/
void load_map_lib(lua_State *L);

/***
Start drawing to a screen.
Must be called before any draw operation.
@function start
@tparam number screen the screen to draw to (`GFX_TOP` or `GFX_BOTTOM`)
@tparam[opt=GFX_LEFT] number eye the eye to draw to (`GFX_LEFT` or `GFX_RIGHT`)
*/
static int gfx_start(lua_State *L) {
	u8 screen = luaL_checkinteger(L, 1);
	u8 eye = luaL_optinteger(L, 2, GFX_LEFT);
	
	sf2d_start_frame(screen, eye);

	return 0;
}

/***
End drawing to a screen.
Must be called after your draw operations.
@function stop
*/
static int gfx_stop(lua_State *L) {
	sf2d_end_frame();

	return 0;
}

/***
Display any drawn pixel.
@function render
*/
static int gfx_render(lua_State *L) {
	sf2d_swapbuffers();

	return 0;
}

/***
Get the current number of frame per second.
@function getFPS
@treturn number number of frame per seconds
*/
static int gfx_getFPS(lua_State *L) {
	float fps = sf2d_get_fps();

	lua_pushnumber(L, fps);

	return 1;
}

/***
Enable or disable the stereoscopic 3D on the top screen.
@function set3D
@tparam boolean enable true to enable, false to disable
*/
static int gfx_set3D(lua_State *L) {
	is3DEnabled = lua_toboolean(L, 1);
	
	sf2d_set_3D(is3DEnabled);
	
	return 0;
}

/***
Check whether or not the stereoscopic 3D is enabled.
@function get3D
@treturn boolean true if enabled, false if disabled
*/
static int gfx_get3D(lua_State *L) {
  lua_pushboolean(L, is3DEnabled);
  return 1;
}

/***
Enable or disable the VBlank waiting.
@function setVBlankWait
@tparam boolean true to enable, false to disable
*/
static int gfx_setVBlankWait(lua_State *L) {
	bool enable = lua_toboolean(L, 1);
	
	sf2d_set_vblank_wait(enable);
	
	return 0;
}

/***
Wait for the VBlank interruption.
@function waitForVBlank
@tparam[opt=GFX_TOP] number screen the screen's VBlank to wait for
*/
static int gfx_waitForVBlank(lua_State *L) {
	u8 screen = luaL_optinteger(L, 1, GFX_TOP);
	if (screen == GFX_TOP) {
		gspWaitForVBlank0();
	} else {
		gspWaitForVBlank1();
	}
	
	return 0;
}

/***
Get free VRAM space.
@function vramSpaceFree
@treturn integer free VRAM in bytes
*/
static int gfx_vramSpaceFree(lua_State *L) {
  lua_pushinteger(L, vramSpaceFree());
  
  return 1;
}

/***
Draw a line on the current screen.
@function line
@tparam integer x1 line's starting point horizontal coordinate, in pixels
@tparam integer y1 line's starting point vertical coordinate, in pixels
@tparam integer x2 line's endpoint horizontal coordinate, in pixels
@tparam integer y2 line's endpoint vertical coordinate, in pixels
@tparam[opt=default color] integer color drawing color
*/
static int gfx_line(lua_State *L) {
	int x1 = luaL_checkinteger(L, 1);
	int y1 = luaL_checkinteger(L, 2);
	int x2 = luaL_checkinteger(L, 3);
	int y2 = luaL_checkinteger(L, 4);
	
	u32 color = luaL_optinteger(L, 5, color_default);
	
	sf2d_draw_line(x1, y1, x2, y2, color);

	return 0;
}

/***
Draw a point, a single pixel, on the current screen.
@function point
@tparam integer x point horizontal coordinate, in pixels
@tparam integer y point vertical coordinate, in pixels
@tparam[opt=default color] integer color drawing color
*/
static int gfx_point(lua_State *L) {
	int x = luaL_checkinteger(L, 1);
	int y = luaL_checkinteger(L, 2);
	
	u32 color = luaL_optinteger(L, 3, color_default);
	
	sf2d_draw_rectangle(x, y, 1, 1, color); // well, it looks like a point

	return 0;
}

/***
Draw a rectangle on the current screen.
@function rectangle
@tparam integer x rectangle origin horizontal coordinate, in pixels
@tparam integer y rectangle origin vertical coordinate, in pixels
@tparam integer width rectangle width, in pixels
@tparam integer height rectangle height, in pixels
@tparam[opt=0] number angle rectangle rotation, in radians
@tparam[opt=default color] integer color drawing color
*/
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

/***
Draw a circle on the current screen.
@function circle
@tparam integer x circle center horizontal coordinate, in pixels
@tparam integer y circle center vertical coordinate, in pixels
@tparam integer radius circle radius, in pixels
@tparam[opt=default color] integer color drawing color
*/
static int gfx_circle(lua_State *L) {
	int x = luaL_checkinteger(L, 1);
	int y = luaL_checkinteger(L, 2);
	int radius = luaL_checkinteger(L, 3);
	
	u32 color = luaL_optinteger(L, 4, color_default);
	
	sf2d_draw_fill_circle(x, y, radius, color);

	return 0;
}

/***
Draw a text on the current screen.
@function text
@tparam integer x text drawing origin horizontal coordinate, in pixels
@tparam integer y text drawing origin vertical coordinate, in pixels
@tparam string text the text to draw
@tparam[opt=9] integer size drawing size, in pixels
@tparam[opt=default color] integer color drawing color
@tparam[opt=default font] font font to use
*/
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
	wchar_t wtext[len+1];
	len = mbstowcs(wtext, text, len);
	*(wtext+len) = 0x0; // text end

	sftd_draw_wtext(font->font, x, y, color, size, wtext);

	return 0;
}

/***
Draw a text with a new line after a certain number of pixels.
Warning: No UTF32 support.
@function wrappedText
@tparam integer x text drawing origin horizontal coordinate, in pixels
@tparam integer y text drawing origin vertical coordinate, in pixels
@tparam string text the text to draw
@tparam integer width width of a line, in pixels
@tparam[opt=9] integer size drawing size, in pixels
@tparam[opt=default color] integer color drawing color
@tparam[opt=default font] font font to use
*/
static int gfx_wrappedText(lua_State *L) {
	int x = luaL_checkinteger(L, 1);
	int y = luaL_checkinteger(L, 2);
	size_t len;
	const char *text = luaL_checklstring(L, 3, &len);
	unsigned int lineWidth = luaL_checkinteger(L, 4);
	
	int size = luaL_optinteger(L, 5, 9);
	u32 color = luaL_optinteger(L, 6, color_default);
	font_userdata *font = luaL_testudata(L, 7, "LFont");
	if (font == NULL) {
		lua_getfield(L, LUA_REGISTRYINDEX, "LFontDefault");
		font = luaL_testudata(L, -1, "LFont");
		if (font == NULL) luaL_error(L, "No default font set and no font object passed");
	}
	if (font->font == NULL) luaL_error(L, "The font object was unloaded");

	// Wide caracters support. (wchar = UTF32 on 3DS.)
	// Disabled as sftd_draw_wtext_wrap() doesn't exist.
	/*wchar_t wtext[len+1];
	len = mbstowcs(wtext, text, len);
	*(wtext+len) = 0x0; // text end */
	
	sftd_draw_text_wrap(font->font, x, y, color, size, lineWidth, text);
	
	return 0;
}

/***
Calculate the size of a text draw with `wrappedText`.
@function calcBoundingBox
@tparam string text The text to check
@tparam integer lineWidth width of a line, in pixels
@tparam[opt=9] integer size drawing size, in pixels
@tparam[opt=default font] font font to use
@treturn integer width of the text, in pixels
@treturn integer height of the text, in pixels
*/
static int gfx_calcBoundingBox(lua_State *L) {
	size_t len;
	const char *text = luaL_checklstring(L, 1, &len);
	unsigned int lineWidth = luaL_checkinteger(L, 2);
	int size = luaL_optinteger(L, 3, 9);
	font_userdata *font = luaL_testudata(L, 4, "LFont");
	if (font == NULL) {
		lua_getfield(L, LUA_REGISTRYINDEX, "LFontDefault");
		font = luaL_testudata(L, -1, "LFont");
		if (font == NULL) luaL_error(L, "No default font set and no font object passed");
	}
	if (font->font == NULL) luaL_error(L, "The font object was unloaded");
	
	int w, h = 0;
	sftd_calc_bounding_box(&w, &h, font->font, size, lineWidth, text);
	
	lua_pushinteger(L, w);
	lua_pushinteger(L, h);
	return 2;
}

// Functions
static const struct luaL_Reg gfx_lib[] = {
	{ "start",           gfx_start           },
	{ "stop",            gfx_stop            },
	{ "render",          gfx_render          },
	{ "getFPS",          gfx_getFPS          },
	{ "set3D",           gfx_set3D           },
	{ "get3D",           gfx_get3D           },
	{ "setVBlankWait",   gfx_setVBlankWait   },
	{ "waitForVBlank",   gfx_waitForVBlank   },
	{ "vramSpaceFree",   gfx_vramSpaceFree   },
	{ "line",            gfx_line            },
	{ "point",           gfx_point           },
	{ "rectangle",       gfx_rectangle       },
	{ "circle",          gfx_circle          },
	{ "text",            gfx_text            },
	{ "wrappedText",     gfx_wrappedText     },
	{ "calcBoundingBox", gfx_calcBoundingBox },
	{ NULL, NULL }
};

// Constants
struct { char *name; int value; } gfx_constants[] = {
	/***
	Constant used to select the top screen.
	It is equal to `0`.
	@field GFX_TOP
	*/
	{ "GFX_TOP",       GFX_TOP    },
	/***
	Constant used to select the bottom screen.
	It is equal to `1`.
	@field GFX_BOTTOM
	*/
	{ "GFX_BOTTOM",    GFX_BOTTOM },
	/***
	Constant used to select the left eye.
	It is equal to `0`.
	@field GFX_LEFT
	*/
	{ "GFX_LEFT",      GFX_LEFT   },
	/***
	Constant used to select the right eye.
	It is equal to `1`.
	@field GFX_RIGHT
	*/
	{ "GFX_RIGHT",     GFX_RIGHT  },
	/***
	The top screen height, in pixels.
	It is equal to `240`.
	@field TOP_HEIGHT
	*/
	{ "TOP_HEIGHT",    240        },
	/***
	The top screen width, in pixels.
	It is equal to `400`.
	@field TOP_WIDTH
	*/
	{ "TOP_WIDTH",     400        },
	/***
	The bottom screen height, in pixels.
	It is equal to `240`.
	@field BOTTOM_HEIGHT
	*/
	{ "BOTTOM_HEIGHT", 240        },
	/***
	The bottom screen width, in pixels.
	It is equal to `320`.
	@field BOTTOM_WIDTH
	*/
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

	isGfxInitialized = true;

	luaL_requiref(L, "ctr.gfx", luaopen_gfx_lib, 0);
}

void unload_gfx_lib(lua_State *L) {
	for (int i = 0; gfx_libs[i].name; i++) {
		if (gfx_libs[i].unload) gfx_libs[i].unload(L);
	}

	sftd_fini();
	sf2d_fini();
}
