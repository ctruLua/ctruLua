/***
The `socket` module. Almost like luasocket, but for TCP only.
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

/***
Initialize the socket module
@function init
@tparam[opt=0x100000] number buffer size (in bytes), must be a multiple of 0x1000
*/
static int socket_init(lua_State *L) {
	u32 size = luaL_optinteger(L, 1, 0x100000);
	Result ret = SOC_Initialize((u32*)memalign(0x1000, size), size);
	
	if (ret) {
		lua_pushboolean(L, false);
		lua_pushinteger(L, ret);
		return 2;
	}
	
	lua_pushboolean(L, true);
	return 1;
}

/***
Disable the socket module. Must be called before exiting ctrµLua.
@function shutdown
*/
static int socket_shutdown(lua_State *L) {
	SOC_Shutdown();
	
	return 0;
}

/***
Return a TCP socket.
@function tcp
@treturn TCPMaster TCP socket
*/
static int socket_tcp(lua_State *L) {
	socket_userdata *userdata = lua_newuserdata(L, sizeof(*userdata));
	luaL_getmetatable(L, "LSocket");
	lua_setmetatable(L, -2);
	
	userdata->socket = socket(AF_INET, SOCK_STREAM, 0);
	if (userdata->socket < 0) {
		lua_pushnil(L);
		lua_pushstring(L, "Failed to create a TCP socket");
		return 2;
	}
	
	userdata->addr.sin_family = AF_INET;
	
	return 1;
}

/***
TCP Sockets
@section TCP
*/

/***
Accept a connection on a server.
@function :accept
@treturn TCPClient tcp client object, or nil.
*/
static int socket_accept(lua_State *L) {
	socket_userdata *userdata = luaL_checkudata(L, 1, "LSocket");
	
	socket_userdata *client = lua_newuserdata(L, sizeof(*client));
	luaL_getmetatable(L, "LSocket");
	lua_setmetatable(L, -2);
	
	socklen_t addrSize = sizeof(client->addr);
	client->socket = accept(userdata->socket, (struct sockaddr*)&client->addr, &addrSize);
	if (client->socket < 0) {
		lua_pushnil(L);
		return 1;
	}
	
	return 1;
}

/***
Bind a socket. The TCP object become a TCPServer object.
@function :bind
@tparam number port the port to bind the socket on.
*/
static int socket_bind(lua_State *L) {
	socket_userdata *userdata = luaL_checkudata(L, 1, "LSocket");
	int port = luaL_checkinteger(L, 2);
	
	userdata->addr.sin_addr.s_addr = htonl(INADDR_ANY);
	userdata->addr.sin_port = htons(port);
	
	bind(userdata->socket, (struct sockaddr*)&userdata->addr, sizeof(userdata->addr));
	
	return 0;
}

/***
Close an existing socket.
@function :close
*/
static int socket_close(lua_State *L) {
	socket_userdata *userdata = luaL_checkudata(L, 1, "LSocket");
	
	closesocket(userdata->socket);
	
	return 0;
}

/***
Connect a socket to a server. The TCP object becomes a TCPClient object.
@function :connect
@tparam string host address of the host
@tparam number port port of the server
*/
static int socket_connect(lua_State *L) {
	socket_userdata *userdata = luaL_checkudata(L, 1, "LSocket");
	char *addr = (char*)luaL_checkstring(L, 2);
	int port = luaL_checkinteger(L, 3);
	
	userdata->host = gethostbyname(addr);
	if (userdata->host == NULL) {
		lua_pushnil(L);
		lua_pushstring(L, "No such host");
		return 2;
	}
	
	userdata->addr.sin_port = htons(port);
	bcopy((char*)userdata->host->h_addr, (char*)&userdata->addr.sin_addr.s_addr, userdata->host->h_length);
	
	if (connect(userdata->socket, (const struct sockaddr*)&userdata->addr, sizeof(userdata->addr)) < 0) {
		lua_pushnil(L);
		lua_pushstring(L, "Connection failed");
		return 2;
	}
	
	lua_pushinteger(L, 1);
	return 1;
}

/***
Open the socket for connections.
@function :listen
@tparam[opt=16] number max maximum number of simultaneous connections
*/
static int socket_listen(lua_State *L) {
	socket_userdata *userdata = luaL_checkudata(L, 1, "LSocket");
	int max = luaL_optinteger(L, 2, 16);
	
	listen(userdata->socket, max);
	
	return 0;
}

/***
Receive some data from the socket.
If no data is avaible, it returns an empty string (non-blocking).
@function :receive
@tparam[opt="l"] number/string size amount of bytes to receive; or
							   "a" to receive everything,
							   "l" to receive the next line, skipping the end of line,
							   "L" to receive the next line, keeping the end of line.
@treturn string data
*/
static int socket_receive(lua_State *L) {
	socket_userdata *userdata = luaL_checkudata(L, 1, "LSocket");
	int count = 0;
	int flags = 0;

	if (lua_isnumber(L, 2)) {
		count = luaL_checkinteger(L, 2);
	} else {
		const char *p = luaL_optstring(L, 2, "l");
		if (*p == 'a') {
			count = SIZE_MAX/2;

		} else if (*p == 'l') {
			luaL_Buffer b;
			luaL_buffinit(L, &b);

			char buff;
			while (recv(userdata->socket, &buff, 1, flags) > 0 && buff != '\n') luaL_addchar(&b, buff);

			luaL_pushresult(&b);
			return 1;

		} else if (*p == 'L') {
			luaL_Buffer b;
			luaL_buffinit(L, &b);

			char buff;
			while (buff != '\n' && recv(userdata->socket, &buff, 1, flags) > 0) luaL_addchar(&b, buff);

			luaL_pushresult(&b);
			return 1;

		} else {
			return luaL_argerror(L, 2, "invalid format");
		}
	}
	
	char *buff = malloc(count+1);
	int len = recv(userdata->socket, buff, count, flags);
	*(buff+len) = 0x0; // text end
	
	lua_pushstring(L, buff);
	return 1;
}

/***
Send some data over the TCP socket.
@function :send
@tparam string data data to send
@treturn number amount of data sent
*/
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
	{"accept",   socket_accept  },
	{"bind",     socket_bind    },
	{"close",    socket_close   },
	{"connect",  socket_connect },
	{"listen",   socket_listen  },
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
