#include <3ds/types.h>
#include <3ds/services/apt.h>
#include <3ds/os.h>

#include <lua.h>
#include <lauxlib.h>

void load_gfx_lib(lua_State *L);
void load_news_lib(lua_State *L);
void load_ptm_lib(lua_State *L);
void load_hid_lib(lua_State *L);
void load_ir_lib(lua_State *L);
void load_fs_lib(lua_State *L);
void load_httpc_lib(lua_State *L);
void load_qtm_lib(lua_State *L);
//void load_cam_lib(lua_State *L);

void unload_gfx_lib(lua_State *L);
void unload_hid_lib(lua_State *L);
void unload_fs_lib(lua_State *L);
void unload_httpc_lib(lua_State *L);

static int ctr_run(lua_State *L) {
	lua_pushboolean(L, aptMainLoop());

	return 1;
}

static int ctr_time(lua_State *L) {
	lua_pushinteger(L, osGetTime());

	return 1;
}

// Functions
static const struct luaL_Reg ctr_lib[] = {
	{ "run",  ctr_run },
	{ "time", ctr_time},
	{ NULL, NULL }
};

// Subtables
struct { char *name; void (*load)(lua_State *L); void (*unload)(lua_State *L); } ctr_libs[] = {
	{ "gfx",   load_gfx_lib,   unload_gfx_lib   },
	{ "news",  load_news_lib,  NULL             },
	{ "ptm",   load_ptm_lib,   NULL             },
	{ "hid",   load_hid_lib,   unload_hid_lib   },
	{ "ir",    load_ir_lib,    NULL             },
	{ "fs",    load_fs_lib,    unload_fs_lib    },
	{ "httpc", load_httpc_lib, unload_httpc_lib },
	{ "qtm",   load_qtm_lib,   NULL             },
//	{ "cam",   load_cam_lib,   NULL             },
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
	luaL_requiref(L, "ctr", luaopen_ctr_lib, 0);
}

void unload_ctr_lib(lua_State *L) {
	for (int i = 0; ctr_libs[i].name; i++) {
		if (ctr_libs[i].unload) ctr_libs[i].unload(L);
	}
}
