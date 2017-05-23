/***
The `uds` module. Used for 3DS-to-3DS wireless communication.
The default wlancommID is 0x637472c2.
@module ctr.uds
@usage local uds = require("ctr.uds")
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <3ds/result.h>
#include <3ds/types.h>
#include <3ds/services/uds.h>

#include <lualib.h>
#include <lauxlib.h>

#define DEFAULT_WLANCOMMID 0x637472c2

bool initStateUDS = false;

udsBindContext bind = {0};
udsNetworkStruct network = {0};
u8 data_channel = 1;

/***
Initialize the UDS module.
@function init
@tparam[opt=0x3000] number context size in bytes, must be a multiple of 0x1000
@tparam[opt=3DS username] string username UTF-8 username on the network
@treturn[1] boolean `true` on success
@treturn[2] boolean `false` on error
@treturn[2] number error code
*/
static int uds_init(lua_State *L) {
	if (!initStateUDS) {
		size_t memSize = luaL_optinteger(L, 1, 0x3000);
		const char* username = luaL_optstring(L, 2, NULL);
	
		Result ret = udsInit(memSize, username);
		if (R_FAILED(ret)) {
			lua_pushboolean(L, false);
			lua_pushinteger(L, ret);
			return 2;
		}
		initStateUDS = true;
	}
	
	lua_pushboolean(L, true);
	return 1;
}

/***
Disable the UDS module.
@function shutdown
*/
static int uds_shutdown(lua_State *L) {
	udsExit();
	initStateUDS = false;
	
	return 0;
}

/***
Scan for network beacons.
@function scan
@tparam[opt=0x637472c2] number commID application local-WLAN unique ID
@tparam[opt=0] number id8 additional ID to use different network types
@tparam[opt=all] string hostMAC if set, only scan networks from this MAC address (format: `XX:XX:XX:XX:XX:XX`, with hexadecimal values)
@treturn[1] table a table containing beacons objects
@treturn[2] nil
@treturn[2] number/string error code
*/
static int uds_scan(lua_State *L) {
	static const size_t tmpbuffSize = 0x4000;
	u32* tmpbuff = malloc(tmpbuffSize);
	if (tmpbuff == NULL) {
		lua_pushnil(L);
		lua_pushstring(L, "Failed to allocated beacon data buffer");
		return 2;
	}
	
	udsNetworkScanInfo* networks = NULL;
	size_t totalNetworks = 0;
	
	u32 wlanCommID = luaL_optinteger(L, 1, DEFAULT_WLANCOMMID);
	u8 id8 = luaL_optinteger(L, 2, 0);
	
	// MAC address conversion
	const char* hostMACString = luaL_optstring(L, 3, NULL);
	u8* hostMAC;
	if (hostMACString != NULL) {
		hostMAC = malloc(6*sizeof(u8));
		unsigned int tmpMAC[6];
		if (sscanf(hostMACString, "%x:%x:%x:%x:%x:%x", &tmpMAC[0], &tmpMAC[1], &tmpMAC[2], &tmpMAC[3], &tmpMAC[4], &tmpMAC[5]) != 6) {
			free(tmpbuff);
			free(hostMAC);
			lua_pushnil(L);
			lua_pushstring(L, "Bad MAC formating");
			return 2;
		}
		for (int i=0;i<6;i++) {
			hostMAC[i] = tmpMAC[i];
		}
	} else {
		hostMAC = NULL;
	}
	
	udsConnectionStatus status;
	udsGetConnectionStatus(&status);
	Result ret = udsScanBeacons(tmpbuff, tmpbuffSize, &networks, &totalNetworks, wlanCommID, id8, hostMAC, (status.status!=0x03));
	free(tmpbuff);
	free(hostMAC);
	if (R_FAILED(ret)) {
		lua_pushnil(L);
		lua_pushinteger(L, ret);
		return 2;
	}
	
	// Convert the networks to a table of userdatas
	lua_createtable(L, 0, totalNetworks);
	for (int i=1;i<=totalNetworks;i++) {
		udsNetworkScanInfo* beacon = lua_newuserdata(L, sizeof(udsNetworkScanInfo));
		luaL_getmetatable(L, "LUDSBeaconScan");
		lua_setmetatable(L, -2);
		memcpy(beacon, &networks[i-1], sizeof(udsNetworkScanInfo));
		lua_seti(L, -3, i);
	}
	free(networks);
	
	return 1;
}

