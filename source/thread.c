/***
The `thread` module.
@module ctr.thread
@usage local thread = require("ctr.thread")
*/

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#include <3ds/svc.h>
#include <3ds/types.h>
#include <3ds/thread.h>
#include <3ds/services/apt.h>	

#include <malloc.h>
#include <string.h>

void load_ctr_lib(lua_State *L);

typedef struct {
	Thread thread;
	const char *code;
	char *error;
} thread_userdata;

void entryPoint(void *thread) {
	lua_State *T = luaL_newstate();
	luaL_openlibs(T);
	load_ctr_lib(T);
	
	if (luaL_dostring(T, ((thread_userdata*)thread)->code)) {
		const char* lerror = luaL_checkstring(T, -1);
		((thread_userdata*)thread)->error = malloc(strlen(lerror)+1);
		strcpy(((thread_userdata*)thread)->error, lerror);
	}
	
	int exitCode = 0;
	if (lua_isinteger(T, -1)) {
		exitCode = lua_tointeger(T, -1);
	}
	lua_close(T);
	threadExit(exitCode);
}

// module
/***
Set the maximum CPU time allocated to threads on CPU #1.
@function setCpuLimit
@tparam number time in percents.
*/
static int thread_setCpuLimit(lua_State *L) {
	u32 percent = luaL_checkinteger(L, 1);
	APT_SetAppCpuTimeLimit(percent);
	
	return 0;
}

/***
Start a new thread.
@function start
@tparam string code Lua code to load in the new thread. May not work with dumped functions.
@tparam[opt=0] number cpu must be >= 0 and < 2 for 3ds or < 4 for new3ds
@tparam[opt=0x27] number priority must be > 0x18 and < 0x3f; the lower is higher.
@tparam[opt=0x100000] number stacksize size of the stack, increase it in case of OoM
@treturn thread a new thread object
*/
static int thread_start(lua_State *L) {
	const char* code = luaL_checkstring(L, 1);
	s32 processor = luaL_optinteger(L, 2, 0);
	s32 priority = luaL_optinteger(L, 3, 0x27);
	size_t stackSize = luaL_optinteger(L, 4, 0x100000);
	
	thread_userdata *thread = lua_newuserdata(L, sizeof(thread_userdata*));
	luaL_getmetatable(L, "LThread");
	lua_setmetatable(L, -2);
	
	thread->code = code;
	thread->error = NULL;
	thread->thread = threadCreate(entryPoint, thread, stackSize, priority, processor, true);
	
	return 1;
}

/***
Wait for a thread to finish.
@function :join
@tparam[opt=2^32-1] number timeout in ns
@treturn number exit code
@treturn string last error, or nil
*/
static int thread_join(lua_State *L) {
	thread_userdata *thread = luaL_checkudata(L, 1, "LThread");
	u64 timeout = luaL_optinteger(L, 2, 4294967295);
	
	threadJoin(thread->thread, timeout);
	
	lua_pushinteger(L, threadGetExitCode(thread->thread));
	if (thread->error != NULL) {
		lua_pushstring(L, thread->error);
		return 2;
	}
	return 1;
}

/***
Return the last error of a thread.
@function :lastError
@treturn string last error, or nil
*/
static int thread_lastError(lua_State *L) {
	thread_userdata *thread = luaL_checkudata(L, 1, "LThread");
	
	if (thread->error == NULL) {
		lua_pushnil(L);
	} else {
		lua_pushstring(L, thread->error);
	}
	return 1;
}

/***
Destroy a finished thread.
@function :destroy
*/
static int thread_destroy(lua_State *L) {
	thread_userdata *thread = luaL_checkudata(L, 1, "LThread");
	
	threadFree(thread->thread);
	free(thread->error);
	
	return 0;
}

static const struct luaL_Reg thread_lib[] = {
	{"start", thread_start},
	{"setCpuLimit", thread_setCpuLimit},
	{NULL, NULL}
};

static const struct luaL_Reg thread_methods[] = {
	{"join", thread_join},
	{"lastError", thread_lastError},
	{"destroy", thread_destroy},
	{"__gc", thread_destroy},
	{NULL, NULL}
};

int luaopen_thread_lib(lua_State *L) {
	luaL_newmetatable(L, "LThread");
	lua_pushvalue(L, -1);
	lua_setfield(L, -2, "__index");
	luaL_setfuncs(L, thread_methods, 0);
	
	luaL_newlib(L, thread_lib);
	return 1;
}

void load_thread_lib(lua_State *L) {
	luaL_requiref(L, "ctr.thread", luaopen_thread_lib, 0);
}
