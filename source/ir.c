#include <3ds/types.h>
#include <3ds/services/ir.h>
#include <3ds/linear.h>

#include <lualib.h>
#include <lauxlib.h>

u32 bufferSize = 0;
u32 *buffer;

static int ir_init(lua_State *L) {
	u8 bitrate = luaL_checkinteger(L, 1);
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

static int ir_shutdown(lua_State *L) {
	IRU_Shutdown();
	
	return 0;
}

static int ir_send(lua_State *L) {
	u8 *data = (u8*)luaL_checkstring(L, 1);
	u32 wait = lua_toboolean(L, 2);
	
	IRU_SendData(data, sizeof(data), wait);
	
	return 0;
}

static int ir_receive(lua_State *L) {
	u32 size = luaL_optinteger(L, 1, bufferSize);
	u32 wait = lua_toboolean(L, 2);
	u8 *data = 0;
	u32 *transfercount = 0;
	
	IRU_RecvData(data, size, 0x00, transfercount, wait);
	
	lua_pushstring(L, (const char *)data);
	
	return 1;
}

static int ir_setBitRate(lua_State *L) {
	u8 bitrate = luaL_checkinteger(L, 1);
	
	IRU_SetBitRate(bitrate);
	
	return 0;
}

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
