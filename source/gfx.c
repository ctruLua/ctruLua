/***
The `gfx` module.
@module ctr.gfx
@usage local gfx = require("ctr.gfx")
*/
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <sf2d.h>
#include <sftd.h>

//#include <3ds/vram.h>
//#include <3ds/services/gsp.h>
#include <3ds/console.h>

#include <lua.h>
#include <lauxlib.h>

#include "gfx.h"
#include "font.h"
#include "texture.h"

typedef struct {
	sf2d_rendertarget *target;
} target_userdata;

bool isGfxInitialized = false;
bool is3DEnabled = false; //TODO: add a function for this in the ctrulib/sf2dlib.

// The scissor-test state, as defined in Lua code. When you apply a new scissor in C, remember to get back to this state to avoid unexpected behaviour.
scissor_state lua_scissor = {
	GPU_SCISSOR_DISABLE,
	0, 0,
	0, 0
};

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
Start drawing to a screen/target.
Must be called before any draw operation.
@function start
@tparam number/target screen the screen or target to draw to (`gfx.TOP`, `gfx.BOTTOM`, or render target)
@tparam[opt=gfx.LEFT] number eye the eye to draw to (`gfx.LEFT` or `gfx.RIGHT`)
*/
static int gfx_start(lua_State *L) {
	if (lua_isinteger(L, 1)) {
		u8 screen = luaL_checkinteger(L, 1);
		u8 eye = luaL_optinteger(L, 2, GFX_LEFT);
	
		sf2d_start_frame(screen, eye);
	} else if (lua_isuserdata(L, 1)) {
		target_userdata *target = luaL_checkudata(L, 1, "LTarget");
		
		sf2d_start_frame_target(target->target);
	}

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
@tparam[opt=1] number width line's thickness, in pixels
@tparam[opt=default color] integer color drawing color
*/
static int gfx_line(lua_State *L) {
	int x1 = luaL_checkinteger(L, 1);
	int y1 = luaL_checkinteger(L, 2);
	int x2 = luaL_checkinteger(L, 3);
	int y2 = luaL_checkinteger(L, 4);
	float width = luaL_optnumber(L, 5, 1.0f);
	
	u32 color = luaL_optinteger(L, 6, color_default);
	
	sf2d_draw_line(x1, y1, x2, y2, width, color);

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
@tparam[opt=default size] integer size drawing size, in pixels
@tparam[opt=default color] integer color drawing color
@tparam[opt=default font] font font to use
*/
static int gfx_text(lua_State *L) {
	int x = luaL_checkinteger(L, 1);
	int y = luaL_checkinteger(L, 2);
	size_t len;
	const char *text = luaL_checklstring(L, 3, &len);

	int size = luaL_optinteger(L, 4, textSize);
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
@tparam[opt=default Size] integer size drawing size, in pixels
@tparam[opt=default color] integer color drawing color
@tparam[opt=default font] font font to use
*/
static int gfx_wrappedText(lua_State *L) {
	int x = luaL_checkinteger(L, 1);
	int y = luaL_checkinteger(L, 2);
	size_t len;
	const char *text = luaL_checklstring(L, 3, &len);
	unsigned int lineWidth = luaL_checkinteger(L, 4);
	
	int size = luaL_optinteger(L, 5, textSize);
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
@tparam[opt=default size] integer size drawing size, in pixels
@tparam[opt=default font] font font to use
@treturn integer width of the text, in pixels
@treturn integer height of the text, in pixels
*/
static int gfx_calcBoundingBox(lua_State *L) {
	size_t len;
	const char *text = luaL_checklstring(L, 1, &len);
	unsigned int lineWidth = luaL_checkinteger(L, 2);
	int size = luaL_optinteger(L, 3, textSize);
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

/***
Enables or disable the scissor test.
When the scissor test is enabled, the drawing area will be limited to a specific rectangle, every pixel drawn outside will be discarded.
Calls this function without argument to disable the scissor test.
@function scissor
@tparam integer x scissor rectangle origin horizontal coordinate, in pixels
@tparam integer y scissor rectangle origin vertical coordinate, in pixels
@tparam integer width scissor rectangle width, in pixels
@tparam integer height scissor rectangle height, in pixels
@tparam[opt=false] boolean invert if true the scissor will be inverted (will draw only outside of the rectangle)
*/
static int gfx_scissor(lua_State *L) {
	if (lua_gettop(L) == 0) {
		lua_scissor.mode = GPU_SCISSOR_DISABLE;
		lua_scissor.x = 0;
		lua_scissor.y = 0;
		lua_scissor.width = 0;
		lua_scissor.height = 0;
	} else {
		lua_scissor.x = luaL_checkinteger(L, 1);
		lua_scissor.y = luaL_checkinteger(L, 2);
		lua_scissor.width = luaL_checkinteger(L, 3);
		lua_scissor.height = luaL_checkinteger(L, 4);
		lua_scissor.mode = lua_toboolean(L, 5) ? GPU_SCISSOR_INVERT : GPU_SCISSOR_NORMAL;
	}

	sf2d_set_scissor_test(lua_scissor.mode, lua_scissor.x, lua_scissor.y, lua_scissor.width, lua_scissor.height);

	return 0;
}

/***
__Work in progress__. Create a render target. Don't use it.
@function target
@tparam integer width
@tparam integer height
@treturn target
*/
static int gfx_target(lua_State *L) {
	int width = luaL_checkinteger(L, 1);
	int height = luaL_checkinteger(L, 2);
	int wpo2 = 0, hpo2 = 0;
	for (;width>pow(2,wpo2);wpo2++);
	width = pow(2,wpo2);
	for (;height>pow(2,hpo2);hpo2++);
	height = pow(2,hpo2);
	
	target_userdata *target;
	target = (target_userdata*)lua_newuserdata(L, sizeof(*target));
	
	luaL_getmetatable(L, "LTarget");
	lua_setmetatable(L, -2);
	
	target->target = sf2d_create_rendertarget(width, height);
	
	return 1;
}

/***
Render targets
@section target
*/

/***
Clear a target to a specified color.
@function :clear
@tparam[opt=default color] integer color color to fill the target with
*/
static int gfx_target_clear(lua_State *L) {
	target_userdata *target = luaL_checkudata(L, 1, "LTarget");
	u32 color = luaL_optinteger(L, 2, color_default);
	
	sf2d_clear_target(target->target, color);
	
	return 0;
}

/***
Destroy a target.
@function :destroy
*/
static int gfx_target_destroy(lua_State *L) {
	target_userdata *target = luaL_checkudata(L, 1, "LTarget");
	
	sf2d_free_target(target->target);
	
	return 0;
}

static const struct luaL_Reg target_methods[];
/***

*/
static int gfx_target___index(lua_State *L) {
	target_userdata *target = luaL_checkudata(L, 1, "LTarget");
	const char* name = luaL_checkstring(L, 2);
	
	if (strcmp(name, "texture") == 0) {
		texture_userdata *texture;
		texture = (texture_userdata*)lua_newuserdata(L, sizeof(*texture));
		luaL_getmetatable(L, "LTexture");
		lua_setmetatable(L, -2);
		
		texture->texture = &(target->target->texture);
		texture->scaleX = 1.0f;
		texture->scaleY = 1.0f;
		texture->blendColor = 0xffffffff;
		
		return 1;
	} else if (strcmp(name, "duck") == 0) {
		sf2d_rendertarget *target = sf2d_create_rendertarget(64, 64);
		for(int i=0;;i++) {
			sf2d_clear_target(target, 0xff000000);
			sf2d_start_frame_target(target);
			sf2d_draw_fill_circle(i%380, i%200, 10, 0xff0000ff);
			sf2d_end_frame();
			//sf2d_texture_tile32(&target->texture);
			
			sf2d_start_frame(GFX_TOP, GFX_LEFT);
			sf2d_draw_texture(&target->texture, 10, 10);
			sf2d_end_frame();
			sf2d_swapbuffers();
		}
	} else {
		for (u8 i=0;target_methods[i].name;i++) {
			if (strcmp(target_methods[i].name, name) == 0) {
				lua_pushcfunction(L, target_methods[i].func);
				return 1;
			}
		}
	}
	
	lua_pushnil(L);
	return 1;
}

/***
Console
@section console
*/

/***
Initialize the console. You can print on it using print(), or any function that normally outputs to stdout.
Warning: you can't use a screen for both a console and drawing, you have to disable the console first.
@function console
@tparam[opt=gfx.TOP] number screen screen to draw the console on.
@tparam[opt=false] boolean debug enable stderr output on the console
*/
u8 consoleScreen = GFX_TOP;
static int gfx_console(lua_State *L) {
	consoleScreen = luaL_optinteger(L, 1, GFX_TOP);
	bool err = false;
	if (lua_isboolean(L, 2)) {
		err = lua_toboolean(L, 2);
	}
	
	consoleInit(consoleScreen, NULL);
	if (err)
		consoleDebugInit(debugDevice_CONSOLE);
	
	return 0;
}

/***
Clear the console.
@function clearConsole
*/
static int gfx_clearConsole(lua_State *L) {
	consoleClear();
	
	return 0;
}

/***
Disable the console.
@function disableConsole
*/
static int gfx_disableConsole(lua_State *L) {
	gfxSetScreenFormat(consoleScreen, GSP_BGR8_OES);
	gfxSetDoubleBuffering(consoleScreen, true);
	gfxSwapBuffersGpu();
	gspWaitForVBlank();
	
	return 0;
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
	{ "scissor",         gfx_scissor         },
	{ "target",          gfx_target          },
	{ "console",         gfx_console         },
	{ "clearConsole",    gfx_clearConsole    },
	{ "disableConsole",  gfx_disableConsole  },
	{ NULL, NULL }
};

// Render target
static const struct luaL_Reg target_methods[] = {
	{ "__index", gfx_target___index },
	{"clear",    gfx_target_clear   },
	{"destroy",  gfx_target_destroy },
	{"__gc",     gfx_target_destroy },
	{ NULL, NULL }
};

/***
Constants
@section constants
*/
// Constants
struct { char *name; int value; } gfx_constants[] = {
	/***
	Constant used to select the top screen.
	It is equal to `0`.
	@field TOP
	*/
	{ "TOP",           GFX_TOP    },
	/***
	Constant used to select the bottom screen.
	It is equal to `1`.
	@field BOTTOM
	*/
	{ "BOTTOM",        GFX_BOTTOM },
	/***
	Constant used to select the left eye.
	It is equal to `0`.
	@field LEFT
	*/
	{ "LEFT",          GFX_LEFT   },
	/***
	Constant used to select the right eye.
	It is equal to `1`.
	@field RIGHT
	*/
	{ "RIGHT",         GFX_RIGHT  },
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
	luaL_newmetatable(L, "LTarget");
	lua_pushvalue(L, -1);
	lua_setfield(L, -2, "__index");
	luaL_setfuncs(L, target_methods, 0);
	
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
	if (!isGfxInitialized) {
		sf2d_init();
		sftd_init();
	}

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
