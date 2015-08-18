#include <3ds/types.h>
#include <3ds/services/apt.h>

#include <lua.h>

static int os_run(lua_State *L) {
	bool run = aptMainLoop();

	lua_pushboolean(L, run);

	return 1;
}

void load_os_lib(lua_State *L) {
	lua_getglobal(L, "os");

	lua_pushcfunction(L, os_run);
	lua_setfield(L, -2, "run");
	
	lua_pop(L, 1);

	return;
}