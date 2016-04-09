/***
The `news` module.
@module ctr.news
@usage local news = require("ctr.news")
*/
#include <3ds/types.h>
#include <3ds/util/utf.h>
#include <3ds/services/news.h>

#include <lua.h>
#include <lauxlib.h>

#include <stdlib.h>
#include <string.h>

bool initStateNews = false;

/***
Initialize the news module.
@function init
*/
static int news_init(lua_State *L) {
	if (!initStateNews) {
		newsInit();
		initStateNews = true;
	}
	return 0;
}

/***
Send a notification to the user. Should work now.
@function notification
@tparam string title title of the notification
@tparam[opt=nil] string message message of the notification, or nil for no message
@tparam[opt=nil] string imageData data from a JPEG image (content of a file) or raw data, or nil for no image
@tparam[opt=false] boolean jpeg set to `true` if the data is from a JPEG file
*/
static int news_notification(lua_State *L) {
	const char *title = luaL_checkstring(L, 1);
	const char *message = luaL_optstring(L, 2, NULL);
	
	u32 imageDataLength = 0;
	const void *imageData = luaL_optlstring(L, 3, NULL, (size_t*)&imageDataLength);
	bool jpeg = false;
	if (lua_isboolean(L, 4))
		jpeg = lua_toboolean(L, 4);
	
	const u16* cTitle = malloc(strlen(title)*sizeof(u16));
	const u16* cMessage = malloc(strlen(message)*sizeof(u16));
	u32 titleLength, messageLength;
	
	titleLength = (u32) utf8_to_utf16((uint16_t*)cTitle, (uint8_t*)title, strlen(title));
	if (message != NULL) {
		messageLength = (u32) utf8_to_utf16((uint16_t*)cMessage, (uint8_t*)message, strlen(message));
	} else {
		messageLength = 0;
		cMessage = NULL;
	}
	
	NEWS_AddNotification(cTitle, titleLength, cMessage, messageLength, imageData, imageDataLength, jpeg);
	
	return 0;
}

/***
Disable the news module.
@function shutdown
*/
static int news_shutdown(lua_State *L) {
	if (initStateNews) {
		newsExit();
		initStateNews = false;
	}
	return 0;
}

static const struct luaL_Reg news_lib[] = {
	{"init",         news_init        },
	{"notification", news_notification},
	{"shutdown",     news_shutdown    },
	{NULL, NULL}
};

int luaopen_news_lib(lua_State *L) {
	luaL_newlib(L, news_lib);
	return 1;
}

void load_news_lib(lua_State *L) {
	luaL_requiref(L, "ctr.news", luaopen_news_lib, 0);
}

void unload_news_lib(lua_State *L) {
	if (initStateNews) {
		newsExit();
		initStateNews = false;
	}
}
