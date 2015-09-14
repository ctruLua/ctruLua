/***
The `ir` module.
@module ctr.ir
@usage local ir = require("ctr.ir")
*/
#include <3ds/types.h>
#include <3ds/services/ir.h>
#include <3ds/linear.h>

#include <lualib.h>
#include <lauxlib.h>

u32 bufferSize = 0;
u32 *buffer;

/***
Bitrate codes list (this is not a part of the module, just a reference)
@table bitrates
@field 3 115200
@field 4 96000
@field 5 72000
@field 6 48000 (default)
@field 7 36000
@field 8 24000
@field 9 18000
@field 10 12000
@field 11 9600
@field 12 6000
@field 13 3000
@field 14 57600
@field 15 38400
@field 16 19200
@field 17 7200
@field 18 4800
*/

/***
Initialize the IR module.
@function init
@tparam[opt=6] number bitrate bitrate of the IR module (more informations below)
@tparam[opt=2048] number buffer size of the buffer, in bytes (max 2048)
*/
static int ir_init(lua_State *L) {
	u8 bitrate = luaL_optinteger(L, 1, 6);
	bufferSize = luaL_optinteger(L, 2, 2048); //default: 2Kio
	buffer = linearAlloc(bufferSize);
	
	Result ret = IRU_Initialize(buffer, bufferSize);
	if (ret) {
		lua_pushboolean(L, false);
		lua_pushinteger(L, ret);
		return 2;
	}
	IRU_SetBitRate(bitrate);
	
	lua_pushboolean(L, true);
	return 1;
}

/***
Disable the IR module.
@function shutdown
*/
static int ir_shutdown(lua_State *L) {
	IRU_Shutdown();
	
	return 0;
}

/***
Send some data over the IR module.
@function send
@tparam string data just some data
@tparam[opt=false] boolean wait set to `true` to wait until the data is sent.
*/
static int ir_send(lua_State *L) {
	u8 *data = (u8*)luaL_checkstring(L, 1);
	u32 wait = lua_toboolean(L, 2);
	
	IRU_SendData(data, sizeof(data), wait);
	
	return 0;
}

/***
Receive some data from the IR module.
@function receive
@tparam[opt=buffer size] number size bytes to receive
@tparam[opt=false] boolean wait wait until the data is received
*/
static int ir_receive(lua_State *L) {
	u32 size = luaL_optinteger(L, 1, 0x800);
	u32 wait = lua_toboolean(L, 2);
	u8 *data = 0;
	u32 *transfercount = 0;
	
	IRU_RecvData(data, size, 0x00, transfercount, wait);
	
	lua_pushstring(L, (const char *)data);
	
	return 1;
}

/***
Set the bitrate of the communication.
@function setBitRate
@tparam number bitrate new bitrate for the communication
*/
static int ir_setBitRate(lua_State *L) {
	u8 bitrate = luaL_checkinteger(L, 1);
	
	IRU_SetBitRate(bitrate);
	
	return 0;
}

/***
Return the actual bitrate of the communication.
@function getBitRate
@treturn number actual bitrate
*/
static int ir_getBitRate(lua_State *L) {
	u8 bitrate = 0;
	
	IRU_GetBitRate(&bitrate);
	
	lua_pushinteger(L, bitrate);
	
	return 1;
}

static const struct luaL_Reg ir_lib[] = {
	{"init",       ir_init      },
	{"shutdown",   ir_shutdown  },
	{"send",       ir_send      },
	{"receive",    ir_receive   },
	{"setBitRate", ir_setBitRate},
	{"getBitRate", ir_getBitRate},
	{NULL, NULL}
};

int luaopen_ir_lib(lua_State *L) {
	luaL_newlib(L, ir_lib);
	return 1;
}

void load_ir_lib(lua_State *L) {
	luaL_requiref(L, "ctr.ir", luaopen_ir_lib, 0);
}