/***
Check for data in the receive buffer.
@function available
@treturn boolean `true` if there is data to receive, `false` if not
*/
static int uds_available(lua_State *L) {
	lua_pushboolean(L, udsWaitDataAvailable(&bind, false, false));
	return 1;
}

/***
Return a packet from the receive buffer.
@function receive
@tparam[opt=maximum] number size maximum size of the data to receive
@treturn string data, can be an empty string
@treturn number source node, 0 if nothing has been received
*/
static int uds_receive(lua_State *L) {
	size_t maxSize = luaL_optinteger(L, 1, UDS_DATAFRAME_MAXSIZE);
	char* buff = malloc(maxSize);
	if (buff == NULL) luaL_error(L, "Memory allocation error");
	size_t received = 0;
	u16 sourceID = 0;
	
	Result ret = udsPullPacket(&bind, buff, maxSize, &received, &sourceID);
	if (R_FAILED(ret)) {
		free(buff);
		lua_pushnil(L);
		lua_pushinteger(L, ret);
		return 2;
	}
	
	lua_pushlstring(L, buff, received);
	free(buff);
	lua_pushinteger(L, sourceID);
	return 2;
}

/***
Send a packet to a node.
@function send
@tparam string data data to send
@tparam[opt=BROADCAST] number nodeID nodeID to send the packet to
*/
static int uds_send(lua_State *L) {
	size_t size = 0;
	const char *buff = luaL_checklstring(L, 1, &size);
	u16 nodeID = luaL_optinteger(L, 2, UDS_BROADCAST_NETWORKNODEID);
	
	udsSendTo(nodeID, data_channel, 0, buff, size);
	
	return 0;
}

/***
Return information about nodes in the networks
@function getNodesInfo
@treturn tablea table containing nodes informations, as tables (not userdatas).
A node table is like: `{ username = "azerty", nodeID = 12 }`
*/
static int uds_getNodesInfo(lua_State *L) {
	lua_newtable(L);
	for (int i=0;i<UDS_MAXNODES;i++) {
		udsNodeInfo node;
		udsGetNodeInformation(i, &node);
		if (!udsCheckNodeInfoInitialized(&node)) continue;
		lua_createtable(L, 0, 2);
		char tmpstr[256];
		memset(tmpstr, 0, sizeof(tmpstr));
		udsGetNodeInfoUsername(&node, tmpstr);
		lua_pushstring(L, tmpstr);
		lua_setfield(L, -2, "username");
		lua_pushinteger(L, node.NetworkNodeID);
		lua_setfield(L, -2, "nodeID");

		lua_seti(L, -3, i+1);
	}
	return 1;
}

/***
Return the application data of the current network.
@function getAppData
@tparam[opt=0x4000] number maxSize maximum application data size to return
@treturn string application data
*/
static int uds_getAppData(lua_State *L) {
	size_t maxSize = luaL_optinteger(L, 1, 0x4000);
	char* buff = malloc(maxSize);
	if (buff == NULL) luaL_error(L, "Memory allocation error");
	size_t size = 0;
	
	udsGetApplicationData(buff, maxSize, &size);
	
	lua_pushlstring(L, buff, size);
	free(buff);
	return 1;
}

/***
Client part
@section client
*/

