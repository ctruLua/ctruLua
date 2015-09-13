/***
The `ptm` module.
@module ctr.ptm
@usage local ptm = require("ctr.ptm")
*/
#include <3ds/types.h>
#include <3ds/services/ptm.h>

#include <lua.h>
#include <lauxlib.h>

static Handle *ptmHandle;

/***
Initialize the PTM module.
@function init
*/
static int ptm_init(lua_State *L) {
	ptmInit();
	
	return 0;
}

/***
Disable the PTM module.
@function shutdown
*/
static int ptm_shutdown(lua_State *L) {
	ptmExit();
	
	return 0;
}

/***
Return the shell state. Don't care about this.
@function getShellState
@treturn number shell state
*/
static int ptm_getShellState(lua_State *L) {
	u8 out = 0;
	PTMU_GetShellState(ptmHandle, &out);
	
	lua_pushinteger(L, out);
	
	return 1;
}

/***
Return the battery level.
@function getBatteryLevel
@treturn number battery level (`5`: fully charged; `0`: empty)
*/
static int ptm_getBatteryLevel(lua_State *L) {
	u8 out = 0;
	PTMU_GetBatteryLevel(ptmHandle, &out);
	
	lua_pushinteger(L, out);
	
	return 1;
}

/***
Return whether or not the battery is charging.
@function getBatteryChargeState
@treturn boolean `true` if the battery is charging
*/
static int ptm_getBatteryChargeState(lua_State *L) {
	u8 out = 0;
	PTMU_GetBatteryChargeState(ptmHandle, &out);
	
	lua_pushboolean(L, out);
	
	return 1;
}

/***
Return whether or not the pedometer is counting.
@function getPedometerState
@treturn boolean `true` if the pedometer is counting
*/
static int ptm_getPedometerState(lua_State *L) {
	u8 out = 0;
	PTMU_GetPedometerState(ptmHandle, &out);
	
	lua_pushboolean(L, out);
	
	return 1;
}

/***
Return the total steps taken with the system.
@function getTotalStepCount
@treturn number step count
*/
static int ptm_getTotalStepCount(lua_State *L) {
	u32 steps = 0;
	PTMU_GetTotalStepCount(ptmHandle, &steps);
	
	lua_pushinteger(L, steps);
	
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
