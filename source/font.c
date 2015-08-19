#include <sftd.h>
#include "vera_ttf.h"

#include <lua.h>
#include <lauxlib.h>

sftd_font *font_default;

static const struct luaL_Reg font_lib[] = {
	{ NULL, NULL }
};

int luaopen_font_lib(lua_State *L) {
	luaL_newlib(L, font_lib);

	return 1;
}

void load_font_lib(lua_State *L) {
	font_default = sftd_load_font_mem(vera_ttf, vera_ttf_size); // Load default font

	luaL_requiref(L, "ctr.gfx.font", luaopen_font_lib, false);
}

void unload_font_lib() {
	sftd_free_font(font_default); // Unload current font
}