/***
Connect to a network.
@function connect
@tparam beaconScan beacon beacon to connect to
@tparam[opt=""] string passphrase passphrase for the network
@tparam[opt=CLIENT] number conType type of connection, can be `CONTYPE_CLIENT` or `CONTYPE_SPECTATOR`
@tparam[opt=BROADCAST] number nodeID nodeID to receive data from
@tparam[opt=default] number recvBuffSize size of the buffer that receives data
@tparam[opt=1] number dataChannel data channel of the network; this should be 1
@treturn[1] boolean `true` on success
@treturn[2] boolean `false` on error
@treturn[2] number error code
*/
static int uds_connect(lua_State *L) {
	udsNetworkScanInfo* beacon = luaL_checkudata(L, 1, "LUDSBeaconScan");
	const char* passphrase = luaL_optstring(L, 2, "");
	size_t passphraseSize = strlen(passphrase)+1;
	udsConnectionType connType = luaL_optinteger(L, 3, UDSCONTYPE_Client);
	u16 recvNetworkNodeID = luaL_optinteger(L, 4, UDS_BROADCAST_NETWORKNODEID);
	u32 recvBufferSize = luaL_optinteger(L, 5, UDS_DEFAULT_RECVBUFSIZE);
	u8 dataChannel = luaL_optinteger(L, 6, 1);
	
	Result ret = udsConnectNetwork(&beacon->network, passphrase, passphraseSize, &bind, recvNetworkNodeID, connType, dataChannel, recvBufferSize);
	if (R_FAILED(ret)) {
		lua_pushnil(L);
		lua_pushinteger(L, ret);
		return 2;
	}
	
	lua_pushboolean(L, true);
	return 1;
}

/***
Disconnect from the network.
@function disconnect
*/
static int uds_disconnect(lua_State *L) {
	udsDisconnectNetwork();
	udsUnbind(&bind);
	
	return 0;
}

/***
Server part
@section server
*/

/***
Create a network.
@function createNetwork
@tparam[opt=""] string passphrase passphrase of the network
@tparam[opt=16] number maxNodes maximum number of nodes that can be connected to the network, including the host (max 16)
@tparam[opt=0x637472c2] number commID application local-WLAN unique ID
@tparam[opt=default] number recvBuffSize size of the buffer that receives data
@tparam[opt=1] number dataChannel data channel of the network; this should be 1
@treturn[1] boolean `true` on success
@treturn[2] boolean `false` on error
@treturn[2] number error code
*/
static int uds_createNetwork(lua_State *L) {
	size_t passSize = 0;
	const char *pass = luaL_optlstring(L, 1, "", &passSize);
	u8 maxNodes = luaL_optinteger(L, 2, UDS_MAXNODES);
	u32 commID = luaL_optinteger(L, 3, DEFAULT_WLANCOMMID);
	u32 recvBuffSize = luaL_optinteger(L, 4, UDS_DEFAULT_RECVBUFSIZE);
	u8 dataChannel = luaL_optinteger(L, 5, 1);
	
	udsGenerateDefaultNetworkStruct(&network, commID, dataChannel, maxNodes);
	Result ret = udsCreateNetwork(&network, pass, passSize+1, &bind, dataChannel, recvBuffSize);
	if (R_FAILED(ret)) {
		lua_pushboolean(L, false);
		lua_pushinteger(L, ret);
		return 2;
	}
	
	lua_pushboolean(L, 1);
	return 1;
}

/***
Set the application data of the created network.
@function setAppData
@tparam string appData application data
@treturn[1] boolean `true` on success
@treturn[2] boolean `false` on error
@treturn[2] number error code
*/
static int uds_setAppData(lua_State *L) {
	size_t size = 0;
	const char* data = luaL_checklstring(L, 1, &size);
	
	Result ret = udsSetApplicationData(data, size);
	if (R_FAILED(ret)) {
		lua_pushboolean(L, false);
		lua_pushinteger(L, ret);
		return 2;
	}
	
	lua_pushboolean(L, true);
	return 1;
}

/***
Destroy the network.
@function destroyNetwork
*/
static int uds_destroyNetwork(lua_State *L) {
	udsDestroyNetwork();
	udsUnbind(&bind);
	
	return 0;
}

/***
Eject all the spectators connected to the network.
@function ejectSpectators
@tparam[opt=false] boolean reallow set to `true` to still allow the spectators to connect
*/
static int uds_ejectSpectators(lua_State *L) {
	bool reallow = false;
	if (lua_isboolean(L, 1))
		reallow = lua_toboolean(L, 1);
	
	udsEjectSpectator();
	
	if (reallow)
		udsAllowSpectators();
	
	return 0;
}

