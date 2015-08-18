#include <3ds/types.h>
#include <3ds/services/ptm.h>

#include <lua.h>
#include <lauxlib.h>

static Handle *ptmHandle;

static int ptm_init(lua_State *L) {
	ptmInit();
	
	return 0;
}

static int ptm_shutdown(lua_State *L) {
	ptmExit();
	
	return 0;
}

static int ptm_getShellState(lua_State *L) {
	u8 *out = 0;
	PTMU_GetShellState(ptmHandle, out);
	
	lua_pushinteger(L, (lua_Integer)out);
	
	return 1;
}

static int ptm_getBatteryLevel(lua_State *L) {
	u8 *out = 0;
	PTMU_GetBatteryLevel(ptmHandle, out);
	
	lua_pushinteger(L, (lua_Integer)out);
	
	return 1;
}

static int ptm_getBatteryChargeState(lua_State *L) {
	u8 *out = 0;
	PTMU_GetBatteryChargeState(ptmHandle, out);
	
	lua_pushinteger(L, (lua_Integer)out);
	
	return 1;
}

static int ptm_getPedometerState(lua_State *L) {
	u8 *out = 0;
	PTMU_GetPedometerState(ptmHandle, out);
	
	lua_pushinteger(L, (lua_Integer)out);
	
	return 1;
}

static int ptm_getTotalStepCount(lua_State *L) {
	u32 *steps = 0;
	PTMU_GetTotalStepCount(ptmHandle, steps);
	
	lua_pushinteger(L, (lua_Integer)steps);
	
	return 1;
}

static const struct luaL_Reg ptm_lib[] = {
	{"init",                  ptm_init                 },
	{"shutdown",              ptm_shutdown             },
	{"getShellState",         ptm_getShellState        },
	{"getBatteryLevel",       ptm_getBatteryLevel      },
	{"getBatteryChargeState", ptm_getBatteryChargeState},
	{"getPedometerState",     ptm_getPedometerState    },
	{"getTotalStepCount",     ptm_getTotalStepCount    },
	{NULL, NULL}
};

int luaopen_ptm_lib(lua_State *L) {
	luaL_newlib(L, ptm_lib);
	return 1;
}

void load_ptm_lib(lua_State *L) {
	luaL_requiref(L, "ctr.ptm", luaopen_ptm_lib, 0);
}
