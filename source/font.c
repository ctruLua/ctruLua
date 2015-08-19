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

	font_default = sftd_load_font_mem(vera_ttf, vera_ttf_size); // Load default font

	return 1;
}

void load_font_lib(lua_State *L) {
	luaL_requiref(L, "ctr.gfx.font", luaopen_font_lib, false);
}