/***
Eject a client connected to the network.
@function ejectClient
@tparam[opt=BROADCAST] number nodeID node ID of the client to eject, or `BROADCAST` for all the clients
*/
static int uds_ejectClient(lua_State *L) {
	u16 nodeID = luaL_optinteger(L, 1, UDS_BROADCAST_NETWORKNODEID);
	
	udsEjectClient(nodeID);
	
	return 0;
}

/***
beaconScan
@section beaconScan
*/
static const struct luaL_Reg beaconScan_methods[];

static int beaconScan___index(lua_State *L) {
	udsNetworkScanInfo* beacon = luaL_checkudata(L, 1, "LUDSBeaconScan");
	
	if (lua_isstring(L, 2)) {
		const char* index = luaL_checkstring(L, 2);
		/***
		@tfield integer channel Wifi channel of the beacon
		*/
		if (!strcmp(index, "channel")) {
			lua_pushinteger(L, beacon->datareply_entry.channel);
			return 1;
		/***
		@tfield string mac MAC address of the beacon (`mac` for lowercase, `MAC` for uppercase)
		@usage
beacon.mac -> ab:cd:ef:ab:cd:ef
beacon.MAC -> AB:CD:EF:AB:CD:EF
		*/
		} else if (!strcmp(index, "mac") && !strcmp(index, "MAC")) {
			char* macString = malloc(18);
			if (macString == NULL) luaL_error(L, "Out of memory");
			if (index[1] == 'm') { // lowercase
				sprintf(macString, "%0x2:%0x2:%0x2:%0x2:%0x2:%0x2", beacon->datareply_entry.mac_address[0], beacon->datareply_entry.mac_address[1], beacon->datareply_entry.mac_address[2], beacon->datareply_entry.mac_address[3], beacon->datareply_entry.mac_address[4], beacon->datareply_entry.mac_address[5]);
			} else {
				sprintf(macString, "%0X2:%0X2:%0X2:%0X2:%0X2:%0X2", beacon->datareply_entry.mac_address[0], beacon->datareply_entry.mac_address[1], beacon->datareply_entry.mac_address[2], beacon->datareply_entry.mac_address[3], beacon->datareply_entry.mac_address[4], beacon->datareply_entry.mac_address[5]);
			}
			lua_pushstring(L, macString);
			free(macString);
			return 1;
		/***
		@tfield table nodes a table containing nodes informations, as tables (not userdatas).
		A node table is like: `{ username = "azerty", nodeID = 12 }`
		*/
		} else if (!strcmp(index, "nodes")) {
			lua_newtable(L);
			for (int i=0;i<UDS_MAXNODES;i++) {
				if (!udsCheckNodeInfoInitialized(&beacon->nodes[i])) continue;
				lua_createtable(L, 0, 2);
				char tmpstr[256]; // 256 is maybe too much ... But we have a lot of RAM.
				memset(tmpstr, 0, sizeof(tmpstr));
				udsGetNodeInfoUsername(&beacon->nodes[i], tmpstr);
				lua_pushstring(L, tmpstr);
				lua_setfield(L, -2, "username");
				lua_pushinteger(L, (&beacon->nodes[i])->NetworkNodeID);
				lua_setfield(L, -2, "nodeID");
				
				lua_seti(L, -3, i+1);
			}
			return 1;
		/***
		@tfield number id8 id8 of the beacon's network
		*/
		} else if (!strcmp(index, "id8")) {
			lua_pushinteger(L, beacon->network.id8);
			return 1;
		/***
		@tfield number networkID random ID of the network
		*/
		} else if (!strcmp(index, "networkID")) {
			lua_pushinteger(L, beacon->network.networkID);
			return 1;
		/***
		@tfield boolean allowSpectators `true` if new spectators are allowed on the network
		*/
		} else if (!strcmp(index, "allowSpectators")) {
			lua_pushboolean(L, !(beacon->network.attributes&UDSNETATTR_DisableConnectSpectators));
			return 1;
		/***
		@tfield boolean allowClients `true` if new clients are allowed on the network
		*/
		} else if (!strcmp(index, "allowClients")) {
			lua_pushboolean(L, !(beacon->network.attributes&UDSNETATTR_DisableConnectClients));
			return 1;
		// methods
		} else {
			for (int i=0;beaconScan_methods[i].name;i++) {
				if (!strcmp(beaconScan_methods[i].name, index)) {
					lua_pushcfunction(L, beaconScan_methods[i].func);
					return 1;
				}
			}
		}
	}
	
	lua_pushnil(L);
	return 1;
}

