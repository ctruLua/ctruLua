/***
The `socket` module. Almost like luasocket, but for TCP/UDP only.
See http://w3.impa.br/~diego/software/luasocket/reference.html for a
documentation.
@module ctr.socket
@usage local socket = require("ctr.socket")
*/

#include <3ds.h>
#include <3ds/types.h>
#include <3ds/services/soc.h>

#include <lapi.h>
#include <lauxlib.h>

#include <malloc.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

typedef struct {
	int socket;
	struct sockaddr_in addr;
	struct hostent *host; // only user for client sockets
} socket_userdata;

static int socket_init(lua_State *L) {
	u32 size = luaL_optinteger(L, 1, 0x10000);
	Result ret = SOC_Initialize((u32*)memalign(0x1000, size), size);
	
	if (ret) {
		lua_pushboolean(L, false);
		lua_pushinteger(L, ret);
		return 2;
	}
	
	lua_pushboolean(L, true);
	return 1;
}

static int socket_shutdown(lua_State *L) {
	SOC_Shutdown();
	
	return 0;
}

static int socket_tcp(lua_State *L) {
	socket_userdata *data = lua_newuserdata(L, sizeof(*data));
	luaL_getmetatable(L, "LSocket");
	lua_setmetatable(L, -2);
	
	data->socket = socket(AF_INET, SOCK_STREAM, 0);
	if (data->socket < 0) {
		lua_pushnil(L);
		lua_pushstring(L, "Failed to create a TCP socket");
		return 2;
	}
	
	data->addr.sin_family = AF_INET;
	
	return 1;
}

/* methods */

static int socket_close(lua_State *L) {
	socket_userdata *data = luaL_checkudata(L, 1, "LSocket");
	
	close(data->socket);
	
	return 0;
}

static int socket_connect(lua_State *L) {
	socket_userdata *data = luaL_checkudata(L, 1, "LSocket");
	char *addr = (char*)luaL_checkstring(L, 2);
	int port = luaL_checkinteger(L, 3);
	
	data->host = gethostbyname(addr);
	if (data->host == NULL) {
		lua_pushnil(L);
		lua_pushstring(L, "No such host");
		return 2;
	}
	
	data->addr.sin_port = htons(port);
	bcopy((char*)data->host->h_addr, (char*)&data->addr.sin_addr.s_addr, data->host->h_length);
	
	if (connect(data->socket, (const struct sockaddr*)&data->addr, sizeof(data->addr)) < 0) {
		lua_pushnil(L);
		lua_pushstring(L, "Connection failed");
		return 2;
	}
	
	lua_pushinteger(L, 1);
	return 1;
}

static int socket_receive(lua_State *L) {
	socket_userdata *userdata = luaL_checkudata(L, 1, "LSocket");
	int count = 0;
	int flags = 0;
	if (lua_isnumber(L, 2)) {
		count = luaL_checkinteger(L, 2);
	} else if (lua_isstring(L, 2) && luaL_checkstring(L, 2) == (char*)&"*a") {
		count = SIZE_MAX/2;
	} else {
		lua_pushnil(L);
		lua_pushstring(L, "");
		return 2;
	}
	
	char *buff = malloc(count);
	recv(userdata->socket, buff, count, flags);
	
	lua_pushstring(L, buff);
	return 1;
}

static int socket_send(lua_State *L) {
	socket_userdata *userdata = luaL_checkudata(L, 1, "LSocket");
	size_t size = 0;
	char *data = (char*)luaL_checklstring(L, 2, &size);
	
	size_t sent = send(userdata->socket, data, size, 0);
	
	lua_pushinteger(L, sent);
	return 1;
}

// module functions
static const struct luaL_Reg socket_functions[] = {
	{"init",     socket_init    },
	{"shutdown", socket_shutdown},
	{"tcp",      socket_tcp     },
	{NULL, NULL}
};

// object
static const struct luaL_Reg socket_methods[] = {
	{"close",    socket_close   },
	{"connect",  socket_connect },
	{"receive",  socket_receive },
	{"send",     socket_send    },
	{NULL, NULL}
};

int luaopen_socket_lib(lua_State *L) {
	luaL_newmetatable(L, "LSocket");
	lua_pushvalue(L, -1);
	lua_setfield(L, -2, "__index");
	luaL_setfuncs(L, socket_methods, 0);
	
	luaL_newlib(L, socket_functions);
	
	return 1;
}

void load_socket_lib(lua_State *L) {
	luaL_requiref(L, "ctr.socket", luaopen_socket_lib, false);
}
