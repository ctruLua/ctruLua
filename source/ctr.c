/***
The `ctr` module.
@module ctr
@usage local ctr = require("ctr")
*/
#include <stdlib.h>
#include <unistd.h>

#include <3ds/types.h>
#include <3ds/services/apt.h>
#include <3ds/os.h>
#include <3ds/svc.h>

#include <lua.h>
#include <lauxlib.h>

/***
The `ctr.gfx` module.
@table gfx
@see ctr.gfx
*/
void load_gfx_lib(lua_State *L);
void unload_gfx_lib(lua_State *L);

/***
The `ctr.news` module.
@table news
@see ctr.news
*/
void load_news_lib(lua_State *L);

/***
The `ctr.ptm` module.
@table ptm
@see ctr.ptm
*/
void load_ptm_lib(lua_State *L);

/***
The `ctr.hid` module.
@table hid
@see ctr.hid
*/
void load_hid_lib(lua_State *L);
void unload_hid_lib(lua_State *L);

/***
The `ctr.ir` module.
@table ir
@see ctr.ir
*/
void load_ir_lib(lua_State *L);

/***
The `ctr.fs` module.
@table fs
@see ctr.fs
*/
void load_fs_lib(lua_State *L);
void unload_fs_lib(lua_State *L);

/***
The `ctr.httpc` module.
@table httpc
@see ctr.httpc
*/
void load_httpc_lib(lua_State *L);
void unload_httpc_lib(lua_State *L);

/***
The `ctr.qtm` module.
@table qtm
@see ctr.qtm
*/
void load_qtm_lib(lua_State *L);

/***
The `ctr.cfgu` module.
@table cfgu
@see ctr.cfgu
*/
void load_cfgu_lib(lua_State *L);

/***
The `ctr.socket` module.
@table socket
@see ctr.socket
*/
void load_socket_lib(lua_State *L);

/***
The `ctr.cam` module.
@table cam
@see ctr.cam
*/
void load_cam_lib(lua_State *L);

/***
The `ctr.audio` module.
@table audio
@see ctr.audio
*/
void load_audio_lib(lua_State *L);
void unload_audio_lib(lua_State *L);

/***
The `ctr.apt` module.
@table apt
@see ctr.apt
*/
void load_apt_lib(lua_State *L);

/***
The `ctr.mic` module.
@table mic
@see ctr.mic
*/
void load_mic_lib(lua_State *L);

/***
The `ctr.thread` module.
@table thread
@see ctr.thread
*/
void load_thread_lib(lua_State *L);

/***
Return whether or not the program should continue.
@function run
@treturn boolean `false` if the program should exist or `true` if it can continue
*/
static int ctr_run(lua_State *L) {
	lua_pushboolean(L, aptMainLoop());

	return 1;
}

/***
Return the number of milliseconds since 1st Jan 1900 00:00.
@function time
@treturn number milliseconds
*/
static int ctr_time(lua_State *L) {
	lua_pushinteger(L, osGetTime());

	return 1;
}

/***
Return a number of microseconds based on the system ticks.
@function utime
@treturn number microseconds
*/
static int ctr_utime(lua_State *L) {
	lua_pushinteger(L, svcGetSystemTick()/268.123480);
	
	return 1;
}

// Functions
static const struct luaL_Reg ctr_lib[] = {
	{ "run",   ctr_run  },
	{ "time",  ctr_time },
	{ "utime", ctr_utime},
	{ NULL, NULL }
};

// Subtables
struct { char *name; void (*load)(lua_State *L); void (*unload)(lua_State *L); } ctr_libs[] = {
	{ "gfx",    load_gfx_lib,    unload_gfx_lib   },
	{ "news",   load_news_lib,   NULL             },
	{ "ptm",    load_ptm_lib,    NULL             },
	{ "hid",    load_hid_lib,    unload_hid_lib   },
	{ "ir",     load_ir_lib,     NULL             },
	{ "fs",     load_fs_lib,     unload_fs_lib    },
	{ "httpc",  load_httpc_lib,  unload_httpc_lib },
	{ "qtm",    load_qtm_lib,    NULL             },
	{ "cfgu",   load_cfgu_lib,   NULL             },
	{ "socket", load_socket_lib, NULL             },
	{ "cam",    load_cam_lib,    NULL             },
	{ "audio",  load_audio_lib,  unload_audio_lib },
	{ "apt",    load_apt_lib,    NULL             },
	{ "mic",    load_mic_lib,    NULL             },
	{ "thread", load_thread_lib, NULL             },
	{ NULL, NULL }
};

int luaopen_ctr_lib(lua_State *L) {
	luaL_newlib(L, ctr_lib);

	for (int i = 0; ctr_libs[i].name; i++) {
		ctr_libs[i].load(L);
		lua_setfield(L, -2, ctr_libs[i].name);
	}
	
	/***
	Running version of ctrµLua. This string contains the exact name of the last (pre-)release tag.
	@field version
	*/
	lua_pushstring(L, CTR_VERSION);
	lua_setfield(L, -2, "version");
	/***
	Running build of ctrµLua. This string contains the last commit hash.
	@field build
	*/
	lua_pushstring(L, CTR_BUILD);
	lua_setfield(L, -2, "build");
	
	/***
	Root directory of ctrµLua. Contains the working directory where ctrµLua has been launched OR the romfs root if romfs has been enabled.
	@field root
	*/
	#ifdef ROMFS
	char* buff = "romfs:";
	#else
	char* buff = malloc(1024);
	getcwd(buff, 1024);
	#endif
	lua_pushstring(L, buff);
	lua_setfield(L, -2, "root");

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
