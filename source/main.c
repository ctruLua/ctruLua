#include <3ds.h>

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#define BOOT_FILE "sdmc:/3ds/ctruLua/main.lua"

void load_ctr_lib(lua_State *L);
void unload_ctr_lib(lua_State *L);

bool isGfxInitialised;

// Display an error
void error(const char *error) {
	if (!isGfxInitialised) gfxInitDefault();
	gfxSet3D(false);

	consoleInit(GFX_TOP, NULL);
	printf("------------------ FATAL ERROR -------------------");
	printf(error);
	printf("\n--------------------------------------------------");
	printf("Please exit ctruLua by pressing start.");
	
	while (aptMainLoop()) {
		hidScanInput();
		if (hidKeysDown() & KEY_START) break;
		gfxFlushBuffers();
		gfxSwapBuffers();
		gspWaitForVBlank();
	}

	if (!isGfxInitialised) gfxExit();
}

// Main loop
int main() {
	// Init Lua
	lua_State *L = luaL_newstate();
	if (L == NULL) {
		error("Memory allocation error while creating a new Lua state");
		return 0;
	}

	// Load libs
	luaL_openlibs(L);
	load_ctr_lib(L);

	// Do the actual thing
	if (luaL_dofile(L, BOOT_FILE)) error(luaL_checkstring(L, -1));

	// Unload libs
	unload_ctr_lib(L);

	// Unload Lua
	lua_close(L);
	
	return 0;
}
