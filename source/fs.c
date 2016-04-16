/***
The `fs` module.
@module ctr.fs
@usage local fs = require("ctr.fs")
*/
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>
#include <dirent.h>

#include <3ds/types.h>
#include <3ds/util/utf.h>
#include <3ds/services/fs.h>
#include <3ds/sdmc.h>
#include <3ds/romfs.h>

#include <lua.h>
#include <lauxlib.h>

bool isFsInitialized = false;

/***
The `ctr.fs.lzlib` module.
@table lzlib
@see ctr.fs.lzlib
*/
void load_lzlib(lua_State *L);

/*
Automatically prefix the path if it's absolute (starts with /)
*/
const char* prefix_path(const char* path) {
	if (strncmp(path, "/", 1) == 0) {
		#ifdef ROMFS
		char* prefix = "romfs:";
		#else
		char* prefix = "sdmc:";
		#endif

		char out[1024];
		strcpy(out, prefix);
		return strcat(out, path);

	} else {
		return path;
	}
}

/***
Lists a directory contents (unsorted).
@function list
@tparam string path the directory we wants to list the content
@treturn table the item list. Each item is a table like:
`
	{
		name = "Item name.txt",
		isDirectory = false,
		fileSize = 321 -- (integer) in bytes
	}
`
*/
static int fs_list(lua_State *L) {
	const char* basepath = prefix_path(luaL_checkstring(L, 1));
	char* path;
	bool shouldFreePath = false;
	if (basepath[strlen(basepath)-1] != '/') {
		path = malloc(strlen(basepath)+2);
		strcpy(path, basepath);
		strcat(path, "/");
		shouldFreePath = true;
	} else {
		path = (char*)basepath;
	}

	lua_newtable(L);
	int i = 1; // table index
	
	DIR* dir = opendir(path);
	if (dir == NULL) {
		if (shouldFreePath) free(path);
		lua_pushboolean(L, false);
		lua_pushstring(L, strerror(errno));
		return 2;
	}
	errno = 0;
	struct dirent *entry;
	while (((entry = readdir(dir)) != NULL) && !errno) {
		lua_createtable(L, 0, 3);
		
		lua_pushstring(L, (const char*)entry->d_name);
		lua_setfield(L, -2, "name");
		lua_pushboolean(L, entry->d_type==DT_DIR);
		lua_setfield(L, -2, "isDirectory");
		
		if (entry->d_type==DT_REG) { // Regular files: check size
			char* filepath = malloc(strlen(path)+strlen(entry->d_name)+1);
			if (filepath == NULL) {
				luaL_error(L, "Memory allocation error");
			}
			memset(filepath, 0, strlen(path)+strlen(entry->d_name)+1);
			
			strcpy(filepath, path);
			strcat(filepath, entry->d_name);
			struct stat stats;
			if (stat(filepath, &stats)) {
				free(filepath);
				if (shouldFreePath) free(path);
				luaL_error(L, "Stat error: %s (%d)", strerror(errno), errno);
				lua_pushboolean(L, false);
				lua_pushstring(L, strerror(errno));
				return 2;
			} else {
				lua_pushinteger(L, stats.st_size);
			}
			free(filepath);
		} else { // Everything else: 0 bytes
			lua_pushinteger(L, 0);
		}
		lua_setfield(L, -2, "fileSize");

		lua_seti(L, -2, i);
		i++;
	}
	
	closedir(dir);
	if (shouldFreePath) free(path);

	return 1;
}

/***
Check if a item (file or directory) exists.
@function exists
@tparam string path the item
@treturn boolean true if it exists, false otherwise
*/
static int fs_exists(lua_State *L) {
	const char *path = prefix_path(luaL_checkstring(L, 1));
	
	lua_pushboolean(L, access(path, F_OK) == 0);

	return 1;
}

/***
Get the current working directory.
@function getDirectory
@treturn string the current working directory
*/
static int fs_getDirectory(lua_State *L) {
	char cwd[1024];

	lua_pushstring(L, getcwd(cwd, 1024));

	return 1;
}

/***
Set the current working directory.
@function setDirectory
@tparam path path of the new working directory
@treturn[1] boolean true if success
@treturn[2] boolean false if failed
@treturn[2] string error message
*/
static int fs_setDirectory(lua_State *L) {
	const char *path = prefix_path(luaL_checkstring(L, 1));

	int result = chdir(path);

	if (result == 0) {
		lua_pushboolean(L, true);
		return 1;

	} else {
		lua_pushboolean(L, false);
		lua_pushstring(L, strerror(errno));
		return 2;
	}
}

static const struct luaL_Reg fs_lib[] = {
	{ "list",         fs_list         },
	{ "exists",       fs_exists       },
	{ "getDirectory", fs_getDirectory },
	{ "setDirectory", fs_setDirectory },
	{ NULL, NULL }
};

// submodules
struct { char *name; void (*load)(lua_State *L); void (*unload)(lua_State *L); } fs_libs[] = {
	{"lzlib", load_lzlib, NULL},
	{NULL, NULL}
};

int luaopen_fs_lib(lua_State *L) {
	luaL_newlib(L, fs_lib);
	
	for (int i = 0; fs_libs[i].name; i++) {
		fs_libs[i].load(L);
		lua_setfield(L, -2, fs_libs[i].name);
	}
	
	return 1;
}

void load_fs_lib(lua_State *L) {
	if (!isFsInitialized) {
		sdmcInit();
		#ifdef ROMFS
		romfsInit();
		#endif
		isFsInitialized = true;
	}
	
	luaL_requiref(L, "ctr.fs", luaopen_fs_lib, false);
}

void unload_fs_lib(lua_State *L) {
	sdmcExit();
	#ifdef ROMFS
	romfsExit();
	#endif
}
