/***
The `cfgu` module.
Used to get some user config.
@module ctr.cfgu
@usage local cfgu = require("ctr.cfgu")
*/

#include <stdlib.h>

#include <3ds/types.h>
#include <3ds/services/cfgu.h>
#include <3ds/util/utf.h>

#include <lua.h>
#include <lauxlib.h>

/***
Initialize the CFGU module.
@function init
*/
static int cfgu_init(lua_State *L) {
	cfguInit();
	
	return 0;
}

/***
Disable the CFGU module.
@function shutdown
*/
static int cfgu_shutdown(lua_State *L) {
	cfguExit();
	
	return 0;
}

/***
Return the console region.
@function getRegion
@treturn number region
*/
static int cfgu_getRegion(lua_State *L) {
	u8 region = 0;
	
	CFGU_SecureInfoGetRegion(&region);
	
	lua_pushinteger(L, region);
	return 1;
}

/***
Return the system model.
@function getModel
@treturn number model
*/
static int cfgu_getModel(lua_State *L) {
	u8 model = 0;
	
	CFGU_GetSystemModel(&model);
	
	lua_pushinteger(L, model);
	return 1;
}

/***
Return the system language.
@function getLanguage
@treturn number language
*/
static int cfgu_getLanguage(lua_State *L) {
	u8 language = 0;
	
	CFGU_GetSystemLanguage(&language);
	
	lua_pushinteger(L, language);
	return 1;
}

/***
Generate an unique hash from the console ID.
@function genHash
@tparam number salt 20-bits salt
@treturn number 64-bits hash  
*/
static int cfgu_genHash(lua_State *L) {
	u32 salt = luaL_optinteger(L, 1, 0);
	u64 hash = 0;
	
	CFGU_GenHashConsoleUnique(salt, &hash);
	
	lua_pushinteger(L, hash);
	
	return 1;
}

/***
Return the username.
@function getUsername
@treturn string username
*/
static int cfgu_getUsername(lua_State *L) {
	const u16 *block = malloc(0x1C);
	
	CFGU_GetConfigInfoBlk2(0x1C, 0xA0000, (u8*)block);
	u8 *name = malloc(0x14);
	utf16_to_utf8(name, block, 0x14);
	
	lua_pushlstring(L, (const char *)name, 0x14); // The username is only 0x14 characters long.
	return 1;
}

/***
Return the user birthday.
@function getBirthday
@treturn number month
@treturn number day
*/
static int cfgu_getBirthday(lua_State *L) {
	u16 tmp = 0;
	
	CFGU_GetConfigInfoBlk2(0x2, 0xA0001, (u8*)&tmp);
	
	u8 month = tmp/256;
	u8 day = tmp%256;
	
	lua_pushinteger(L, month);
	lua_pushinteger(L, day);
	return 2;
}

// Functions

static const struct luaL_Reg cfgu_lib[] = {
	{"init",          cfgu_init       },
	{"shutdown",      cfgu_shutdown   },
	{"getRegion",     cfgu_getRegion  },
	{"getModel",      cfgu_getModel   },
	{"getLanguage",   cfgu_getLanguage},
	{"genHash",       cfgu_genHash    },
	{"getUsername",   cfgu_getUsername},
	{"getBirthday",   cfgu_getBirthday},
	{NULL, NULL}
};

// Constants

struct { char *name; int value; } cfgu_constants[] = {
	/***
	Constant returned by `getRegion` if the console is from Japan.
	It is equal to `0`.
	@field REGION_JPN
	*/
	{"REGION_JPN", CFG_REGION_JPN},
	/***
	Constant returned by `getRegion` if the console is from USA.
	It is equal to `1`.
	@field REGION_USA
	*/
	{"REGION_USA", CFG_REGION_USA},
	/***
	Constant returned by `getRegion` if the console is from Europe.
	It is equal to `2`.
	@field REGION_EUR
	*/
	{"REGION_EUR", CFG_REGION_EUR},
	/***
	Constant returned by `getRegion` if the console is from Australia.
	It is equal to `3`.
	@field REGION_AUS
	*/
	{"REGION_AUS", CFG_REGION_AUS},
	/***
	Constant returned by `getRegion` if the console is from China.
	It is equal to `4`.
	@field REGION_CHN
	*/
	{"REGION_CHN", CFG_REGION_CHN},
	/***
	Constant returned by `getRegion` if the console is from Korea.
	It is equal to `5`.
	@field REGION_KOR
	*/
	{"REGION_KOR", CFG_REGION_KOR},
	/***
	Constant returned by `getRegion` if the console is from Taiwan.
	It is equal to `6`.
	@field REGION_TWN
	*/
	{"REGION_TWN", CFG_REGION_TWN},
	
