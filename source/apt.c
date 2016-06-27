/***
The `apt` module.
Used to manage the applets and application status.
@module ctr.apt
@usage local apt = require("ctr.apt")
*/

#include <stdlib.h>

#include <3ds/types.h>
#include <3ds/services/apt.h>

#include <lua.h>
#include <lauxlib.h>

/***
Set the app status.
@function setStatus
@tparam integer status the new app status
*/
static int apt_setStatus(lua_State *L) {
	APT_AppStatus status = luaL_checkinteger(L, 1);

	aptSetStatus(status);

	return 0;
}

/***
Get the app status.
@function getStatus
@treturn integer the app status
*/
static int apt_getStatus(lua_State *L) {
	APT_AppStatus status = aptGetStatus();
	
	lua_pushinteger(L, status);

	return 1;
}

/***
Return to the Home menu.
@function returnToMenu
*/
static int apt_returnToMenu(lua_State *L) {
	aptReturnToMenu();

	return 0;
}

/***
Get the power status.
@function getStatusPower
@treturn boolean true if the power button has been pressed
*/
static int apt_getStatusPower(lua_State *L) {
	u32 status = aptGetStatusPower();

	lua_pushboolean(L, status);

	return 1;
}

/***
Set the power status.
@function setStatusPower
@tparam boolean status new power status
*/
static int apt_setStatusPower(lua_State *L) {
	u32 status = lua_toboolean(L, 1);

	aptSetStatusPower(status);

	return 0;
}

/***
Signal that the application is ready for sleeping.
@function signalReadyForSleep
*/
static int apt_signalReadyForSleep(lua_State *L) {
	aptSignalReadyForSleep();

	return 0;
}

/***
Return the Home menu AppID.
@function getMenuAppID
@treturn number the AppID
*/
static int apt_getMenuAppID(lua_State *L) {
	lua_pushinteger(L, aptGetMenuAppID());

	return 1;
}

/***
Allow or not the system to enter sleep mode.
@function setSleepAllowed
@tparam boolean allowed `true` to allow, `false` to disallow
*/
static int apt_setSleepAllowed(lua_State *L) {
	bool allowed = lua_toboolean(L, 1);

	aptSetSleepAllowed(allowed);

	return 0;
}

/***
Check if sleep mode is allowed.
@function isSleepAllowed
@treturn boolean `true` is allowed, false if not.
*/
static int apt_isSleepAllowed(lua_State *L) {
	lua_pushboolean(L, aptIsSleepAllowed());

	return 1;
}

/***
Checks whether the system is a New 3DS.
@function isNew3DS
@treturn boolean `true` if it's a New3DS, false otherwise
*/
static int apt_isNew3DS(lua_State *L) {
	bool isNew3ds;

	APT_CheckNew3DS(&isNew3ds);

	lua_pushboolean(L, isNew3ds);

	return 1;
}

static const struct luaL_Reg apt_lib[] = {
	{ "setStatus",           apt_setStatus           },
	{ "getStatus",           apt_getStatus           },
	{ "returnToMenu",        apt_returnToMenu        },
	{ "getStatusPower",      apt_getStatusPower      },
	{ "setStatusPower",      apt_setStatusPower      },
	{ "signalReadyForSleep", apt_signalReadyForSleep },
	{ "getMenuAppID",        apt_getMenuAppID        },
	{ "setSleepAllowed",     apt_setSleepAllowed     },
	{ "isSleepAllowed",      apt_isSleepAllowed      },
	{ "isNew3DS",            apt_isNew3DS            },
	{NULL, NULL}
};

