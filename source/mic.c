/***
The `mic` module.
@module ctr.mic
@usage local mic = require("ctr.mic")
*/

#include <3ds/types.h>
#include <3ds/services/mic.h>

#include <lualib.h>
#include <lauxlib.h>

#include <malloc.h>
#include <string.h>

u8* buff;
u32 bufferSize = 0;

/***
Initialize the mic module.
@function init
@tparam[opt=0x50000] number bufferSize size of the buffer (must be a multiple of 0x1000)
*/
static int mic_init(lua_State *L) {
	bufferSize = luaL_optinteger(L, 1, 0x50000);
	
	buff = memalign(0x1000, bufferSize);
	if (buff == NULL) {
		lua_pushnil(L);
		lua_pushstring(L, "Couldn't allocate buffer");
		return 2;
	}
	Result ret = micInit(buff, bufferSize);
	if (ret) {
		free(buff);
		lua_pushnil(L);
		lua_pushinteger(L, ret);
		return 2;
	}
	
	lua_pushboolean(L, true);
	return 1;
}

/***
Shutdown the mic module.
@function shutdown
*/
static int mic_shutdown(lua_State *L) {
	micExit();
	free(buff);
	return 0;
}

/***
Start sampling from the mic.
@function startSampling
@tparam[opt="PCM8"] encoding encoding encoding of the data to record, can be `"PCM8"` or `"PCM16"`
@tparam[opt=8180] number rate sampling rate, can be `8180`, `10910`, `16360` or `32730`
@tparam[opt=false] boolean loop if true, loop back to the beginning of the buffer when the end is reached
@tparam[opt=bufferFreeSize-4] number size size of audio data to write to the buffer, can be reduced to fit in the buffer
@tparam[opt=false] boolean restart if `true`, start at position 0 in the buffer; if `false`, start after the last sample
*/
static int mic_startSampling(lua_State *L) {
	const char *encodingArg = luaL_optstring(L, 1, "PCM8");
	MICU_Encoding encoding = MICU_ENCODING_PCM8;
	if (strcmp(encodingArg, "PCM16")) {
		encoding = MICU_ENCODING_PCM16;
	}
	
	u16 rateArg = luaL_optinteger(L, 2, 8180);
	MICU_SampleRate rate = MICU_SAMPLE_RATE_8180;
	switch (rateArg) {
		case 10910:
			rate = MICU_SAMPLE_RATE_10910;
		case 16360:
			rate = MICU_SAMPLE_RATE_16360;
		case 32730:
			rate = MICU_SAMPLE_RATE_32730;
	}
	
	bool loop = false;
	if (lua_isboolean(L, 3))
		loop = lua_toboolean(L, 3);
	
	
	u32 currentSampleSize = micGetSampleDataSize();
	u32 size = luaL_optinteger(L, 4, bufferSize-currentSampleSize-4);
	if (size > (bufferSize-currentSampleSize-4)) {
		size = bufferSize-currentSampleSize-4;
	}
	
	u32 offset = currentSampleSize;
	if (lua_isboolean(L, 5) && lua_toboolean(L, 5)) // restart to 0
		offset = 0;
	
	MICU_StartSampling(encoding, rate, offset, size, loop);
	
	return 0;
}

/***
Stop sampling from the mic.
@function stopSampling
*/
static int mic_stopSampling(lua_State *L) {
	MICU_StopSampling();
	
	return 0;
}

/***
Adjust the sampling rate.
@function adjustSampling
@tparam number rate sampling rate, can be `8180`, `10910`, `16360` or `32730`
*/
static int mic_adjustSampling(lua_State *L) {
	u16 rateArg = luaL_checkinteger(L, 1);
	MICU_SampleRate rate = MICU_SAMPLE_RATE_8180;
	switch (rateArg) {
		case 10910:
			rate = MICU_SAMPLE_RATE_10910;
		case 16360:
			rate = MICU_SAMPLE_RATE_16360;
		case 32730:
			rate = MICU_SAMPLE_RATE_32730;
	}
	
	MICU_AdjustSampling(rate);
	
	return 0;
}

/***
Check whether the mic is sampling.
@function isSampling
@treturn boolean `true` if sampling
*/
static int mic_isSampling(lua_State *L) {
	bool sampling;
	MICU_IsSampling(&sampling);
	
	lua_pushboolean(L, sampling);
	return 1;
}

