#include <3ds/types.h>
#include <3ds/services/hid.h>

#include <lua.h>
#include <lauxlib.h>

// Key list based on hid.h from the ctrulib by smealum
struct { PAD_KEY key; char *name; } hid_keys_name[] = {
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

static int hid_read(lua_State *L) {
	hidScanInput();

	return 0;
}

static int hid_keys(lua_State *L) {
	u32 kDown = hidKeysDown();
	u32 kHeld = hidKeysHeld();
	u32 kUp = hidKeysUp();

	lua_createtable(L, 0, 3);
	lua_newtable(L); // down table
	lua_newtable(L); // held table
	lua_newtable(L); // up table

	for (int i = 0; hid_keys_name[i].key; i++) {
		PAD_KEY key = hid_keys_name[i].key;
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

static int hid_touch(lua_State *L) {
	touchPosition pos;
	hidTouchRead(&pos);
	
	lua_pushinteger(L, pos.px);
	lua_pushinteger(L, pos.py);
	
	return 2;
}

static int hid_circle(lua_State *L) {
	circlePosition pos;
	hidCircleRead(&pos);
	
	lua_pushinteger(L, pos.dx);
	lua_pushinteger(L, pos.dy);
	
	return 2;
}

static int hid_accel(lua_State *L) {
	accelVector pos;
	hidAccelRead(&pos);
	
	lua_pushinteger(L, pos.x);
	lua_pushinteger(L, pos.y);
	lua_pushinteger(L, pos.z);
	
	return 3;
}

static int hid_gyro(lua_State *L) {
	angularRate pos;
	hidGyroRead(&pos);
	
	lua_pushinteger(L, pos.x);
	lua_pushinteger(L, pos.y);
	lua_pushinteger(L, pos.z);
	
	return 3;
}

static int hid_volume(lua_State *L) {
	u8 volume = 0;
	HIDUSER_GetSoundVolume(&volume);
	
	lua_pushinteger(L, volume);
	
	return 1;
}

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