/***
The `apt` module.
Used to manage the applets and application status.
@module ctr.apt
@usage local apt = require("ctr.apt")
*/

#include <string.h>

#include <3ds/types.h>
#include <3ds/services/apt.h>

#include <lua.h>
#include <lauxlib.h>

// Used in callbacks functions.
lua_State *luaState;

// Hook userdata. Represents a APT status hook (see apt.hook).
typedef struct {
	aptHookCookie cookie; // hook cookie
	int refFn; // reference to the function
	int ref; // reference to self
} hook_userdata;

/***
Return the menu's AppID.
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
Sets up an APT status hook.
@function hook
@tparam function callback function to call when APT's status changes. The first argument is an APT hook type constant.
@treturn Hook Hook object
@see :unhook
*/
void hookFn(APT_HookType type, hook_userdata *hook) { // calls the lua hook function
	lua_rawgeti(luaState, LUA_REGISTRYINDEX, hook->refFn);
	lua_pushinteger(luaState, type);
	lua_call(luaState, 1, 0);
}
static int apt_hook(lua_State *L) {
	if (!lua_isfunction(L, 1)) luaL_error(L, "bad argument #1 to 'hook' (function expected)");

	// Create hook object
	hook_userdata *hook = lua_newuserdata(L, sizeof(hook));
	luaL_getmetatable(L, "LHook");
	lua_setmetatable(L, -2);

	// Reference to the function
	lua_pushvalue(L, 1);
	hook->refFn = luaL_ref(L, LUA_REGISTRYINDEX);

	// Add reference to self so it isn't collected
	lua_pushvalue(L, -1);
	hook->ref = luaL_ref(L, LUA_REGISTRYINDEX);

	aptHook(&hook->cookie, (aptHookFn)hookFn, hook);

	return 1;
}

/***
Sets the function to be called when an APT message from another applet is received.
@function setMessageCallback
@tparam function callback function. The first argument is the sender APPID, and the second argument is the message string.
*/
void callbackFn(void* user, NS_APPID sender, void* msg, size_t msgsize) { // calls the lua callback function
	lua_getfield(luaState, LUA_REGISTRYINDEX, "aptMessageCallback");
	lua_pushinteger(luaState, sender);
	lua_pushlstring(luaState, msg, msgsize);
	lua_call(luaState, 2, 0);
}
static int apt_setMessageCallback(lua_State *L) {
	if (!lua_isfunction(L, 1)) luaL_error(L, "bad argument #1 to 'hook' (function expected)");

	// Stores callback function
	lua_pushvalue(L, 1);
	lua_setfield(L, LUA_REGISTRYINDEX, "aptMessageCallback");

	aptSetMessageCallback(callbackFn, NULL);

	return 0;
}