/***
Return the application data of the beacon
@function :getAppData
@tparam[opt=0x4000] number maxSize maximum application data size to return
@treturn[1] string application data
@treturn[2] nil
@treturn[2] error code
*/
static int beaconScan_getAppData(lua_State *L) {
	udsNetworkScanInfo* beacon = luaL_checkudata(L, 1, "LUDSBeaconScan");
	size_t maxSize = luaL_optinteger(L, 2, 0x4000);
	
	u8* data = malloc(maxSize);
	if (data == NULL) luaL_error(L, "Memory allocation error");
	
	size_t size = 0;
	udsGetNetworkStructApplicationData(&beacon->network, data, maxSize, &size);
	
	lua_pushlstring(L, (const char*)data, size);
	free(data);
	
	return 1;
}

// beaconScan object
static const struct luaL_Reg beaconScan_methods[] = {
	{"__index",    beaconScan___index   },
	{"getAppData", beaconScan_getAppData},
	{NULL, NULL}
};

// module functions
static const struct luaL_Reg uds_lib[] = {
	{"init",            uds_init           },
	{"shutdown",        uds_shutdown       },
	{"scan",            uds_scan           },
	{"getNodesInfo ",   uds_getNodesInfo   },
	{"getAppData",      uds_getAppData     },
	{"connect",         uds_connect        },
	{"available",       uds_available      },
	{"send",            uds_send           },
	{"receive",         uds_receive        },
	{"disconnect",      uds_disconnect     },
	{"createNetwork",   uds_createNetwork  },
	{"setAppData",      uds_setAppData     },
	{"destroyNetwork",  uds_destroyNetwork },
	{"ejectSpectators", uds_ejectSpectators},
	{"ejectClient",     uds_ejectClient    },
	{NULL, NULL}
};

/***
Constants
@section constants
*/

struct { char *name; int value; } uds_constants[] = {
	/***
	@field BROADCAST broadcast node ID
	*/
	{"BROADCAST", UDS_BROADCAST_NETWORKNODEID},
	/***
	@field HOST host node ID
	*/
	{"HOST",      UDS_HOST_NETWORKNODEID     },
	/***
	@field CLIENT used to specify a connection as a client
	*/
	{"CLIENT",    UDSCONTYPE_Client          },
	/***
	@field SPECTATOR used to specify a connection as a spectator
	*/
	{"SPECTATOR", UDSCONTYPE_Spectator       },
	{NULL, 0}
};

int luaopen_uds_lib(lua_State *L) {
	luaL_newmetatable(L, "LUDSBeaconScan");
	lua_pushvalue(L, -1);
	lua_setfield(L, -2, "__index");
	luaL_setfuncs(L, beaconScan_methods, 0);
	
	luaL_newlib(L, uds_lib);
	
	for (int i = 0; uds_constants[i].name; i++) {
		lua_pushinteger(L, uds_constants[i].value);
		lua_setfield(L, -2, uds_constants[i].name);
	}
	
	return 1;
}

void load_uds_lib(lua_State *L) {
	luaL_requiref(L, "ctr.uds", luaopen_uds_lib, 0);
}

void unload_uds_lib(lua_State *L) {
	if (initStateUDS) {
		udsConnectionStatus status;
		udsGetConnectionStatus(&status);
		switch (status.status) {
			case 0x6: // connected as host
				udsDestroyNetwork();
				udsUnbind(&bind);
				break;
			
			case 0x9: // connected as client
			case 0xA: // connected as spectator
				udsDisconnectNetwork();
				udsUnbind(&bind);
				break;
			
			default:
				break;
		}
		udsExit();
		initStateUDS = false;
	}
}
