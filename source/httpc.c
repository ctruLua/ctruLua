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
#include <3ds/services/sslc.h>

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
@tparam[opt="GET"] string method method to use; can be `"GET"`, `"POST"`, `"HEAD"`, `"PUT"` or `"DELETE"`
@treturn[1] boolean `true` if everything went fine
@treturn[2] boolean `false` in case of error
@treturn[2] integer error code
*/
static int httpc_open(lua_State *L) {
	httpcContext *context = lua_touserdata(L, 1);
	char *url = (char*)luaL_checkstring(L, 2);
	char *smethod = (char*)luaL_optstring(L, 3, "GET");
	HTTPC_RequestMethod method = HTTPC_METHOD_GET; // default to GET
	if (strcmp(smethod, "POST")) {
		method = HTTPC_METHOD_POST;
	} else if (strcmp(smethod, "HEAD")) {
		method = HTTPC_METHOD_HEAD;
	} else if (strcmp(smethod, "PUT")) {
		method = HTTPC_METHOD_PUT;
	} else if (strcmp(smethod, "DELETE")) {
		method = HTTPC_METHOD_DELETE;
	}
	Result ret = 0;
	
	ret = httpcOpenContext(context, method, url, 0);
	if (ret != 0) {
		lua_pushboolean(L, false);
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
@treturn[1] boolean `true` if everything went fine
@treturn[2] boolean `false` in case of error
@treturn[2] integer error code
*/
static int httpc_addRequestHeaderField(lua_State *L) {
	httpcContext *context = lua_touserdata(L, 1);
	char *name = (char*)luaL_checkstring(L, 2);
	char *value = (char*)luaL_checkstring(L, 3);
	
	Result ret = httpcAddRequestHeaderField(context, name ,value);
	if (ret != 0) {
		lua_pushboolean(L, false);
		lua_pushinteger(L, ret);
		return 2;
	}
	lua_pushboolean(L, true);
	return 1;
}

/***
Begin a request to get the content at the URL.
@function :beginRequest
@treturn[1] boolean `true` if everything went fine
@treturn[2] boolean `false` in case of error
@treturn[2] integer error code
*/
static int httpc_beginRequest(lua_State *L) {
	httpcContext *context = lua_touserdata(L, 1);
	Result ret = 0;
	
	ret = httpcBeginRequest(context);
	if (ret != 0) {
		lua_pushboolean(L, false);
		lua_pushinteger(L, ret);
		return 2;
	}
	lua_pushboolean(L, true);
	return 1;
}

/***
Return the status code returned by the request.
@function :getStatusCode
@treturn[1] integer the status code
@treturn[2] nil in case of error
@treturn[2] integer error code
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
@treturn[1] string data
@treturn[2] nil in case of error
@treturn[2] integer error code
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
	//free(buff); FIXME we need to free this buffer at some point ?
	//lua_pushinteger(L, size); // only for test purposes.
	return 1;
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

/***
Add a POST form field to a HTTP context.
@function :addPostData
@tparam string name name of the field
@tparam string value value of the field 
*/
static int httpc_addPostData(lua_State *L) {
	httpcContext *context = lua_touserdata(L, 1);
	char *name = (char*)luaL_checkstring(L, 2);
	char *value = (char*)luaL_checkstring(L, 3);
	
	httpcAddPostDataAscii(context, name, value);
	
	return 0;
}

/***
Get a header field from a response.
@function :getResponseHeader
@tparam string name name of the header field to get
@tparam[opt=2048] number maximum size of the value to get
@treturn string field value
*/
static int httpc_getResponseHeader(lua_State *L) {
	httpcContext *context = lua_touserdata(L, 1);
	char *name = (char*)luaL_checkstring(L, 2);
	u32 maxSize = luaL_checkinteger(L, 3);
	char* value = 0;
	
	httpcGetResponseHeader(context, name, value, maxSize);
	
	lua_pushstring(L, value);
	return 1;
}

/***
Add a trusted RootCA cert to a context.
@function :addTrustedRootCA
@tparam string DER certificate
@treturn[1] boolean `true` if everything went fine
@treturn[2] boolean `false` in case of error
@treturn[2] integer error code
*/
static int httpc_addTrustedRootCA(lua_State *L) {
	httpcContext *context = lua_touserdata(L, 1);
	u32 certsize;
	u8* cert = (u8*)luaL_checklstring(L, 2, (size_t*)&certsize);
	
	Result ret = httpcAddTrustedRootCA(context, cert, certsize);
	if (ret != 0) {
		lua_pushboolean(L, false);
		lua_pushinteger(L, ret);
		return 2;
	}
	
	lua_pushboolean(L, true);
	return 1;
}

/***
Set SSL options for a context.
@function :setSSLOptions
@tparam boolean disableVerify disable server certificate verification if `true`
@tparam[opt=false] boolean tlsv10 use TLS v1.0 if `true`
*/
static int httpc_setSSLOptions(lua_State *L) {
	httpcContext *context = lua_touserdata(L, 1);
	
	bool disVer = lua_toboolean(L, 2);
	bool tsl10 = false;
	if (lua_isboolean(L, 3))
		tsl10 = lua_toboolean(L, 3);
	
	httpcSetSSLOpt(context, (disVer?SSLCOPT_DisableVerify:0)|(tsl10?SSLCOPT_TLSv10:0));
	
	return 0;
}

/***
Add all the default certificates to the context.
@function addDefaultCert
*/
static int httpc_addDefaultCert(lua_State *L) {
	httpcContext *context = lua_touserdata(L, 1);
	
	httpcAddDefaultCert(context, SSLC_DefaultRootCert_CyberTrust);
	httpcAddDefaultCert(context, SSLC_DefaultRootCert_AddTrust_External_CA);
	httpcAddDefaultCert(context, SSLC_DefaultRootCert_COMODO);
	httpcAddDefaultCert(context, SSLC_DefaultRootCert_USERTrust);
	httpcAddDefaultCert(context, SSLC_DefaultRootCert_DigiCert_EV);
	
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
	{"__gc",                  httpc_close                },
	{"addPostData",           httpc_addPostData          },
	{"getResponseHeader",     httpc_getResponseHeader    },
	{"addTrustedRootCA",      httpc_addTrustedRootCA     },
	{"setSSLOptions",         httpc_setSSLOptions        },
	{"addDefaultCert",        httpc_addDefaultCert       },
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
		httpcInit(0x1000);
		isHttpcInitialized = true;
	}
	
	luaL_requiref(L, "ctr.httpc", luaopen_httpc_lib, false);
}

void unload_httpc_lib(lua_State *L) {
	httpcExit();
}

