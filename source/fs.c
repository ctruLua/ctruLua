/***
The `fs` module.
@module ctr.fs
@usage local fs = require("ctr.fs")
*/
#include <unistd.h>
#include <errno.h>
#include <string.h>

#include <3ds/types.h>
#include <3ds/util/utf.h>
#include <3ds/services/fs.h>

#include <lua.h>
#include <lauxlib.h>

bool isFsInitialized = false;

Handle *fsuHandle;
FS_Archive sdmcArchive;
#ifdef ROMFS
FS_Archive romfsArchive;
#endif

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
		shortName = "ITEM~",
		shortExt = "TXT",
		isDirectory = false,
		isHidden = false,
		isArchive = false,
		isReadOnly = false,
		fileSize = 321 -- (integer) in bytes
	}
`
*/
static int fs_list(lua_State *L) {
	const char *path = prefix_path(luaL_checkstring(L, 1));

	lua_newtable(L);
	int i = 1; // table index

	// Get default archive
	#ifdef ROMFS
	FS_Archive archive = romfsArchive;
	#else
	FS_Archive archive = sdmcArchive;
	#endif
	// Archive path override (and skip path prefix)
	if (strncmp(path, "sdmc:", 5) == 0) {
		path += 5;
		archive = sdmcArchive;
	#ifdef ROMFS
	} else if (strncmp(path, "romfs:", 6) == 0) {
		path += 6;
		archive = romfsArchive;
	#endif
	}

	FS_Path dirPath = fsMakePath(PATH_ASCII, path);

	Handle dirHandle;
	FSUSER_OpenDirectory(&dirHandle, archive, dirPath);

	u32 entriesRead = 0;
	do {
		FS_DirectoryEntry buffer;

		FSDIR_Read(dirHandle, &entriesRead, 1, &buffer);

		if (!entriesRead) break;

		uint8_t name[0x106+1]; // utf8 file name
		size_t size = utf16_to_utf8(name, buffer.name, 0x106);
		*(name+size) = 0x0; // mark text end

		lua_createtable(L, 0, 8);

		lua_pushstring(L, (const char *)name);
		lua_setfield(L, -2, "name");
		lua_pushstring(L, (const char *)buffer.shortName);
		lua_setfield(L, -2, "shortName");
		lua_pushstring(L, (const char *)buffer.shortExt);
		lua_setfield(L, -2, "shortExt");
		lua_pushboolean(L, buffer.attributes&FS_ATTRIBUTE_DIRECTORY);
		lua_setfield(L, -2, "isDirectory");
		lua_pushboolean(L, buffer.attributes&FS_ATTRIBUTE_HIDDEN);
		lua_setfield(L, -2, "isHidden");
		lua_pushboolean(L, buffer.attributes&FS_ATTRIBUTE_ARCHIVE);
		lua_setfield(L, -2, "isArchive");
		lua_pushboolean(L, buffer.attributes&FS_ATTRIBUTE_READ_ONLY);
		lua_setfield(L, -2, "isReadOnly");
		lua_pushinteger(L, buffer.fileSize);
		lua_setfield(L, -2, "fileSize");

		lua_seti(L, -2, i);
		i++;

	} while (entriesRead > 0);

	FSDIR_Close(dirHandle);

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
		fsInit();

		fsuHandle = fsGetSessionHandle();
		FSUSER_Initialize(*fsuHandle);

		sdmcArchive = (FS_Archive){ARCHIVE_SDMC, fsMakePath(PATH_EMPTY, "")};
		FSUSER_OpenArchive(&sdmcArchive);
		#ifdef ROMFS
		romfsArchive = (FS_Archive){ARCHIVE_ROMFS, fsMakePath(PATH_EMPTY, "")};
		FSUSER_OpenArchive(&romfsArchive);
		#endif
		isFsInitialized = true;
	}
	
	luaL_requiref(L, "ctr.fs", luaopen_fs_lib, false);
}

void unload_fs_lib(lua_State *L) {
	FSUSER_CloseArchive(&sdmcArchive);
	#ifdef ROMFS
	FSUSER_CloseArchive(&romfsArchive);
	#endif

	fsExit();
}

