#include <3ds.h>
#include <sf2d.h>
#include <sftd.h>

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#define BOOT_FILE "sdmc:/3ds/ctruLua/main.lua"

int load_ctr_lib(lua_State *L);
void unload_font_lib();

bool gfxinit = false;

// Display an error
void error(const char *error) {
	if (!gfxinit) gfxInitDefault();
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

	if (!gfxinit) gfxExit();
}

// Main loop
int main() {
	// Init Lua
	lua_State *L = luaL_newstate();
	if (L == NULL) {
		error("Memory allocation error while creating a new Lua state");
		return 0;
	}
	
	// Init GFX
	sf2d_init();
	sftd_init();

	gfxinit = true;
	
	// Init accel/gyro
	HIDUSER_EnableAccelerometer();
	HIDUSER_EnableGyroscope();

	// Load libs
	luaL_openlibs(L);
	load_ctr_lib(L);

	// Do the actual thing
	if (luaL_dofile(L, BOOT_FILE)) error(luaL_checkstring(L, -1));

	// Unload Lua
	lua_close(L);

	// Unload current font
	unload_font_lib();
	
	// Disable accel/gyro
	HIDUSER_DisableAccelerometer();
	HIDUSER_DisableGyroscope();
	
	// Uninit GFX
	sftd_fini();
	sf2d_fini();
	
	return 0;
}
