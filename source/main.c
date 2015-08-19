#include <3ds.h>
#include <sf2d.h>
#include <sftd.h>

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#define BOOT_FILE "/ctruLua/main.lua"

int load_ctr_lib(lua_State *L);
void unload_font_lib();

// Display an error
void error(const char *error) {
	gfxInitDefault();

	consoleInit(GFX_TOP, NULL);
	printf("------------------ FATAL ERROR -------------------");
	printf(error);
	printf("\n--------------------------------------------------");
	printf("Please exit ctruLua by rebooting the console.");

	while (aptMainLoop()) {
		gfxFlushBuffers();
		gfxSwapBuffers();
		gspWaitForVBlank();
	}
}

// Main loop
int main() {
	// Init GFX
	sf2d_init();
	sftd_init();
	//sf2d_set_3d(true);
	
	// Init accel/gyro
	HIDUSER_EnableAccelerometer();
	HIDUSER_EnableGyroscope();

	// Init Lua
	lua_State *L = luaL_newstate();
	if (L == NULL) error("Memory allocation error while creating a new Lua state");
	luaL_openlibs(L);
	load_ctr_lib(L);

	// Do the actual thing
	if (luaL_dofile(L, BOOT_FILE)) error(luaL_checkstring(L, -1));

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
