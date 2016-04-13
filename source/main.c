#include <unistd.h>

#include <3ds.h>

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

void load_ctr_lib(lua_State *L);
void unload_ctr_lib(lua_State *L);

bool isGfxInitialized;

// Display an error
void error(const char *error) {
	if (!isGfxInitialized) gfxInitDefault();
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

	if (!isGfxInitialized) gfxExit();
}

// Main loop
int main(int argc, char** argv) {
	// Default arguments
	#ifdef ROMFS
	char* mainFile = "romfs:/main.lua";
	#else
	char* mainFile = "main.lua";
	#endif
	
	// Init Lua
	lua_State *L = luaL_newstate();
	if (L == NULL) {
		error("Memory allocation error while creating a new Lua state");
		return 0;
	}

	// Load libs
	luaL_openlibs(L);
	load_ctr_lib(L);
	isGfxInitialized = true;
	
	// Parse arguments
	for (int i=0;i<argc;i++) {
		if (argv[i][0] == '-') {
			switch(argv[i][1]) {
				case 'm': { // main file replacement
					mainFile = &argv[i][2];
					if (argv[i][2] == ' ') mainFile = &argv[i][3];
					break;
				}
				
				case 'r': { // root directory replacement
					char* root;
					root = &argv[i][2];
					if (argv[i][2] == ' ') root = &argv[i][3];
					if (chdir(root)) error("No such root path");
					break;
				}
					
			}
		}
	}
	
	// Do the actual thing
	if (luaL_dofile(L, mainFile)) error(luaL_checkstring(L, -1));

	// Unload libs
	unload_ctr_lib(L);
	
	// Unload Lua
	lua_close(L);
	
	return 0;
}