/***
Return a string containing the raw sampled audio data.
@function getData
@tparam[opt=true] boolean lastSampleOnly set to `true` to only get the last sample, and to `false` to get everything
@treturn string raw audio data
*/
static int mic_getData(lua_State *L) {
	bool last = false;
	if (lua_isboolean(L, 1))
		last = lua_toboolean(L, 1);
	
	u32 offset = 0;
	if (last) {
		offset = micGetLastSampleOffset();
	}
	u32 size = micGetSampleDataSize();
	
	char* data = malloc(size-offset);
	for (int i=offset;i<size;i++) {
		data[i-offset] = buff[i%size];
	}
	lua_pushlstring(L, data, size-offset);
	
	return 1;
}

/***
Set the gain of the mic.
@function setGain
@tparam number gain gain
*/
static int mic_setGain(lua_State *L) {
	u8 gain = luaL_checkinteger(L, 1);
	
	MICU_SetGain(gain);
	
	return 0;
}

/***
Return the gain of the mic.
@function getGain
@treturn number
*/
static int mic_getGain(lua_State *L) {
	u8 gain;
	MICU_GetGain(&gain);
	
	lua_pushinteger(L, gain);
	return 1;
}

/***
Power on/off the mic.
@function setPower
@tparam boolean power `true` to power on, `false` to power off
*/
static int mic_setPower(lua_State *L) {
	bool power = false;
	if (lua_isboolean(L, 1)) {
		power = lua_toboolean(L, 1);
	} else {
		luaL_error(L, "bad argument #1 to 'setPower' (boolean expected, got %s)", lua_typename(L, lua_type(L, 1)));
	}
	
	MICU_SetPower(power);
	
	return 0;
}

/***
Return the power status of the mic.
@function getPower
@treturn boolean `true` if powered, `false` if not
*/
static int mic_getPower(lua_State *L) {
	bool power;
	MICU_GetPower(&power);
	
	lua_pushboolean(L, power);
	return 1;
}

/***
Set whether to clamp the mic input.
@function setClamp
@tparam boolean clamp `true` to clamp, `false` to not
*/
static int mic_setClamp(lua_State *L) {
	bool clamp = false;
	if (lua_isboolean(L, 1)) {
		clamp = lua_toboolean(L, 1);
	} else {
		luaL_error(L, "bad argument #1 to 'setClamp' (boolean expected, got %s)", lua_typename(L, lua_type(L, 1)));
	}
	
	MICU_SetClamp(clamp);
	
	return 0;
}

/***
Check if the mic input is clamped.
@function getClamp
@treturn boolean `true` if clamped, `false` if not
*/
static int mic_getClamp(lua_State *L) {
	bool clamp;
	MICU_GetClamp(&clamp);
	
	lua_pushboolean(L, clamp);
	return 1;
}

/***
Allow or not to sample when the shell is closed.
@function allowShellClosed
@tparam boolean allow `true` to allow, `false` to not.
*/
static int mic_allowShellClosed(lua_State *L) {
	bool allow = false;
	if (lua_isboolean(L, 1)) {
		allow = lua_toboolean(L, 1);
	} else {
		luaL_error(L, "bad argument #1 to 'allowShellClosed' (boolean expected, got %s)", lua_typename(L, lua_type(L, 1)));
	}
	
	MICU_SetAllowShellClosed(allow);
	
	return 0;
}

static const struct luaL_Reg mic_lib[] = {
	{"init",             mic_init            },
	{"shutdown",         mic_shutdown        },
	{"startSampling",    mic_startSampling   },
	{"stopSampling",     mic_stopSampling    },
	{"adjustSampling",   mic_adjustSampling  },
	{"isSampling",       mic_isSampling      },
	{"getData",          mic_getData         },
	{"setGain",          mic_setGain         },
	{"getGain",          mic_getGain         },
	{"setPower",         mic_setPower        },
	{"getPower",         mic_getPower        },
	{"setClamp",         mic_setClamp        },
	{"getClamp",         mic_getClamp        },
	{"allowShellClosed", mic_allowShellClosed},
	{NULL, NULL}
};

int luaopen_mic_lib(lua_State *L) {
	luaL_newlib(L, mic_lib);
	return 1;
}

void load_mic_lib(lua_State *L) {
	luaL_requiref(L, "ctr.mic", luaopen_mic_lib, 0);
}
