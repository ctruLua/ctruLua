/***
The `socket` module. Almost like luasocket, but for the TCP part only.
The UDP part is only without connection.
All sockets are not blocking by default.
@module ctr.socket
@usage local socket = require("ctr.socket")
*/

#include <3ds.h>
#include <3ds/types.h>
#include <3ds/services/soc.h>
#include <3ds/services/sslc.h>

#include <lapi.h>
#include <lauxlib.h>

#include <malloc.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

typedef struct {
	int socket;
	struct sockaddr_in addr;
	struct hostent *host; // only used for client sockets
	sslcContext sslContext;
	bool isSSL;
} socket_userdata;

bool initStateSocket = false;

u32 rootCertChain = 0;

/***
Initialize the socket module
@function init
@tparam[opt=0x100000] number buffer size (in bytes), must be a multiple of 0x1000
@treturn[1] boolean `true` if everything went fine
@treturn[2] boolean `false` in case of error
@treturn[2] number/string error code/message
*/
static int socket_init(lua_State *L) {
	if (!initStateSocket) {
		u32 size = luaL_optinteger(L, 1, 0x100000);
		if (size%0x1000 != 0) {
			lua_pushboolean(L, false);
			lua_pushstring(L, "Not a multiple of 0x1000");
			return 2;
		}
		
		u32* mem = (u32*)memalign(0x1000, size);
		if (mem == NULL) {
			lua_pushboolean(L, false);
			lua_pushstring(L, "Failed to allocate memory");
			return 2;
		}
		
		Result ret = socInit(mem, size);
	
		if (R_FAILED(ret)) {
			lua_pushboolean(L, false);
			lua_pushinteger(L, ret);
			return 2;
		}
		
		ret = sslcInit(0);
		if (R_FAILED(ret)) {
			lua_pushboolean(L, false);
			lua_pushinteger(L, ret);
			return 2;
		}
		
		sslcCreateRootCertChain(&rootCertChain);
		sslcRootCertChainAddDefaultCert(rootCertChain, SSLC_DefaultRootCert_CyberTrust, NULL);
		sslcRootCertChainAddDefaultCert(rootCertChain, SSLC_DefaultRootCert_AddTrust_External_CA, NULL);
		sslcRootCertChainAddDefaultCert(rootCertChain, SSLC_DefaultRootCert_COMODO, NULL);
		sslcRootCertChainAddDefaultCert(rootCertChain, SSLC_DefaultRootCert_USERTrust, NULL);
		sslcRootCertChainAddDefaultCert(rootCertChain, SSLC_DefaultRootCert_DigiCert_EV, NULL);
		
		initStateSocket = true;
	}
	
	lua_pushboolean(L, true);
	return 1;
}

/***
Disable the socket module. Must be called before exiting ctrµLua.
@function shutdown
*/
static int socket_shutdown(lua_State *L) {
	if (initStateSocket) {
		sslcDestroyRootCertChain(rootCertChain);
		sslcExit();
		socExit();
		initStateSocket = false;
	}
	
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
	
	userdata->isSSL = false;
	fcntl(userdata->socket, F_SETFL, fcntl(userdata->socket, F_GETFL, 0)|O_NONBLOCK);
	
	return 1;
}

/***
Return an UDP socket.
@function udp
@treturn UDPMaster UDP socket
*/
static int socket_udp(lua_State *L) {
	socket_userdata *userdata = lua_newuserdata(L, sizeof(*userdata));
	luaL_getmetatable(L, "LSocket");
	lua_setmetatable(L, -2);
	
	userdata->socket = socket(AF_INET, SOCK_DGRAM, 0);
	if (userdata->socket < 0) {
		lua_pushnil(L);
		lua_pushstring(L, strerror(errno));
		return 2;
	}
	
	userdata->addr.sin_family = AF_INET;
	fcntl(userdata->socket, F_SETFL, fcntl(userdata->socket, F_GETFL, 0)|O_NONBLOCK);
	
	return 1;
}

/***
Add a trusted root CA to the certChain.
@function addTrustedRootCA
@tparam string cert DER cert
@treturn[1] boolean `true` if everything went fine
@treturn[2] nil in case of error
@treturn[2] number error code
*/
static int socket_addTrustedRootCA(lua_State *L) {
	size_t size = 0;
	const char* cert = luaL_checklstring(L, 1, &size);
	
	Result ret = sslcAddTrustedRootCA(rootCertChain, (u8*)cert, size, NULL);
	if (R_FAILED(ret)) {
		lua_pushnil(L);
		lua_pushinteger(L, ret);
		return 2;
	}
	
	lua_pushboolean(L, true);
	return 1;
}

/***
All sockets
@section sockets
*/