	/***
	Constant returned by `getLanguage` if the language is Japanese.
	It is equal to `0`.
	@field LANGUAGE_JP
	*/
	{"LANGUAGE_JP", CFG_LANGUAGE_JP},
	/***
	Constant returned by `getLanguage` if the language is English.
	It is equal to `1`.
	@field LANGUAGE_EN
	*/
	{"LANGUAGE_EN", CFG_LANGUAGE_EN},
	/***
	Constant returned by `getLanguage` if the language is French.
	It is equal to `2`.
	@field LANGUAGE_FR
	*/
	{"LANGUAGE_FR", CFG_LANGUAGE_FR},
	/***
	Constant returned by `getLanguage` if the language is German.
	It is equal to `3`.
	@field LANGUAGE_DE
	*/
	{"LANGUAGE_DE", CFG_LANGUAGE_DE},
	/***
	Constant returned by `getLanguage` if the language is Italian.
	It is equal to `4`.
	@field LANGUAGE_JP
	*/
	{"LANGUAGE_IT", CFG_LANGUAGE_IT},
	/***
	Constant returned by `getLanguage` if the language is Spanish.
	It is equal to `5`.
	@field LANGUAGE_ES
	*/
	{"LANGUAGE_ES", CFG_LANGUAGE_ES},
	/***
	Constant returned by `getLanguage` if the language is Chinese.
	It is equal to `6`.
	@field LANGUAGE_ZH
	*/
	{"LANGUAGE_ZH", CFG_LANGUAGE_ZH},
	/***
	Constant returned by `getLanguage` if the language is Korean.
	It is equal to `7`.
	@field LANGUAGE_KO
	*/
	{"LANGUAGE_KO", CFG_LANGUAGE_KO},
	/***
	Constant returned by `getLanguage` if the language is Dutch.
	It is equal to `8`.
	@field LANGUAGE_NL
	*/
	{"LANGUAGE_NL", CFG_LANGUAGE_NL},
	/***
	Constant returned by `getLanguage` if the language is Portuguese.
	It is equal to `9`.
	@field LANGUAGE_PT
	*/
	{"LANGUAGE_PT", CFG_LANGUAGE_PT},
	/***
	Constant returned by `getLanguage` if the language is Russian.
	It is equal to `10`.
	@field LANGUAGE_RU
	*/
	{"LANGUAGE_RU", CFG_LANGUAGE_RU},
	/***
	Constant returned by `getLanguage` if the language is Taiwanese.
	It is equal to `11`.
	@field LANGUAGE_TW
	*/
	{"LANGUAGE_TW", CFG_LANGUAGE_TW},
	
	/***
	Constant returned by `getModel` if the console is a 3DS.
	It is equal to `0`.
	@field MODEL_3DS
	*/
	{"MODEL_3DS",    0},
	/***
	Constant returned by `getModel` if the console is a 3DS XL.
	It is equal to `1`.
	@field MODEL_3DSXL
	*/
	{"MODEL_3DSXL",  1},
	/***
	Constant returned by `getModel` if the console is a New 3DS.
	It is equal to `2`.
	@field MODEL_3DSXL
	*/
	{"MODEL_N3DS",   2},
	/***
	Constant returned by `getModel` if the console is a 2DS.
	It is equal to `3`.
	@field MODEL_2DS
	*/
	{"MODEL_2DS",    3},
	/***
	Constant returned by `getModel` if the console is a New 3DS XL.
	It is equal to `4`.
	@field MODEL_N3DSXL
	*/
	{"MODEL_N3DSXL", 4},
	
	{NULL, 0}
};

int luaopen_cfgu_lib(lua_State *L) {
	luaL_newlib(L, cfgu_lib);
	
	for (int i = 0; cfgu_constants[i].name; i++) {
		lua_pushinteger(L, cfgu_constants[i].value);
		lua_setfield(L, -2, cfgu_constants[i].name);
	}

	return 1;
}

void load_cfgu_lib(lua_State *L) {
	luaL_requiref(L, "ctr.cfgu", luaopen_cfgu_lib, false);
}
