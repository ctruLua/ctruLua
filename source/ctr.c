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
#include <3ds/services/hb.h>

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
void unload_news_lib(lua_State *L);

/***
The `ctr.ptm` module.
@table ptm
@see ctr.ptm
*/
void load_ptm_lib(lua_State *L);
void unload_ptm_lib(lua_State *L);

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
void unload_cfgu_lib(lua_State *L);

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
void unload_apt_lib(lua_State *L);

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
The `ctr.uds` module.
@table uds
@see ctr.uds
*/
void load_uds_lib(lua_State *L);
void unload_uds_lib(lua_State *L);

/***
The `ctr.swkbd` module.
@table swkbd
@see ctr.swkbd
*/
void load_swkbd_lib(lua_State *L);

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
Return the number of milliseconds spent since some point in time.
This can be used to measure a duration with milliseconds precision; however this can't be used to get the current time or date.
See Lua's os.date() for this use.
For various reasons (see the C source), this will actually returns a negative value.
@function time
@treturn number milliseconds
@usage
-- Measuring a duration:
local startTime = ctr.time()
-- do stuff
local duration = ctr.time() - startTime
*/
static int ctr_time(lua_State *L) {
	// osGetTime actually returns the number of seconds elapsed since 1st Jan 1900 00:00.
	// However, it returns a u64, we build Lua with 32bits numbers, and every number is signed in Lua, so this obvioulsy doesn't work
	// and actually returns a negative value. It still works for durations however. Because having the date and time with millisecond-presion
	// doesn't really seem useful and changing ctrµLua's API to work on 64bits numbers will take a long time, we choosed to keep this as-is.
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
	{ "news",   load_news_lib,   unload_news_lib  },
	{ "ptm",    load_ptm_lib,    unload_ptm_lib   },
	{ "hid",    load_hid_lib,    unload_hid_lib   },
	{ "ir",     load_ir_lib,     NULL             },
	{ "fs",     load_fs_lib,     unload_fs_lib    },
	{ "httpc",  load_httpc_lib,  unload_httpc_lib },
	{ "qtm",    load_qtm_lib,    NULL             },
	{ "cfgu",   load_cfgu_lib,   unload_cfgu_lib  },
	{ "socket", load_socket_lib, NULL             },
	{ "cam",    load_cam_lib,    NULL             },
	{ "audio",  load_audio_lib,  unload_audio_lib },
	{ "apt",    load_apt_lib,    unload_apt_lib   },
	{ "mic",    load_mic_lib,    NULL             },
	{ "thread", load_thread_lib, NULL             },
	{ "uds",    load_uds_lib,    unload_uds_lib   },
	{ "swkbd",  load_swkbd_lib,  NULL             },
	{ NULL, NULL, NULL }
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
	char* buff = "romfs:/";
	chdir(buff);
	#else
	char* buff = malloc(1024);
	getcwd(buff, 1024);
	#endif
	lua_pushstring(L, buff);
	lua_setfield(L, -2, "root");
	
	/***
	Whether or not ctrµLua has been launched with ninjhax
	@field hb
	*/
	if (!hbInit()) {
		hbExit();
		lua_pushboolean(L, true);
	} else {
		lua_pushboolean(L, false);
	}
	lua_setfield(L, -2, "hb");

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
