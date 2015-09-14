#include <unistd.h>

#include <3ds/types.h>
#include <3ds/util/utf.h>
#include <3ds/services/fs.h>

#include <lua.h>
#include <lauxlib.h>

Handle *fsuHandle;
FS_archive sdmcArchive;

void load_lzlib(lua_State *L);

static int fs_list(lua_State *L) {
	const char *path = luaL_checkstring(L, 1);

	lua_newtable(L);
	int i = 1; // table index

	FS_path dirPath = FS_makePath(PATH_CHAR, path);

	Handle dirHandle;
	FSUSER_OpenDirectory(fsuHandle, &dirHandle, sdmcArchive, dirPath);

	u32 entriesRead = 0;
	do {
		FS_dirent buffer;

		FSDIR_Read(dirHandle, &entriesRead, 1, &buffer);

		if (!entriesRead) break;

		uint8_t name[256]; // utf8 file name
		size_t size = utf16_to_utf8(name, buffer.name, 0x106);
		*(name+size) = 0x0; // mark text end

		lua_createtable(L, 0, 8);

		lua_pushstring(L, (const char *)name);
		lua_setfield(L, -2, "name");
		lua_pushstring(L, (const char *)buffer.shortName);
		lua_setfield(L, -2, "shortName");
		lua_pushstring(L, (const char *)buffer.shortExt);
		lua_setfield(L, -2, "shortExt");
		lua_pushboolean(L, buffer.isDirectory);
		lua_setfield(L, -2, "isDirectory");
		lua_pushboolean(L, buffer.isHidden);
		lua_setfield(L, -2, "isHidden");
		lua_pushboolean(L, buffer.isArchive);
		lua_setfield(L, -2, "isArchive");
		lua_pushboolean(L, buffer.isReadOnly);
		lua_setfield(L, -2, "isReadOnly");
		lua_pushinteger(L, buffer.fileSize);
		lua_setfield(L, -2, "fileSize");

		lua_seti(L, -2, i);
		i++;

	} while (entriesRead > 0);

	FSDIR_Close(dirHandle);

	return 1;
}

static int fs_exists(lua_State *L) {
	const char *path = luaL_checkstring(L, 1);
	
	lua_pushboolean(L, access(path, F_OK) == 0);

	return 1;
}

static int fs_getDirectory(lua_State *L) {
	char cwd[256];

	lua_pushstring(L, getcwd(cwd, 256));

	return 1;
}

static int fs_setDirectory(lua_State *L) {
	const char *path = luaL_checkstring(L, 1);

	int result = chdir(path);

	if (result == 0)
		lua_pushboolean(L, true);
	else
		lua_pushboolean(L, false);

	return 1;
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
	{"zip", load_lzlib, NULL},
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
	fsInit();

	fsuHandle = fsGetSessionHandle();
	FSUSER_Initialize(fsuHandle);

	sdmcArchive = (FS_archive){ARCH_SDMC, FS_makePath(PATH_EMPTY, "")};
	FSUSER_OpenArchive(fsuHandle, &sdmcArchive);

	luaL_requiref(L, "ctr.fs", luaopen_fs_lib, false);
}

void unload_fs_lib(lua_State *L) {
	FSUSER_CloseArchive(fsuHandle, &sdmcArchive);

	fsExit();
}