/***
Bind a socket. The socket object become a socketServer object.
@function :bind
@tparam number port the port to bind the socket on.
*/
static int socket_bind(lua_State *L) {
	socket_userdata *userdata = luaL_checkudata(L, 1, "LSocket");
	int port = luaL_checkinteger(L, 2);
	
	userdata->addr.sin_addr.s_addr = gethostid();
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
	
	if (userdata->isSSL) {
		sslcDestroyContext(&userdata->sslContext);
	}
	
	closesocket(userdata->socket);
	
	return 0;
}

/***
Get some informations from a socket.
@function :getpeername
@treturn string IP
@treturn number port
*/
static int socket_getpeername(lua_State *L) {
	socket_userdata *userdata = luaL_checkudata(L, 1, "LSocket");
	
	struct sockaddr_in addr;
	socklen_t addrSize = sizeof(addr);
	
	getpeername(userdata->socket, (struct sockaddr*)&addr, &addrSize);
	
	lua_pushstring(L, inet_ntoa(addr.sin_addr));
	lua_pushinteger(L, ntohs(addr.sin_port));
	
	return 2;
}

/***
Get some local informations from a socket.
@function :getsockname
@treturn string IP
@treturn number port
*/
static int socket_getsockname(lua_State *L) {
	socket_userdata *userdata = luaL_checkudata(L, 1, "LSocket");
	
	struct sockaddr_in addr;
	socklen_t addrSize = sizeof(addr);
	
	getsockname(userdata->socket, (struct sockaddr*)&addr, &addrSize);
	
	lua_pushstring(L, inet_ntoa(addr.sin_addr));
	lua_pushinteger(L, ntohs(addr.sin_port));
	
	return 2;
}

/***
Set if the socket should be blocking.
@function :setBlocking
@tparam[opt=true] boolean block if `false`, the socket won't block
*/
static int socket_setBlocking(lua_State *L) {
	socket_userdata *userdata = luaL_checkudata(L, 1, "LSocket");
	bool block = true;
	if (lua_isboolean(L, 2))
		block = lua_toboolean(L, 2);
	
	int flags = fcntl(userdata->socket, F_GETFL, 0);
	flags = block?(flags&~O_NONBLOCK):(flags|O_NONBLOCK);
	fcntl(userdata->socket, F_SETFL, flags);
	
	return 0;
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
	client->isSSL = false;
	
	socklen_t addrSize = sizeof(client->addr);
	client->socket = accept(userdata->socket, (struct sockaddr*)&client->addr, &addrSize);
	if (client->socket < 0) {
		lua_pushnil(L);
		return 1;
	}
	
	return 1;
}

/***
Connect a socket to a server. The TCP object becomes a TCPClient object.
@function :connect
@tparam string host address of the host
@tparam number port port of the server
@tparam[opt=false] boolean ssl use SSL if `true`
@treturn[1] boolean true if success
@treturn[2] boolean false if failed
@treturn[2] string error string
*/
static int socket_connect(lua_State *L) {
	socket_userdata *userdata = luaL_checkudata(L, 1, "LSocket");
	char *addr = (char*)luaL_checkstring(L, 2);
	int port = luaL_checkinteger(L, 3);
	bool ssl = lua_toboolean(L, 4);
	
	userdata->host = gethostbyname(addr);
	if (userdata->host == NULL) {
		lua_pushnil(L);
		lua_pushstring(L, strerror(errno));
		return 2;
	}
	
	userdata->addr.sin_port = htons(port);
	bcopy((char*)userdata->host->h_addr, (char*)&userdata->addr.sin_addr.s_addr, userdata->host->h_length);
	
	if (connect(userdata->socket, (const struct sockaddr*)&userdata->addr, sizeof(userdata->addr)) < 0) {
		lua_pushnil(L);
		lua_pushstring(L, strerror(errno));
		return 2;
	}
	
	if (ssl) { // SSL context setup
		sslcCreateContext(&userdata->sslContext, userdata->socket, SSLCOPT_Default, addr);
		sslcContextSetRootCertChain(&userdata->sslContext, rootCertChain);
		if (R_FAILED(sslcStartConnection(&userdata->sslContext, NULL, NULL))) {
			sslcDestroyContext(&userdata->sslContext);
			lua_pushnil(L);
			lua_pushstring(L, "SSL connection failed");
			return 2;
		}
		userdata->isSSL = true;
	}
	
	lua_pushboolean(L, 1);
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
			if (!userdata->isSSL) {
				while (recv(userdata->socket, &buff, 1, flags) > 0 && buff != '\n') luaL_addchar(&b, buff);
			} else {
				while (!R_FAILED(sslcRead(&userdata->sslContext, &buff, 1, false)) && buff != '\n') luaL_addchar(&b, buff);
			}

			luaL_pushresult(&b);
			return 1;

		} else if (*p == 'L') {
			luaL_Buffer b;
			luaL_buffinit(L, &b);

			char buff;
			if (!userdata->isSSL) {
				while (buff != '\n' && recv(userdata->socket, &buff, 1, flags) > 0) luaL_addchar(&b, buff);
			} else {
				while (buff != '\n' && !R_FAILED(sslcRead(&userdata->sslContext, &buff, 1, false))) luaL_addchar(&b, buff);
			}

			luaL_pushresult(&b);
			return 1;

		} else {
			return luaL_argerror(L, 2, "invalid format");
		}
	}
	
	char *buff = malloc(count+1);
	int len;
	if (!userdata->isSSL) {
		len = recv(userdata->socket, buff, count, flags);
	} else {
		len = sslcRead(&userdata->sslContext, buff, count, false);
		if (R_FAILED(len)) {
			lua_pushnil(L);
			lua_pushinteger(L, len);
			return 2;
		}
	}
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
	
	size_t sent;
	if (!userdata->isSSL) {
		sent = send(userdata->socket, data, size, 0);
	} else {
		sent = sslcWrite(&userdata->sslContext, data, size);
		if (R_FAILED(sent)) {
			lua_pushnil(L);
			lua_pushinteger(L, sent);
			return 2;
		}
	}
	
	if (sent < 0) {
		lua_pushnil(L);
		lua_pushstring(L, strerror(errno));
		return 2;
	}
	
	lua_pushinteger(L, sent);
	return 1;
}

