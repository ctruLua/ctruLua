#include <3ds/types.h>
#include <3ds/util/utf.h>
#include <3ds/services/news.h>

#include <lua.h>
#include <lauxlib.h>

static int news_init(lua_State *L) {
	newsInit();
		
	return 0;
}

static int news_notification(lua_State *L) {
	const char *title = luaL_checkstring(L, 1);
	const char *message = luaL_checkstring(L, 2);
	const void *imageData = luaL_checkstring(L, 3);
	bool jpeg = false;
	if (lua_isboolean(L, 4))
		jpeg = lua_toboolean(L, 4);
  
  const u16* cTitle = 0;
	const u16* cMessage = 0;
	u32 titleLength, messageLength, imageDataLength = 0;
	
	titleLength = (u32) utf8_to_utf16((uint16_t*)cTitle, (uint8_t*)title, sizeof(title));
	messageLength = (u32) utf8_to_utf16((uint16_t*)cMessage, (uint8_t*)message, sizeof(message));
	
	NEWSU_AddNotification(cTitle, titleLength, cMessage, messageLength, imageData, imageDataLength, jpeg);
	
	return 0;
}

static int news_shutdown(lua_State *L) {
	newsExit();
	
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
