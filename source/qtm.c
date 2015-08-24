#include <3ds.h>
#include <3ds/types.h>
#include <3ds/services/qtm.h>

#include <lapi.h>
#include <lauxlib.h>

static int qtm_init(lua_State *L) {
	qtmInit();

	return 0;
}

static int qtm_shutdown(lua_State *L) {
	qtmExit();

	return 0;
}

static int qtm_checkInitialized(lua_State *L) {
	bool isInit = qtmCheckInitialized();

	lua_pushboolean(L, isInit);
	return 1;
}

static int qtm_getHeadtrackingInfo(lua_State *L) {
	return 0;
}

static int qtm_checkHeadFullyDetected(lua_State *L) {
	return 0;
}

static int qtm_convertCoordToScreen(lua_State *L) {
	return 0;
}

// module
static const struct luaL_Reg qtm_functions[] = {
	{"init",                   qtm_init                  },
	{"shutdown",               qtm_shutdown              },
	{"checkInitialized",       qtm_checkInitialized      },
	{"getHeadtrackingInfo",    qtm_getHeadtrackingInfo   },
	{"checkHeadFullyDetected", qtm_checkHeadFullyDetected},
	{"convertCoordToScreen",   qtm_convertCoordToScreen  },
	{NULL, NULL}
};

int luaopen_qtm_lib(lua_State *L) {
	luaL_newlib(L, qtm_functions);
	return 1;
}

void load_qtm_lib(lua_State *L) {
	luaL_requiref(L, "ctr.qtm", luaopen_qtm_lib, false);
}
