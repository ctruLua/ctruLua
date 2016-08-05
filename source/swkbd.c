/***
The `swkbd` module.
@module ctr.swkbd
@usage local swkbd = require("ctr.swkbd")
*/

#include <3ds/types.h>
#include <3ds/applets/swkbd.h>

#include <lapi.h>
#include <lauxlib.h>

#include <stdlib.h>
#include <malloc.h>

// functions

/***
Create a software keyboard.
@function keyboard
@tparam[opt=TYPE_NORMAL] TYPE type keyboard type.
@tparam[opt=2] integer buttons number of buttons, can be 1, 2 or 3. Will default to 2 if the number isn't 1, 2 or 3
@tparam[opt=max] integer maxLength maximum length of the text. (maximum 65000)
@treturn keyboard a software keyboard object
*/
static int swkbd_keyboard(lua_State *L) {
	SwkbdType type = luaL_optinteger(L, 1, SWKBD_TYPE_NORMAL);
	int buttons = luaL_optinteger(L, 2, 2);
	int maxLength = luaL_optinteger(L, 3, -1);
	
	SwkbdState *keyboard = lua_newuserdata(L, sizeof(*keyboard));
	luaL_getmetatable(L, "LKeyboard");
	lua_setmetatable(L, -2);
	
	swkbdInit(keyboard, type, buttons, maxLength);
	
	return 1;
}

// methods

/***
Keyboard object
@section object
*/

/***
Launch the keyboard applet.
@function :launch
@tparam[opt=max] integer maxLength maximum text length (max 65000)
@treturn[1] string text (UTF-8)
@treturn[1] integer button pressed (1, 2 or 3)
@treturn[2] nil
@treturn[2] string error. In case of an "OUTOFMEM" error, this function will throw an error instead of returning.
*/
static int keyboard_launch(lua_State *L) {
	SwkbdState *keyboard = luaL_checkudata(L, 1, "LKeyboard");
	int length = luaL_optinteger(L, 2, 65000);
	
	char* buff = malloc(length);
	
	SwkbdButton button = swkbdInputText(keyboard, buff, length);
	
	if (button<0) {
		free(buff);
		lua_pushnil(L);
		switch (button) {
			case SWKBD_NONE:
				lua_pushstring(L, "That wasn't supposed to happen.");
				break;
			
			case SWKBD_INVALID_INPUT:
				lua_pushstring(L, "Invalid parameters.");
				break;
			
			case SWKBD_OUTOFMEM:
				luaL_error(L, "Out of memory.");
				break;
			
			default:
				lua_pushfstring(L, "Unexpected error: %d.", button);
				break;
		}
		
		return 2;
	}
	lua_pushstring(L, buff);
	free(buff);
	lua_pushinteger(L, button);
	
	return 2;
}

/***
Set the initial text in the textbox.
@function :setInitialText
@tparam[opt=""] string text initial text
*/
static int keyboard_setInitialText(lua_State *L) {
	SwkbdState *keyboard = luaL_checkudata(L, 1, "LKeyboard");
	const char* text = luaL_optstring(L, 2, "");
	
	swkbdSetInitialText(keyboard, text);
	
	return 0;
}

/***
Set the text displayed in the textbox when it's empty.
@function :setHintText
@tparam[opt=""] string text hint text
*/
static int keyboard_setHintText(lua_State *L) {
	SwkbdState *keyboard = luaL_checkudata(L, 1, "LKeyboard");
	const char* text = luaL_optstring(L, 2, "");
	
	swkbdSetHintText(keyboard, text);
	
	return 0;
}

////////////////////

static const struct luaL_Reg swkbd_lib[] = {
	{"keyboard", swkbd_keyboard},
	{NULL, NULL}
};

static const struct luaL_Reg swkbd_object_methods[] = {
	{"launch",         keyboard_launch},
	{"setInitialText", keyboard_setInitialText},
	{"setHintText",    keyboard_setHintText},
	{NULL, NULL}
};

/***
Constants
@section constants
*/

struct { char *name; int value; } swkbd_constants[] = {
	/***
	@field TYPE_NORMAL
	*/
	{"TYPE_NORMAL", SWKBD_TYPE_NORMAL},
	/***
	@field TYPE_QWERTY
	*/
	{"TYPE_QWERTY", SWKBD_TYPE_QWERTY},
	/***
	@field TYPE_NUMPAD
	*/
	{"TYPE_NUMPAD", SWKBD_TYPE_NUMPAD},
	/***
	@field TYPE_WESTERN
	*/
	{"TYPE_WESTERN", SWKBD_TYPE_WESTERN},
	/***
	@field ANYTHING
	*/
	{"ANYTHING", SWKBD_ANYTHING},
	/***
	@field NOTEMPTY
	*/
	{"NOTEMPTY", SWKBD_NOTEMPTY},
	/***
	@field NOTEMPTY_NOTBLANK
	*/
	{"NOTEMPTY_NOTBLANK", SWKBD_NOTEMPTY_NOTBLANK},
	/***
	@field NOTBLANK_NOTEMPTY
	*/
	{"NOTBLANK_NOTEMPTY", SWKBD_NOTBLANK_NOTEMPTY},
	/***
	@field NOTBLANK
	*/
	{"NOTBLANK", SWKBD_NOTBLANK},
	/***
	@field FIXEDLEN
	*/
	{"FIXEDLEN", SWKBD_FIXEDLEN},
	
	{NULL, 0}
};

int luaopen_swkbd_lib(lua_State *L) {
	luaL_newmetatable(L, "LKeyboard");
	lua_pushvalue(L, -1);
	lua_setfield(L, -2, "__index");
	luaL_setfuncs(L, swkbd_object_methods, 0);
	
	luaL_newlib(L, swkbd_lib);
	
	for (int i = 0; swkbd_constants[i].name; i++) {
		lua_pushinteger(L, swkbd_constants[i].value);
		lua_setfield(L, -2, swkbd_constants[i].name);
	}
	
	return 1;
}

void load_swkbd_lib(lua_State *L) {
	luaL_requiref(L, "ctr.swkbd", luaopen_swkbd_lib, false);
}