/***
UDP sockets
@section UDP
*/

/***
Receive some data from a server.
@function :receivefrom
@tparam number count amount of data to receive
@tparam string host host name
@tparam number port port
@treturn string data
*/
static int socket_receivefrom(lua_State *L) {
	socket_userdata *userdata = luaL_checkudata(L, 1, "LSocket");
	int count = luaL_checkinteger(L, 2);
	size_t namesize = 0;
	char *hostname = (char*)luaL_optlstring(L, 3, NULL, &namesize);
	int port = luaL_optinteger(L, 4, 0);
	
	struct sockaddr_in from = {0};
	if (hostname != NULL) { // For a server
		struct hostent *hostinfo = gethostbyname(hostname);
		if (hostinfo == NULL) {
			lua_pushnil(L);
			return 1;
		}
		from.sin_addr = *(struct in_addr*)hostinfo->h_addr;
		from.sin_port = htons(port);
		from.sin_family = AF_INET;
	}
	
	char* buffer = malloc(count+1);
	int n = recvfrom(userdata->socket, buffer, count, 0, (struct sockaddr*)&from, NULL);
	*(buffer+n) = 0x0;
	
	lua_pushstring(L, buffer);
	if (hostname != NULL) {
		return 1;
	} else {
		lua_pushstring(L, inet_ntoa(from.sin_addr));
		lua_pushinteger(L, ntohs(from.sin_port));
		return 3;
	}
}

/***
Send some data to a server.
@function :sendto
@tparam string data data to send
@tparam string host host name
@tparam number port port
*/
static int socket_sendto(lua_State *L) {
	socket_userdata *userdata = luaL_checkudata(L, 1, "LSocket");
	size_t datasize = 0;
	char *data = (char*)luaL_checklstring(L, 2, &datasize);
	size_t namesize = 0;
	char *hostname = (char*)luaL_checklstring(L, 3, &namesize);
	int port = luaL_checkinteger(L, 4);
	
	struct hostent *hostinfo = gethostbyname(hostname);
	if (hostinfo == NULL) {
		lua_pushnil(L);
		return 1;
	}
	struct sockaddr_in to = {0};
	to.sin_addr = *(struct in_addr*)hostinfo->h_addr;
	to.sin_port = htons(port);
	to.sin_family = AF_INET;
	
	sendto(userdata->socket, data, datasize, 0, (struct sockaddr*)&to, sizeof(to));
	
	return 0;
}

// module functions
static const struct luaL_Reg socket_functions[] = {
	{"init",             socket_init            },
	{"shutdown",         socket_shutdown        },
	{"tcp",              socket_tcp             },
	{"udp",              socket_udp             },
	{"addTrustedRootCA", socket_addTrustedRootCA},
	{NULL, NULL}
};

// object
static const struct luaL_Reg socket_methods[] = {
	{"accept",      socket_accept     },
	{"bind",        socket_bind       },
	{"close",       socket_close      },
	{"setBlocking", socket_setBlocking},
	{"__gc",        socket_close      },
	{"connect",     socket_connect    },
	{"listen",      socket_listen     },
	{"receive",     socket_receive    },
	{"receivefrom", socket_receivefrom},
	{"send",        socket_send       },
	{"sendto",      socket_sendto     },
	{"getpeername", socket_getpeername},
	{"getsockname", socket_getsockname},
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
