/***
The `ir` module.
@module ctr.ir
@usage local ir = require("ctr.ir")
*/
#include <3ds/types.h>
#include <3ds/services/ir.h>
//#include <3ds/linear.h>

#include <lualib.h>
#include <lauxlib.h>

#include <string.h>

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
@treturn[1] boolean `true` if everything went fine
@treturn[2] boolean `false` in case of error
@treturn[2] integer error code
*/
static int ir_init(lua_State *L) {
	u8 bitrate = luaL_optinteger(L, 1, 6);
	
	Result ret = IRU_Initialize();
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
@treturn[1] boolean `true` if everything went fine
@treturn[2] boolean `false` in case of error
@treturn[2] integer error code
*/
static int ir_shutdown(lua_State *L) {
	Result ret = IRU_Shutdown();
	if (ret) {
		lua_pushboolean(L, false);
		lua_pushinteger(L, ret);
		return 2;
	}
	
	lua_pushboolean(L, true);
	return 1;
}

/***
Send some data over the IR module.
@function send
@tparam string data just some data
@tparam[opt=false] boolean wait set to `true` to wait until the data is sent.
@treturn[1] boolean `true` if everything went fine
@treturn[2] boolean `false` in case of error
@treturn[2] integer error code
*/
static int ir_send(lua_State *L) {
	u8 *data = (u8*)luaL_checkstring(L, 1);
	u32 wait = lua_toboolean(L, 2);
	
	Result ret = IRU_StartSendTransfer(data, strlen((const char*)data));
	if (wait)
		IRU_WaitSendTransfer();
	
	if (ret) {
		lua_pushboolean(L, false);
		lua_pushinteger(L, ret);
		return 2;
	}
	
	lua_pushboolean(L, true);
	return 1;
}

/***
Receive some data from the IR module.
@function receive
@tparam number size bytes to receive
@tparam[opt=false] boolean wait wait until the data is received
@treturn[1] string data
@treturn[2] nil in case of error
@treturn[2] integer error code
*/
static int ir_receive(lua_State *L) {
	u32 size = luaL_checkinteger(L, 1);
	u32 wait = lua_toboolean(L, 2);
	u8 *data = 0;
	u32 transfercount = 0;
	
	Result ret = iruRecvData(data, size, 0x00, &transfercount, wait);
	if (ret) {
		lua_pushnil(L);
		lua_pushinteger(L, ret);
		return 2;
	}
	
	lua_pushlstring(L, (const char *)data, (size_t)transfercount);
	
	return 1;
}

/***
Set the bitrate of the communication.
@function setBitRate
@tparam number bitrate new bitrate for the communication
@treturn[1] boolean `true` if everything went fine
@treturn[2] boolean `false` in case of error
@treturn[2] integer error code
*/
static int ir_setBitRate(lua_State *L) {
	u8 bitrate = luaL_checkinteger(L, 1);
	
	Result ret = IRU_SetBitRate(bitrate);
	if (ret) {
		lua_pushboolean(L, false);
		lua_pushinteger(L, ret);
		return 2;
	}
	
	lua_pushboolean(L, true);

	return 1;
}

/***
Return the actual bitrate of the communication.
@function getBitRate
@treturn[1] number actual bitrate
@treturn[2] nil in case of error
@treturn[2] integer error code
*/
static int ir_getBitRate(lua_State *L) {
	u8 bitrate = 0;
	
	Result ret = IRU_GetBitRate(&bitrate);
	if (ret) {
		lua_pushnil(L);
		lua_pushinteger(L, ret);
		return 2;
	}
	
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
