#include <3ds.h>
#include <sf2d.h>
#include <sftd.h>

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#define BOOT_FILE "/ctruLua/main.lua"

int load_ctr_lib(lua_State *L);

// Display an error
void error(char *error) {
	gfxInitDefault();

	consoleInit(GFX_TOP, NULL);
	printf("------------------ FATAL ERROR -------------------");
	printf(error);
	printf("\n--------------------------------------------------");
	printf("Please exit ctruLua by pressing the home button.");

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
	if (L == NULL) error("memory allocation error while creating a new Lua state");
	luaL_openlibs(L);
	load_ctr_lib(L);

	// Do the actual thing
	if(luaL_dofile(L, BOOT_FILE)) error("Can open "BOOT_FILE);

	// Un-init (?)
	sftd_fini();
	sf2d_fini();
	// Disable needed things
	HIDUSER_DisableAccelerometer();
	HIDUSER_DisableGyroscope();
	return 0;
}
