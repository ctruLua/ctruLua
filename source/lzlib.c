/***
The `fs.lzlib` module. See https://github.com/LuaDist/lzlib for informations and documentation.
@module ctr.fs.lzlib
@usage local lzlib = require("ctr.fs.lzlib")
*/

#include <3ds/types.h>

#include <lua.h>
#include <lauxlib.h>

#include <lzlib.c>

void load_lzlib(lua_State *L) {
	luaL_requiref(L, "ctr.fs.lzlib", luaopen_zlib, false);
}
