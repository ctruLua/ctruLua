/***
The `httpc` module.
@module ctr.httpc
@usage local httpc = require("ctr.httpc")
*/
#include <stdlib.h>
#include <string.h>

#include <3ds.h>
#include <3ds/types.h>
#include <3ds/services/httpc.h>

#include <lapi.h>
#include <lauxlib.h>

bool isHttpcInitialized = false;

/***
Create a HTTP Context.
@function context
@treturn context a new http context
*/
static int httpc_context(lua_State *L) {
	httpcContext context;
	Result ret = httpcOpenContext(&context, "http://google.com/", 0); // Initialization only.
	if (ret != 0) {
		lua_pushnil(L);
		lua_pushinteger(L, ret);
		return 2;
	}
	lua_newuserdata(L, sizeof(&context));
	luaL_getmetatable(L, "LHTTPC");
	lua_setmetatable(L, -2);

	return 1;
}

/***
context object
@section Methods
*/

/***
Open an url in the context.
@function :open
@tparam string url the url to open
*/
static int httpc_open(lua_State *L) {
	httpcContext *context = lua_touserdata(L, 1);
	char *url = (char*)luaL_checkstring(L, 2);
	Result ret = 0;
	
	ret = httpcOpenContext(context, url, 0);
	if (ret != 0) {
		lua_pushnil(L);
		lua_pushinteger(L, ret);
		return 2;
	}
	lua_pushboolean(L, true);
	return 1;
}

/***
Add a field in the request header.
@function :addRequestHeaderField
@tparam string name Name of the field
@tparam string value Value of the field
*/
static int httpc_addRequestHeaderField(lua_State *L) {
	httpcContext *context = lua_touserdata(L, 1);
	char *name = (char*)luaL_checkstring(L, 2);
	char *value = (char*)luaL_checkstring(L, 3);
	
	Result ret = httpcAddRequestHeaderField(context, name ,value);
	if (ret != 0) {
		lua_pushnil(L);
		lua_pushinteger(L, ret);
		return 2;
	}
	lua_pushboolean(L, true);
	return 1;
}

/***
Begin a request to get the content at the URL.
@function :beginRequest
*/
static int httpc_beginRequest(lua_State *L) {
	httpcContext *context = lua_touserdata(L, 1);
	Result ret = 0;
	
	ret = httpcBeginRequest(context);
	if (ret != 0) {
		lua_pushnil(L);
		lua_pushinteger(L, ret);
		return 2;
	}
	lua_pushboolean(L, true);
	return 1;
}

/***
Return the status code returned by the request.
@function :getStatusCode
@treturn number the status code
*/
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

/***
Return the amount of data to download.
@function :getDownloadSize
@treturn number size in (bytes)
*/
static int httpc_getDownloadSize(lua_State *L) {
	httpcContext *context = lua_touserdata(L, 1);
	u32 contentSize = 0;
	
	httpcGetDownloadSizeState(context, NULL, &contentSize);
	
	lua_pushinteger(L, contentSize);
	return 1;
}

/***
Download and return the data of the context.
@function :downloadData
@treturn string data
*/
static int httpc_downloadData(lua_State *L) {
	httpcContext *context = lua_touserdata(L, 1);
	u32 status = 0;
	Result ret = httpcGetResponseStatusCode(context, &status, 0);
	if (ret != 0) {
		lua_pushnil(L);
		lua_pushinteger(L, ret);
		return 2;
	}
	
	u32 size = 0;
	httpcGetDownloadSizeState(context, NULL, &size);
	u8 *buff = (u8*)malloc(size);
	
	ret = httpcDownloadData(context, buff, size, NULL);
	if (ret != 0) {
		lua_pushnil(L);
		lua_pushinteger(L, ret);
		return 2;
	}
	
	lua_pushstring(L, (char*)buff);
	//free(buff);
	lua_pushinteger(L, size); // only for test purposes.
	return 2;
}

/***
Close the context.
@function :close
*/
static int httpc_close(lua_State *L) {
	httpcContext *context = lua_touserdata(L, 1);
	
	httpcCloseContext(context);
	
	return 0;
}

// object
static const struct luaL_Reg httpc_methods[] = {
	{"open",                  httpc_open                 },
	{"addRequestHeaderField", httpc_addRequestHeaderField},
	{"beginRequest",          httpc_beginRequest         },
	{"getStatusCode",         httpc_getStatusCode        },
	{"getDownloadSize",       httpc_getDownloadSize      },
	{"downloadData",          httpc_downloadData         },
	{"close",                 httpc_close                },
	{NULL, NULL}
};

// module
static const struct luaL_Reg httpc_functions[] = {
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
	if (!isHttpcInitialized) {
		httpcInit();
		isHttpcInitialized = true;
	}
	
	luaL_requiref(L, "ctr.httpc", luaopen_httpc_lib, false);
}

void unload_httpc_lib(lua_State *L) {
	httpcExit();
}