struct { char *name; int value; } apt_constants[] = {
	/***
	@field APPID_HOMEMENU
	*/
	{"APPID_HOMEMENU",           APPID_HOMEMENU          },
	/***
	@field APPID_CAMERA
	*/
	{"APPID_CAMERA",             APPID_CAMERA            },
	/***
	@field APPID_FRIENDS_LIST
	*/
	{"APPID_FRIENDS_LIST",       APPID_FRIENDS_LIST      },
	/***
	@field APPID_GAME_NOTES
	*/
	{"APPID_GAME_NOTES",         APPID_GAME_NOTES        },
	/***
	@field APPID_WEB
	*/
	{"APPID_WEB",                APPID_WEB               },
	/***
	@field APPID_INSTRUCTION_MANUAL
	*/
	{"APPID_INSTRUCTION_MANUAL", APPID_INSTRUCTION_MANUAL},
	/***
	@field APPID_NOTIFICATIONS
	*/
	{"APPID_NOTIFICATIONS",      APPID_NOTIFICATIONS     },
	/***
	@field APPID_MIIVERSE
	*/
	{"APPID_MIIVERSE",           APPID_MIIVERSE          },
	/***
	@field APPID_MIIVERSE_POSTING
	*/
	{"APPID_MIIVERSE_POSTING",   APPID_MIIVERSE_POSTING  },
	/***
	@field APPID_AMIIBO_SETTINGS
	*/
	{"APPID_AMIIBO_SETTINGS",    APPID_AMIIBO_SETTINGS   },
	/***
	@field APPID_APPLICATION
	*/
	{"APPID_APPLICATION",        APPID_APPLICATION       },
	/***
	@field APPID_ESHOP
	*/
	{"APPID_ESHOP",              APPID_ESHOP             },
	/***
	@field APPID_SOFTWARE_KEYBOARD
	*/
	{"APPID_SOFTWARE_KEYBOARD",  APPID_SOFTWARE_KEYBOARD },
	/***
	@field APPID_APPLETED
	*/
	{"APPID_APPLETED",           APPID_APPLETED          },
	/***
	@field APPID_PNOTE_AP
	*/
	{"APPID_PNOTE_AP",           APPID_PNOTE_AP          },
	/***
	@field APPID_SNOTE_AP
	*/
	{"APPID_SNOTE_AP",           APPID_SNOTE_AP          },
	/***
	@field APPID_ERROR
	*/
	{"APPID_ERROR",              APPID_ERROR             },
	/***
	@field APPID_MINT
	*/
	{"APPID_MINT",               APPID_MINT              },
	/***
	@field APPID_EXTRAPAD
	*/
	{"APPID_EXTRAPAD",           APPID_EXTRAPAD          },
	/***
	@field APPID_MEMOLIB
	*/
	{"APPID_MEMOLIB",            APPID_MEMOLIB           },
	/***
	@field APP_NOTINITIALIZED
	*/
	{"APP_NOTINITIALIZED",    APP_NOTINITIALIZED   },
	/***
	@field APP_RUNNING
	*/
	{"APP_RUNNING",           APP_RUNNING          },
	/***
	@field APP_SUSPENDED
	*/
	{"APP_SUSPENDED",         APP_SUSPENDED        },
	/***
	@field APP_EXITING
	*/
	{"APP_EXITING",           APP_EXITING          },
	/***
	@field APP_SUSPENDING
	*/
	{"APP_SUSPENDING",        APP_SUSPENDING       },
	/***
	@field APP_SLEEPMODE
	*/
	{"APP_SLEEPMODE",         APP_SLEEPMODE        },
	/***
	@field APP_PREPARE_SLEEPMODE
	*/
	{"APP_PREPARE_SLEEPMODE", APP_PREPARE_SLEEPMODE},
	/***
	@field APP_APPLETSTARTED
	*/
	{"APP_APPLETSTARTED",     APP_APPLETSTARTED    },
	/***
	@field APP_APPLETCLOSED
	*/
	{"APP_APPLETCLOSED",      APP_APPLETCLOSED     },
	/***
	@field APTSIGNAL_HOMEBUTTON
	*/
	{"APTSIGNAL_HOMEBUTTON",   APTSIGNAL_HOMEBUTTON  },
	/***
	@field APTSIGNAL_HOMEBUTTON2
	*/
	{"APTSIGNAL_HOMEBUTTON2",   APTSIGNAL_HOMEBUTTON2 },
	/***
	@field APTSIGNAL_SLEEP_QUERY
	*/
	{"APTSIGNAL_SLEEP_QUERY",  APTSIGNAL_SLEEP_QUERY },
	/***
	@field APTSIGNAL_SLEEP_CANCEL
	*/
	{"APTSIGNAL_SLEEP_CANCEL", APTSIGNAL_SLEEP_CANCEL},
	/***
	@field APTSIGNAL_SLEEP_ENTER
	*/
	{"APTSIGNAL_SLEEP_ENTER",  APTSIGNAL_SLEEP_ENTER },
	/***
	@field APTSIGNAL_WAKEUP
	*/
	{"APTSIGNAL_SLEEP_WAKEUP", APTSIGNAL_SLEEP_WAKEUP},
	/***
	@field APTSIGNAL_SHUTDOWN
	*/
	{"APTSIGNAL_SHUTDOWN",     APTSIGNAL_SHUTDOWN    },
	/***
	@field APTSIGNAL_POWERBUTTON
	*/
	{"APTSIGNAL_POWERBUTTON",  APTSIGNAL_POWERBUTTON },
	/***
	@field APTSIGNAL_POWERBUTTON2
	*/
	{"APTSIGNAL_POWERBUTTON2", APTSIGNAL_POWERBUTTON2},
	/***
	@field APTSIGNAL_TRY_SLEEP
	*/
	{"APTSIGNAL_TRY_SLEEP",    APTSIGNAL_TRY_SLEEP   },
	/***
	@field APTSIGNAL_ORDERTOCLOSE
	*/
	{"APTSIGNAL_ORDERTOCLOSE", APTSIGNAL_ORDERTOCLOSE},
	/***
	@field APTHOOK_ONSUSPEND
	*/
	{"APTHOOK_ONSUSPEND", APTHOOK_ONSUSPEND},
	/***
	@field APTHOOK_ONRESTORE
	*/
	{"APTHOOK_ONRESTORE", APTHOOK_ONRESTORE},
	/***
	@field APTHOOK_ONSLEEP
	*/
	{"APTHOOK_ONSLEEP",   APTHOOK_ONSLEEP  },
	/***
	@field APTHOOK_ONWAKEUP
	*/
	{"APTHOOK_ONWAKEUP",  APTHOOK_ONWAKEUP },
	/***
	@field APTHOOK_ONEXIT
	*/
	{"APTHOOK_ONEXIT",    APTHOOK_ONEXIT   },
	/***
	@field APTHOOK_COUNT
	*/
	{"APTHOOK_COUNT",     APTHOOK_COUNT    },
	{NULL, 0}
};

int luaopen_apt_lib(lua_State *L) {
	aptInit();
	
	luaL_newlib(L, apt_lib);
	
	for (int i = 0; apt_constants[i].name; i++) {
		lua_pushinteger(L, apt_constants[i].value);
		lua_setfield(L, -2, apt_constants[i].name);
	}
	
	return 1;
}

void load_apt_lib(lua_State *L) {
	luaL_requiref(L, "ctr.apt", luaopen_apt_lib, false);
}

void unload_apt_lib(lua_State *L) {
	aptExit();
}
