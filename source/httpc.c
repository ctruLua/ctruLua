#include <stdlib.h>

#include <3ds.h>
#include <3ds/types.h>
#include <3ds/services/httpc.h>

#include <lapi.h>
#include <lauxlib.h>

static int httpc_init(lua_State *L) {
	httpcInit();
	return 0;
}

static int httpc_shutdown(lua_State *L) {
	httpcExit();
	return 0;
}

static int httpc_context(lua_State *L) {
	httpcContext *context;
	context = (httpcContext*)lua_newuserdata(L, sizeof(*context));
	luaL_getmetatable(L, "LHTTPC");
	lua_setmetatable(L, -2);

	return 1;
}

static int httpc_open(lua_State *L) {
	httpcContext *context = lua_touserdata(L, 1);
	char *url = (char*)luaL_checkstring(L, 2);
	Result ret = 0;
	
	ret = httpcOpenContext(context, url, 0);
	
	lua_pushinteger(L, ret);
	return 1;
}

static int httpc_beginRequest(lua_State *L) {
	httpcContext *context = lua_touserdata(L, 1);
	Result ret = 0;
	
	ret = httpcBeginRequest(context);
	
	lua_pushinteger(L, ret);
	return 1;
}

static int httpc_getStatusCode(lua_State *L) {
	httpcContext *context = lua_touserdata(L, 1);
	u32 statusCode = 0;

	Result ret = httpcGetResponseStatusCode(context, &statusCode, 0);
	if (ret != 0) {
		lua_pushnil(L);
		lua_pushinteger(L, ret);
		return 2;
	}
	lua_pushinteger(L, statusCode);
	return 1;
}

static int httpc_getDownloadSize(lua_State *L) {
	httpcContext *context = lua_touserdata(L, 1);
	u32 contentSize = 0;
	
	httpcGetDownloadSizeState(context, NULL, &contentSize);
	
	lua_pushinteger(L, contentSize);
	return 1;
}

static int httpc_downloadData(lua_State *L) {
	httpcContext *context = lua_touserdata(L, 1);
	u32 size = 0;
	httpcGetDownloadSizeState(context, NULL, &size);
	u8 *buff = (u8*)malloc(size);
	
	httpcDownloadData(context, buff, size, NULL);
	
	lua_pushstring(L, (char*)buff);
	return 1;
}

static int httpc_close(lua_State *L) {
	httpcContext *context = lua_touserdata(L, 1);
	
	httpcCloseContext(context);
	
	return 0;
}

// object
static const struct luaL_Reg httpc_methods[] = {
	{"open",            httpc_open},
	{"beginRequest",    httpc_beginRequest},
	{"getStatusCode",   httpc_getStatusCode},
	{"getDownloadSize", httpc_getDownloadSize},
	{"downloadData",    httpc_downloadData},
	{"close",           httpc_close},
	{NULL, NULL}
};

// module
static const struct luaL_Reg httpc_functions[] = {
	{"init",     httpc_init    },
	{"shutdown", httpc_shutdown},
	{"context",  httpc_context },
	{NULL, NULL}
};

int luaopen_httpc_lib(lua_State *L) {
	luaL_newmetatable(L, "LHTTPC");
	lua_pushvalue(L, -1);
	lua_setfield(L, -2, "__index");
	luaL_setfuncs(L, httpc_methods, 0);
	
	luaL_newlib(L, httpc_functions);
	
	return 1;
}

void load_httpc_lib(lua_State *L) {
	luaL_requiref(L, "ctr.httpc", luaopen_httpc_lib, false);
}