/***
Launches a library applet.
@function launchLibraryApplet
@tparam APPID appId application ID of the applet to launch
@treturn boolean whether the application should continue running after the library applet launch
*/
static int apt_launchLibraryApplet(lua_State *L) {
	NS_APPID appId = luaL_checkinteger(L, 1);

	// To set launch parameters and get result data on exit, use the lib dedicated to the applet.
	u32 aptbuf[0x400/4];
	memset(aptbuf, 0, sizeof(aptbuf));

	bool shouldContinue = aptLaunchLibraryApplet(appId, aptbuf, sizeof(aptbuf), 0);

	lua_pushboolean(L, shouldContinue);

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

/***
Hook object
@section
*/
/***
Removes the APT status hook.
@function :unhook
*/
static int hook_object_unhook(lua_State *L) {
	hook_userdata *hook = luaL_checkudata(L, 1, "LHook");

	aptUnhook(&hook->cookie);

	luaL_unref(L, LUA_REGISTRYINDEX, hook->refFn); // release reference to function
	luaL_unref(L, LUA_REGISTRYINDEX, hook->ref); // release reference to self

	return 0;
}

// Font object methods
static const struct luaL_Reg hook_object_methods[] = {
	{ "unhook",              hook_object_unhook      },
	{ NULL, NULL }
};

// Library functions
static const struct luaL_Reg apt_lib[] = {
	{ "getMenuAppID",        apt_getMenuAppID        },
	{ "setSleepAllowed",     apt_setSleepAllowed     },
	{ "isSleepAllowed",      apt_isSleepAllowed      },
	{ "hook",                apt_hook                },
	{ "setMessageCallback",  apt_setMessageCallback  },
	{ "launchLibraryApplet", apt_launchLibraryApplet },
	{ "isNew3DS",            apt_isNew3DS            },
	{ NULL, NULL }
};

// Constants
struct { char *name; int value; } apt_constants[] = {
	/***
	NS Applications IDs constants
	@section
	*/
	/***
	@field APPID_NONE
	*/
	{ "APPID_NONE",               APPID_NONE               },
	/***
	Home Menu
	@field APPID_HOMEMENU
	*/
	{ "APPID_HOMEMENU",           APPID_HOMEMENU           },
	/***
	Camera applet
	@field APPID_CAMERA
	*/
	{ "APPID_CAMERA",             APPID_CAMERA             },
	/***
	Friends List applet
	@field APPID_FRIENDS_LIST
	*/
	{ "APPID_FRIENDS_LIST",       APPID_FRIENDS_LIST       },
	/***
	Games Notes applet
	@field APPID_GAME_NOTES
	*/
	{ "APPID_GAME_NOTES",         APPID_GAME_NOTES         },
	/***
	Internet Browser
	@field APPID_WEB
	*/
	{ "APPID_WEB",                APPID_WEB                },
	/***
	Instruction Manual applet
	@field APPID_INSTRUCTION_MANUAL
	*/
	{ "APPID_INSTRUCTION_MANUAL", APPID_INSTRUCTION_MANUAL },
	/***
	Notifications applet
	@field APPID_NOTIFICATIONS
	*/
	{ "APPID_NOTIFICATIONS",      APPID_NOTIFICATIONS      },
	/***
	Miiverse applet (olv)
	@field APPID_MIIVERSE
	*/
	{ "APPID_MIIVERSE",           APPID_MIIVERSE           },
	/***
	Miiverse posting applet (solv3)
	@field APPID_MIIVERSE_POSTING
	*/
	{ "APPID_MIIVERSE_POSTING",   APPID_MIIVERSE_POSTING   },
	/***
	Amiibo settings applet (cabinet)
	@field APPID_AMIIBO_SETTINGS
	*/
	{ "APPID_AMIIBO_SETTINGS",    APPID_AMIIBO_SETTINGS    },
	/***
	Application
	@field APPID_APPLICATION
	*/
	{ "APPID_APPLICATION",        APPID_APPLICATION        },
	/***
	eShop (tiger)
	@field APPID_ESHOP
	*/
	{ "APPID_ESHOP",              APPID_ESHOP              },
	/***
	Software Keyboard
	@field APPID_SOFTWARE_KEYBOARD
	*/
	{ "APPID_SOFTWARE_KEYBOARD",  APPID_SOFTWARE_KEYBOARD  },
	/***
	appletEd
	@field APPID_APPLETED
	*/
	{ "APPID_APPLETED",           APPID_APPLETED           },
	/***
	PNOTE_AP
	@field APPID_PNOTE_AP
	*/
	{ "APPID_PNOTE_AP",           APPID_PNOTE_AP           },
	/***
	SNOTE_AP
	@field APPID_SNOTE_AP
	*/
	{ "APPID_SNOTE_AP",           APPID_SNOTE_AP           },
	/***
	error
	@field APPID_ERROR
	*/
	{ "APPID_ERROR",              APPID_ERROR              },
	/***
	mint
	@field APPID_MINT
	*/
	{ "APPID_MINT",               APPID_MINT               },
	/***
	extrapad
	@field APPID_EXTRAPAD
	*/
	{ "APPID_EXTRAPAD",           APPID_EXTRAPAD           },
	/***
	memolib
	@field APPID_MEMOLIB
	*/
	{ "APPID_MEMOLIB",            APPID_MEMOLIB            },

	/***
	APT applet position constants
	@section
	*/
	/***
	No position specified
	@field APTPOS_NONE
	*/
	{ "APTPOS_NONE",              APTPOS_NONE              },
	/***
	Application
	@field APTPOS_APP
	*/
	{ "APTPOS_APP",               APTPOS_APP               },
	/***
	Application library (?)
	@field APTPOS_APPLIB
	*/
	{ "APTPOS_APPLIB",            APTPOS_APPLIB            },
	/***
	System applet
	@field APTPOS_SYS
	*/
	{ "APTPOS_SYS",               APTPOS_SYS               },
	/***
	System library (?)
	@field APTPOS_SYSLIB
	*/
	{ "APTPOS_SYSLIB",            APTPOS_SYSLIB            },
	/***
	Resident applet
	@field APTPOS_RESIDENT
	*/
	{ "APTPOS_RESIDENT",          APTPOS_RESIDENT          },

	/***
	APT query reply constants
	@section
	*/
	/***
	@field APTREPLY_REJECT
	*/
	{ "APTREPLY_REJECT",           APTREPLY_REJECT           },
	/***
	@field APTREPLY_ACCEPT
	*/
	{ "APTREPLY_ACCEPT",           APTREPLY_ACCEPT           },
	/***
	@field APTREPLY_LATER
	*/
	{ "APTREPLY_LATER",            APTREPLY_LATER            },

	/***
	APT signals constants
	@section
	*/
	/***
	No signal received
	@field APTSIGNAL_NONE
	*/
	{ "APTSIGNAL_NONE",            APTSIGNAL_NONE            },
	/***
	HOME button pressed
	@field APTSIGNAL_HOMEBUTTON
	*/
	{ "APTSIGNAL_HOMEBUTTON",      APTSIGNAL_HOMEBUTTON      },
	/***
	HOME button pressed (again?)
	@field APTSIGNAL_HOMEBUTTON2
	*/
	{ "APTSIGNAL_HOMEBUTTON2",     APTSIGNAL_HOMEBUTTON2     },
	/***
	Prepare to enter sleep mode
	@field APTSIGNAL_SLEEP_QUERY
	*/
	{ "APTSIGNAL_SLEEP_QUERY",     APTSIGNAL_SLEEP_QUERY     },
	/***
	Triggered when ptm:s GetShellStatus() returns 5
	@field APTSIGNAL_SLEEP_CANCEL
	*/
	{ "APTSIGNAL_SLEEP_CANCEL",    APTSIGNAL_SLEEP_CANCEL    },
	/***
	Enter sleep mode
	@field APTSIGNAL_SLEEP_ENTER
	*/
	{ "APTSIGNAL_SLEEP_ENTER",     APTSIGNAL_SLEEP_ENTER     },
	/***
	Wake from sleep mode
	@field APTSIGNAL_WAKEUP
	*/
	{ "APTSIGNAL_SLEEP_WAKEUP",    APTSIGNAL_SLEEP_WAKEUP    },
	/***
	Shutdown
	@field APTSIGNAL_SHUTDOWN
	*/
	{ "APTSIGNAL_SHUTDOWN",        APTSIGNAL_SHUTDOWN        },
	/***
	POWER button pressed
	@field APTSIGNAL_POWERBUTTON
	*/
	{ "APTSIGNAL_POWERBUTTON",     APTSIGNAL_POWERBUTTON     },
	/***
	POWER button cleared (?)
	@field APTSIGNAL_POWERBUTTON2
	*/
	{ "APTSIGNAL_POWERBUTTON2",    APTSIGNAL_POWERBUTTON2    },
	/***
	System sleeping (?)
	@field APTSIGNAL_TRY_SLEEP
	*/
	{ "APTSIGNAL_TRY_SLEEP",       APTSIGNAL_TRY_SLEEP       },
	/***
	Order to close (such as when an error happens?)
	@field APTSIGNAL_ORDERTOCLOSE
	*/
	{ "APTSIGNAL_ORDERTOCLOSE",    APTSIGNAL_ORDERTOCLOSE    },

	/***
	APT commands constants
	@section
	*/
	/***
	No command received
	@field APTCMD_NONE
	*/
	{ "APTCMD_NONE",               APTCMD_NONE               },
	/***
	Applet should wake up
	@field APTCMD_WAKEUP
	*/
	{ "APTCMD_WAKEUP",             APTCMD_WAKEUP             },
	/***
	Source applet sent us a parameter
	@field APTCMD_REQUEST
	*/
	{ "APTCMD_REQUEST",            APTCMD_REQUEST            },
	/***
	Target applet replied to our parameter
	@field APTCMD_RESPONSE
	*/
	{ "APTCMD_RESPONSE",           APTCMD_RESPONSE           },
	/***
	Exit (??)
	@field APTCMD_EXIT
	*/
	{ "APTCMD_EXIT",               APTCMD_EXIT               },
	/***
	Message (??)
	@field APTCMD_MESSAGE
	*/
	{ "APTCMD_MESSAGE",            APTCMD_MESSAGE            },
	/***
	HOME button pressed once
	@field APTCMD_HOMEBUTTON_ONCE
	*/
	{ "APTCMD_HOMEBUTTON_ONCE",    APTCMD_HOMEBUTTON_ONCE    },
	/***
	HOME button pressed twice (double-pressed)
	@field APTCMD_HOMEBUTTON_TWICE
	*/
	{ "APTCMD_HOMEBUTTON_TWICE",   APTCMD_HOMEBUTTON_TWICE   },
	/***
	DSP should sleep (manual DSP rights related?)
	@field APTCMD_DSP_SLEEP
	*/
	{ "APTCMD_DSP_SLEEP",          APTCMD_DSP_SLEEP          },
	/***
	DSP should wake up (manual DSP rights related?)
	@field APTCMD_DSP_WAKEUP
	*/
	{ "APTCMD_DSP_WAKEUP",         APTCMD_DSP_WAKEUP         },
	/***
	Applet wakes up due to a different applet exiting
	@field APTCMD_WAKEUP_EXIT
	*/
	{ "APTCMD_WAKEUP_EXIT",        APTCMD_WAKEUP_EXIT        },
	/***
	Applet wakes up after being paused through HOME menu
	@field APTCMD_WAKEUP_PAUSE
	*/
	{ "APTCMD_WAKEUP_PAUSE",       APTCMD_WAKEUP_PAUSE       },
	/***
	Applet wakes up due to being cancelled
	@field APTCMD_WAKEUP_CANCEL
	*/
	{ "APTCMD_WAKEUP_CANCEL",      APTCMD_WAKEUP_CANCEL      },
	/***
	Applet wakes up due to all applets being cancelled
	@field APTCMD_WAKEUP_CANCELALL
	*/
	{ "APTCMD_WAKEUP_CANCELALL",   APTCMD_WAKEUP_CANCELALL   },
	/***
	Applet wakes up due to POWER button being pressed (?)
	@field APTCMD_WAKEUP_POWERBUTTON
	*/
	{ "APTCMD_WAKEUP_POWERBUTTON", APTCMD_WAKEUP_POWERBUTTON },
	/***
	Applet wakes up and is instructed to jump to HOME menu (?)
	@field APTCMD_WAKEUP_JUMPTOHOME
	*/
	{ "APTCMD_WAKEUP_JUMPTOHOME",  APTCMD_WAKEUP_JUMPTOHOME  },
	/***
	Request for sysapplet (?)
	@field APTCMD_SYSAPPLET_REQUEST
	*/
	{ "APTCMD_SYSAPPLET_REQUEST",  APTCMD_SYSAPPLET_REQUEST  },
	/***
	Applet wakes up and is instructed to launch another applet (?)
	@field APTCMD_WAKEUP_LAUNCHAPP
	*/
	{ "APTCMD_WAKEUP_LAUNCHAPP",   APTCMD_WAKEUP_LAUNCHAPP   },

	/***
	APT hook types constants
	@section
	*/
	/***
	App suspended
	@field APTHOOK_ONSUSPEND
	*/
	{ "APTHOOK_ONSUSPEND",         APTHOOK_ONSUSPEND         },
	/***
	App restored
	@field APTHOOK_ONRESTORE
	*/
	{ "APTHOOK_ONRESTORE",         APTHOOK_ONRESTORE         },
	/***
	App sleeping
	@field APTHOOK_ONSLEEP
	*/
	{ "APTHOOK_ONSLEEP",           APTHOOK_ONSLEEP           },
	/***
	App waking up
	@field APTHOOK_ONWAKEUP
	*/
	{ "APTHOOK_ONWAKEUP",          APTHOOK_ONWAKEUP          },
	/***
	App exiting
	@field APTHOOK_ONEXIT
	*/
	{ "APTHOOK_ONEXIT",            APTHOOK_ONEXIT            },
	/***
	Number of APT hook types
	@field APTHOOK_COUNT
	*/
	{ "APTHOOK_COUNT",             APTHOOK_COUNT             },
	{ NULL, 0 }
};

int luaopen_apt_lib(lua_State *L) {
	luaState = L;

	// Objects
	luaL_newmetatable(L, "LHook");
	lua_pushvalue(L, -1);
	lua_setfield(L, -2, "__index");
	luaL_setfuncs(L, hook_object_methods, 0);

	// Library
	luaL_newlib(L, apt_lib);

	for (int i = 0; apt_constants[i].name; i++) {
		lua_pushinteger(L, apt_constants[i].value);
		lua_setfield(L, -2, apt_constants[i].name);
	}

	return 1;
}

void load_apt_lib(lua_State *L) {
	aptInit();

	luaL_requiref(L, "ctr.apt", luaopen_apt_lib, false);
}

void unload_apt_lib(lua_State *L) {
	aptExit();
}
