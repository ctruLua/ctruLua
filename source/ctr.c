#include <3ds/types.h>
#include <3ds/os.h>

#include <lua.h>
#include <lauxlib.h>

int load_os_lib(lua_State *L);
int load_gfx_lib(lua_State *L);
int load_news_lib(lua_State *L);
int load_ptm_lib(lua_State *L);
int load_hid_lib(lua_State *L);

static int ctr_time(lua_State *L) {
	lua_pushinteger(L, osGetTime());

	return 1;
}

// Functions
static const struct luaL_Reg ctr_lib[] = {
	{ "time", ctr_time},
	{ NULL, NULL }
};

// Subtables
struct { char *name; int (*load)(lua_State *L); } ctr_libs[] = {
	{ "gfx",  load_gfx_lib  },
	{ "news", load_news_lib },
	{ "ptm",  load_ptm_lib  },
	{ "hid",  load_hid_lib  },
	{ NULL, NULL }
};

int luaopen_ctr_lib(lua_State *L) {
	luaL_newlib(L, ctr_lib);

	for (int i = 0; ctr_libs[i].name; i++) {
		ctr_libs[i].load(L);
		lua_setfield(L, -2, ctr_libs[i].name);
	}

	return 1;
}

void load_ctr_lib(lua_State *L) {
	load_os_lib(L);
	luaL_requiref(L, "ctr", luaopen_ctr_lib, 0);
}
