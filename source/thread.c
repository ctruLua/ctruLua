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
void load_pool_lib(lua_State *L);

typedef struct {
	enum {INT, NUM, STR, BOL, NIL} type;
	union {
		lua_Integer integer;
		lua_Number number;
		char* string;
		bool boolean;
	};
} poolEntry;

typedef struct {
	Thread thread;
	const char *code;
	char *error;
	poolEntry* pool;
	int poolSize; // in entries
} thread_userdata;

void entryPoint(void *thread) {
	if (((thread_userdata*)thread)->poolSize>0) {
		((thread_userdata*)thread)->pool = malloc(((thread_userdata*)thread)->poolSize*sizeof(poolEntry));
		if (((thread_userdata*)thread)->pool == NULL) {
			((thread_userdata*)thread)->error = "Out of memory.";
			threadExit(-1);
		} else {
			for (int i=0;i<((thread_userdata*)thread)->poolSize;i++) {
				((thread_userdata*)thread)->pool[i].type=NIL;
			}
		}
	}
	
	lua_State *T = luaL_newstate();
	luaL_openlibs(T);
	load_ctr_lib(T);
	load_pool_lib(T);
	
	lua_pushinteger(T, (u32)((thread_userdata*)thread));
	lua_setfield(T, LUA_REGISTRYINDEX, "LThreadSelf");
	
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
	if (((thread_userdata*)thread)->poolSize>0) {
		for (int i=0;i<((thread_userdata*)thread)->poolSize;i++) {
			if (((thread_userdata*)thread)->pool[i].type == STR) {
				free(((thread_userdata*)thread)->pool[i].string);
			}
		}
		free(((thread_userdata*)thread)->pool);
	}
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
@tparam[opt=0] number poolSize size of the RAM pool for the thread (used to communicate with the main thread)
@tparam[opt=0] number cpu must be >= 0 and < 2 for 3ds or < 4 for new3ds
@tparam[opt=0x27] number priority must be > 0x18 and < 0x3f; the lower is higher.
@tparam[opt=0x100000] number stacksize size of the stack, increase it in case of OoM
@treturn thread a new thread object
*/
static int thread_start(lua_State *L) {
	const char* code = luaL_checkstring(L, 1);
	s32 poolSize = luaL_optinteger(L, 2, 0);
	s32 processor = luaL_optinteger(L, 3, 0);
	s32 priority = luaL_optinteger(L, 4, 0x27);
	size_t stackSize = luaL_optinteger(L, 5, 0x100000);
	
	thread_userdata *thread = lua_newuserdata(L, sizeof(thread_userdata*));
	luaL_getmetatable(L, "LThread");
	lua_setmetatable(L, -2);
	
	thread->poolSize = poolSize;
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
	if (thread->poolSize > 0) {
		for (int i=0;i<thread->poolSize;i++) {
			if (thread->pool[i].type == STR) {
				free(thread->pool[i].string);
			}
		}
		free(thread->pool);
	}
	
	return 0;
}

static const struct luaL_Reg thread_methods[];

static int thread___index(lua_State *L) {
	thread_userdata* thread = luaL_checkudata(L, 1, "LThread");
	
	poolEntry* pool = thread->pool;
	
	if (lua_isstring(L, 2)) {
		const char *mname = lua_tostring(L, 2);
		for (u8 i=0;thread_methods[i].name;i++) {
			if (strcmp(thread_methods[i].name, mname) == 0) {
				lua_pushcfunction(L, thread_methods[i].func);
				return 1;
			}
		}
		lua_pushnil(L);
		return 1;
	} else if (lua_isinteger(L, 2)) {
		u32 addr = lua_tointeger(L, 2);
		if (addr > thread->poolSize || addr < 1) {
			lua_pushnil(L);
			return 1;
		}
		switch (pool[addr+1].type) {
			case INT:
				lua_pushinteger(L, pool[addr+1].integer);
				break;
			case NUM:
				lua_pushnumber(L, pool[addr+1].number);
				break;
			case STR:
				lua_pushstring(L, pool[addr+1].string);
				break;
			case BOL:
				lua_pushboolean(L, pool[addr+1].boolean);
				break;
			
			default:
				lua_pushnil(L);
				break;
		}
		return 1;
	} else {
		lua_pushnil(L);
	}
	return 1;
}

static int thread___newindex(lua_State *L) {
	thread_userdata* thread = luaL_checkudata(L, 1, "LThread");
	
	poolEntry* pool = thread->pool;
	
	if (lua_isinteger(L, 2)) {
		int addr = lua_tointeger(L, 2);
		if (addr > thread->poolSize || addr < 1) return 0;
		
		if (pool[addr+1].type == STR) {
			free(pool[addr+1].string);
		}
		
		switch (lua_type(L, 3)) {
			case LUA_TNUMBER:
				if (lua_isinteger(L, 3)) {
					pool[addr+1].type = INT;
					pool[addr+1].integer = lua_tointeger(L, 3);
				} else {
					pool[addr+1].type = NUM;
					pool[addr+1].number = lua_tonumber(L, 3);
				}
				break;
			case LUA_TSTRING:
				pool[addr+1].type = STR;
				const char* str = lua_tostring(L, 3);
				thread->pool[addr+1].string = malloc(strlen(str)+1);
				if (pool[addr+1].string == NULL) {
					luaL_error(L, "Memory allocation error");
					return 0;
				}
				strcpy(pool[addr+1].string, str);
				break;
			case LUA_TBOOLEAN:
				pool[addr+1].type = BOL;
				pool[addr+1].boolean = lua_toboolean(L, 3);
				break;
			default: // including LUA_TNIL
				pool[addr+1].type = NIL;
				break;
		}
	}
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
	{"__index", thread___index},
	{"__newindex", thread___newindex},
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

////////////////////////////////////////////////////////////////////////////////
// Pool lib for accessing the pool from the thread                            //
// Libception                                                                 //
// Documentation is in thread.pool.doc.c, because LDoc                        //
////////////////////////////////////////////////////////////////////////////////

static int pool_set(lua_State *L) {
	lua_getfield(L, LUA_REGISTRYINDEX, "LThreadSelf");
	thread_userdata* thread = (thread_userdata*)lua_tointeger(L, -1);
	poolEntry* pool = thread->pool;
	
	if (lua_isinteger(L, 1)) {
		int addr = lua_tointeger(L, 1);
		if (addr > thread->poolSize || addr < 1) return 0;
		
		if (pool[addr+1].type == STR) {
			free(pool[addr+1].string);
		}
		
		switch (lua_type(L, 2)) {
			case LUA_TNUMBER:
				if (lua_isinteger(L, 2)) {
					pool[addr+1].type = INT;
					pool[addr+1].integer = lua_tointeger(L, 2);
				} else {
					pool[addr+1].type = NUM;
					pool[addr+1].number = lua_tonumber(L, 2);
				}
				break;
			case LUA_TSTRING:
				pool[addr+1].type = STR;
				const char* str = lua_tostring(L, 2);
				thread->pool[addr+1].string = malloc(strlen(str)+1);
				if (pool[addr+1].string == NULL) {
					luaL_error(L, "Memory allocation error");
					return 0;
				}
				strcpy(pool[addr+1].string, str);
				break;
			case LUA_TBOOLEAN:
				pool[addr+1].type = BOL;
				pool[addr+1].boolean = lua_toboolean(L, 2);
				break;
			case LUA_TNIL: // including LUA_TNIL
				pool[addr+1].type = NIL;
				break;
			default:
				luaL_error(L, "Unsupported type: %s", lua_typename(L, lua_type(L, 2)));
		}
	}
	return 0;
}

static int pool_get(lua_State *L) {
	if (!lua_isinteger(L, 1)) {
		lua_pushnil(L);
		return 1;
	}
	lua_getfield(L, LUA_REGISTRYINDEX, "LThreadSelf");
	thread_userdata* thread = (thread_userdata*)lua_tointeger(L, -1);
	poolEntry* pool = thread->pool;
	
	u32 addr = lua_tointeger(L, 1);
	if (addr > thread->poolSize || addr < 1) {
		lua_pushnil(L);
		return 1;
	}
	switch (pool[addr+1].type) {
		case INT:
			lua_pushinteger(L, pool[addr+1].integer);
			break;
		case NUM:
			lua_pushnumber(L, pool[addr+1].number);
			break;
		case STR:
			lua_pushstring(L, pool[addr+1].string);
			break;
		case BOL:
			lua_pushboolean(L, pool[addr+1].boolean);
			break;
		
		default:
			lua_pushnil(L);
			break;
	}
	return 1;
}

static const struct luaL_Reg pool_lib[] = {
	{"set", pool_set},
	{"get", pool_get},
	{NULL, NULL}
};

int luaopen_pool_lib(lua_State *L) {
	luaL_newlib(L, pool_lib);
	return 1;
}

void load_pool_lib(lua_State *L) {
	luaL_requiref(L, "ctr.thread.pool", luaopen_pool_lib, 0);
}
