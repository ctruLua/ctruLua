#include <3ds.h>
#include <3ds/types.h>
#include <3ds/services/qtm.h>

#include <lapi.h>
#include <lauxlib.h>

typedef struct {
	qtmHeadtrackingInfo *info;
} qtm_userdata;

static int qtm_init(lua_State *L) {
	Result ret = qtmInit();
	if (ret) {
		lua_pushboolean(L, false);
		lua_pushinteger(L, ret);
		return 2;
	}
	
	lua_pushboolean(L, true);
	return 1;
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
	qtm_userdata *data = lua_newuserdata(L, sizeof(*data));
	luaL_getmetatable(L, "LQTM");
	lua_setmetatable(L, -2);
	Result ret = qtmGetHeadtrackingInfo(0, data->info);
	if (ret) {
		lua_pushnil(L);
		return 1;
	}
	return 1;
}

static int qtm_checkHeadFullyDetected(lua_State *L) {
	qtm_userdata *info = luaL_checkudata(L, 1, "LQTM");
	lua_pushboolean(L, qtmCheckHeadFullyDetected(info->info));
	return 1;
}

static int qtm___index(lua_State *L) {
	qtm_userdata *info = luaL_checkudata(L, 1, "LQTM");
	lua_Integer index = luaL_checkinteger(L, 2);
	index = index - 1; // Lua index begins at 1
	if (index > 3 || index < 0) {
		lua_pushnil(L);
		lua_pushnil(L);
		return 2;
	}
	
	lua_pushnumber(L, info->info->coords0[index].x);
	lua_pushnumber(L, info->info->coords0[index].y);
	
	return 2;
}

static int qtm_convertCoordToScreen(lua_State *L) {
	qtm_userdata *info = luaL_checkudata(L, 1, "LQTM");
	lua_Integer index = luaL_checkinteger(L, 2);
	index = index - 1; // Lua index begins at 1
	if (index > 3 || index < 0) {
		lua_pushnil(L);
		lua_pushnil(L);
		return 2;
	}
	float screenWidth = luaL_optnumber(L, 3, 400.0f);
	float screenHeight = luaL_optnumber(L, 4, 320.0f);
	
	u32 x, y = 0;
	qtmConvertCoordToScreen(&info->info->coords0[index], &screenWidth, &screenHeight, &x, &y);
	
	lua_pushinteger(L, x);
	lua_pushinteger(L, y);
	
	return 2;
}

// object
static const struct luaL_Reg qtm_methods[] = {
	{"checkHeadFullyDetected", qtm_checkHeadFullyDetected},
	{"convertCoordToScreen",   qtm_convertCoordToScreen  },
	{"__index",                qtm___index               },
	{NULL, NULL}
};

// module functions
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
	luaL_newmetatable(L, "LQTM");
	lua_pushvalue(L, -1);
	lua_setfield(L, -2, "__index");
	luaL_setfuncs(L, qtm_methods, 0);
	
	luaL_newlib(L, qtm_functions);
	return 1;
}

void load_qtm_lib(lua_State *L) {
	luaL_requiref(L, "ctr.qtm", luaopen_qtm_lib, false);
}
