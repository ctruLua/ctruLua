/***
The `hid` module.
The circle pad pro is supported, it's keys replace de "3ds only" keys
@module ctr.hid
@usage local hid = require("ctr.hid")
*/
#include <3ds/types.h>
#include <3ds/services/hid.h>

#include <lua.h>
#include <lauxlib.h>

/***
Keys list
@table keys
@field a A
@field b B
@field select Select
@field start Start
@field dRight D-Pad right
@field dLeft D-Pad Left
@field dUp D-Pad Up
@field dDown D-Pad Down
@field r R trigger
@field l L trigger
@field x X
@field y Y
@field zl ZL trigger (new3ds only)
@field zr ZR trigger (new3ds only)
@field touch
@field cstickRight C-Stick right (new3ds only)
@field cstickLeft C-Stick left (new3ds only)
@field cstickUp C-Stick up (new3ds only)
@field cstickDown C-Stick down (new3ds only)
@field cpadRight Circle pad right
@field cpadLeft Circle pad left
@field cpadUp Circle pad up
@field cpadDown Circle pad down
@field up Generic up
@field down Generic down
@field left Generic left
@field right Generic right
*/

/***
Keys states
@table states
@field down keys which have been just pressed
@field held keys which are held down
@field up keys whick are been just released
*/

// Key list based on hid.h from the ctrulib by smealum
struct { u32 key; char *name; } hid_keys_name[] = {
	{ KEY_A            , "a"          },
	{ KEY_B            , "b"          },
	{ KEY_SELECT       , "select"     },
	{ KEY_START        , "start"      },
	{ KEY_DRIGHT       , "dRight"     },
	{ KEY_DLEFT        , "dLeft"      },
	{ KEY_DUP          , "dUp"        },
	{ KEY_DDOWN        , "dDown"      },
	{ KEY_R            , "r"          },
	{ KEY_L            , "l"          },
	{ KEY_X            , "x"          },
	{ KEY_Y            , "y"          },
	{ KEY_ZL           , "zl"         }, // (new 3DS only)
	{ KEY_ZR           , "zr"         }, // (new 3DS only)
	{ KEY_TOUCH        , "touch"      }, // Not actually provided by HID
	{ KEY_CSTICK_RIGHT , "cstickRight"}, // c-stick (new 3DS only)
	{ KEY_CSTICK_LEFT  , "cstickLeft" }, // c-stick (new 3DS only)
	{ KEY_CSTICK_UP    , "cstickUp"   }, // c-stick (new 3DS only)
	{ KEY_CSTICK_DOWN  , "cstickDown" }, // c-stick (new 3DS only)
	{ KEY_CPAD_RIGHT   , "cpadRight"  }, // circle pad
	{ KEY_CPAD_LEFT    , "cpadLeft"   }, // circle pad
	{ KEY_CPAD_UP      , "cpadUp"     }, // circle pad
	{ KEY_CPAD_DOWN    , "cpadDown"   }, // circle pad

	// Generic catch-all directions
	{ KEY_UP           , "up"         },
	{ KEY_DOWN         , "down"       },
	{ KEY_LEFT         , "left"       },
	{ KEY_RIGHT        , "right"      },
	{ 0, NULL }
};

/***
Refresh the HID state.
@function read
*/
static int hid_read(lua_State *L) {
	hidScanInput();

	return 0;
}

/***
Return the keys states as `state.key` in a table.
@function keys
@treturn table keys states
@usage
-- Just an example
hid.read()
local keys = hid.keys()
if keys.held.a then
	-- do stuff
end
*/
static int hid_keys(lua_State *L) {
	u32 kDown = hidKeysDown();
	u32 kHeld = hidKeysHeld();
	u32 kUp = hidKeysUp();

	lua_createtable(L, 0, 3);
	lua_newtable(L); // down table
	lua_newtable(L); // held table
	lua_newtable(L); // up table

	for (int i = 0; hid_keys_name[i].key; i++) {
		u32 key = hid_keys_name[i].key;
		char *name = hid_keys_name[i].name;
		
		if (kDown & key) {
			lua_pushboolean(L, true);
			lua_setfield(L, -4, name);
		}
		if (kHeld & key) {
			lua_pushboolean(L, true);
			lua_setfield(L, -3, name);
		}
		if (kUp & key) {
			lua_pushboolean(L, true);
			lua_setfield(L, -2, name);
		}
	}

	lua_setfield(L, -4, "up");
	lua_setfield(L, -3, "held");
	lua_setfield(L, -2, "down");

	return 1;
}

/***
Return the touch position on the touch screen.
`0,0` is the top-left corner.
@function touch
@treturn number X position
@treturn number Y position
*/
static int hid_touch(lua_State *L) {
	touchPosition pos;
	hidTouchRead(&pos);
	
	lua_pushinteger(L, pos.px);
	lua_pushinteger(L, pos.py);
	
	return 2;
}

/***
Return the circle pad position.
`0,0` is the center position. Warning: the circle pad doesn't always go back to `0,0`.
@function circle
@treturn number X position
@treturn number Y position
*/
static int hid_circle(lua_State *L) {
	circlePosition pos;
	hidCircleRead(&pos);
	
	lua_pushinteger(L, pos.dx);
	lua_pushinteger(L, pos.dy);
	
	return 2;
}

/***
Return the accelerometer vector
@function accel
@treturn number X acceleration
@treturn number Y acceleration
@treturn number Z acceleration
*/
static int hid_accel(lua_State *L) {
	accelVector pos;
	hidAccelRead(&pos);
	
	lua_pushinteger(L, pos.x);
	lua_pushinteger(L, pos.y);
	lua_pushinteger(L, pos.z);
	
	return 3;
}

/***
Return the gyroscope rate.
@function gyro
@treturn number roll
@treturn number pitch
@treturn number yaw
*/
static int hid_gyro(lua_State *L) {
	angularRate pos;
	hidGyroRead(&pos);
	
	lua_pushinteger(L, pos.x);
	lua_pushinteger(L, pos.y);
	lua_pushinteger(L, pos.z);
	
	return 3;
}

/***
Return the sound volume.
@function volume
@treturn number volume (`0` to `63`)
*/
static int hid_volume(lua_State *L) {
	u8 volume = 0;
	HIDUSER_GetSoundVolume(&volume);
	
	lua_pushinteger(L, volume);
	
	return 1;
}

/***
Return the 3D cursor position.
@function pos3d
@treturn number 3d cursor position (`0` to `1`)
*/
static int hid_3d(lua_State *L) {
  float slider = (*(float*)0x1FF81080);
  
  lua_pushnumber(L, slider);
  
  return 1;
}

static const struct luaL_Reg hid_lib[] = {
	{ "read",   hid_read   },
	{ "keys",   hid_keys   },
	{ "touch",  hid_touch  },
	{ "circle", hid_circle },
	{ "accel",  hid_accel  },
	{ "gyro",   hid_gyro   },
	{ "volume", hid_volume },
	{ "pos3d",  hid_3d     },
	{ NULL, NULL }
};

int luaopen_hid_lib(lua_State *L) {
	luaL_newlib(L, hid_lib);
	return 1;
}

void load_hid_lib(lua_State *L) {
	HIDUSER_EnableAccelerometer();
	HIDUSER_EnableGyroscope();

	luaL_requiref(L, "ctr.hid", luaopen_hid_lib, false);
}

void unload_hid_lib(lua_State *L) {
	HIDUSER_DisableAccelerometer();
	HIDUSER_DisableGyroscope();
}
