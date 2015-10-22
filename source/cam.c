/***
The `cam` module.
@module ctr.cam
@usage local cam = require("ctr.cam")
*/

#include <3ds.h>
#include <3ds/types.h>
#include <3ds/svc.h>
#include <3ds/services/cam.h>

#include <sf2d.h>

#include <lua.h>
#include <lauxlib.h>

#include <malloc.h>

#include "texture.h"

/***
Initialize the camera module.
@function init
*/
static int cam_init(lua_State *L) {
	Result ret = camInit();
	if (ret) {
		lua_pushboolean(L, false);
		lua_pushinteger(L, ret);
		return 2;
	}
	
	lua_pushboolean(L, true);
	return 1;
}

/***
Disable the camera module.
@function shutdown
*/
static int cam_shutdown(lua_State *L) {
  camExit();
  
  return 0;
}

/***
Activate a camera.
@function activate
@tparam number camera camera to activate (`SELECT_x`)
*/
static int cam_activate(lua_State *L) {
	u8 cam = luaL_checkinteger(L, 1);
	
	CAMU_Activate(cam);
	
	return 0;
}

/***
Set the exposure of a camera.
@function setExposure
@tparam number camera (`SELECT_x`)
@tparam number exposure (from `-128` to `127`)
*/
static int cam_setExposure(lua_State *L) {
	u8 cam = luaL_checkinteger(L, 1);
	s8 expo = luaL_checkinteger(L, 2);
	
	CAMU_SetExposure(cam, expo);
	
	return 0;
}

/***
Set the white balance of a camera.
@function setWhiteBalance
@tparam number camera (`SELECT_x`)
@tparam number white white balance (`WHITE_BALANCE_x`)
*/
static int cam_setWhiteBalance(lua_State *L) {
	u8 cam = luaL_checkinteger(L, 1);
	u8 bal = luaL_checkinteger(L, 2);
	
	CAMU_SetWhiteBalance(cam, bal);
	
	return 0;
}

/***
Set the sharpness of a camera.
@function setSharpness
@tparam number camera (`SELECT_x`)
@tparam number sharpness from (from `-128` to `127`)
*/
static int cam_setSharpness(lua_State *L) {
	u8 cam = luaL_checkinteger(L, 1);
	s8 sharp = luaL_checkinteger(L, 2);
	
	CAMU_SetSharpness(cam, sharp);
	
	return 0;
}

/***
Set the auto exposure mode of a camera.
@function setAutoExposure
@tparam number camera (`SELECT_x`)
@tparam boolean auto `true` to enable, `false` to disable
*/
static int cam_setAutoExposure(lua_State *L) {
	u8 cam = luaL_checkinteger(L, 1);
	bool expo = lua_toboolean(L, 2);
	
	CAMU_SetAutoExposure(cam, expo);
	
	return 0;
}

/***
Check if the auto exposure mode is enabled for a camera.
@function isAutoExposure
@tparam number camera (`SELECT_x`)
@treturn boolean `true` if enabled, `false` if not
*/
static int cam_isAutoExposure(lua_State *L) {
	u8 cam = luaL_checkinteger(L, 1);
	bool expo = false;
	
	CAMU_IsAutoExposure(&expo, cam);
	
	lua_pushboolean(L, expo);
	return 1;
}

/***
Set the auto white balance mode for a camera.
@function setAutoWhiteBalance
@tparam number camera (`SELECT_x`)
@tparam boolean auto `true` to enable, `false` to disable
*/
static int cam_setAutoWhiteBalance(lua_State *L) {
	u8 cam = luaL_checkinteger(L, 1);
	bool bal = lua_toboolean(L, 2);
	
	CAMU_SetAutoWhiteBalance(cam, bal);
	
	return 0;
}

/***
Check if the auto white balance mode is enabled for a camera.
@function isAutoWhiteBalance
@tparam number camera (`SELECT_x`)
@treturn boolean `true` if enabled, `false` if not
*/
static int cam_isAutoWhiteBalance(lua_State *L) {
	u8 cam = luaL_checkinteger(L, 1);
	bool bal = false;
	
	CAMU_IsAutoWhiteBalance(&bal, cam);
	
	lua_pushboolean(L, bal);
	return 1;
}

/***
Set the contrast on a camera.
@function setContrast
@tparam number camera (`SELECT_x`)
@tparam number contrast (`CONTRAST_x`)
*/
static int cam_setContrast(lua_State *L) {
	u8 cam = luaL_checkinteger(L, 1);
	u8 cont = luaL_checkinteger(L, 2);
	
	CAMU_SetContrast(cam, cont);
	
	return 0;
}

/***
Set the lens correction of a camera.
@function setLensCorrection
@tparam number camera (`SELECT_x`)
@tparam number correction lens correction (`LENS_CORRECTION_x`)
*/
static int cam_setLensCorrection(lua_State *L) {
	u8 cam = luaL_checkinteger(L, 1);
	u8 corr = luaL_checkinteger(L, 2);
	
	CAMU_SetLensCorrection(cam, corr);
	
	return 0;
}

/***
Set the window where the exposure will be check.
@function setAutoExposureWindow
@tparam number x x starting position of the window
@tparam number y y starting position of the window
@tparam number w width of the window
@tparam number h height of the window
*/
static int cam_setAutoExposureWindow(lua_State *L) {
	u8 cam = luaL_checkinteger(L, 1);
	s16 x = luaL_checkinteger(L, 2);
	s16 y = luaL_checkinteger(L, 3);
	s16 w = luaL_checkinteger(L, 4);
	s16 h = luaL_checkinteger(L, 5);
	
	CAMU_SetAutoExposureWindow(cam, x, y, w, h);
	
	return 0;
}

/***
Set the window where the white balance will be check.
@function setAutoWhiteBalanceWindow
@tparam number x x starting position of the window
@tparam number y y starting position of the window
@tparam number w width of the window
@tparam number h height of the window
*/
static int cam_setAutoWhiteBalanceWindow(lua_State *L) {
	u8 cam = luaL_checkinteger(L, 1);
	s16 x = luaL_checkinteger(L, 2);
	s16 y = luaL_checkinteger(L, 3);
	s16 w = luaL_checkinteger(L, 4);
	s16 h = luaL_checkinteger(L, 5);
	
	CAMU_SetAutoWhiteBalanceWindow(cam, x, y, w, h);
	
	return 0;
}

/***
Enable or disable the noise filter on a camera.
@function setNoiseFilter
@tparam number camera (`SELECT_x`)
@tparam boolean filter `true` to enable, `false` to disable
*/
static int cam_setNoiseFilter(lua_State *L) {
	u8 cam = luaL_checkinteger(L, 1);
	bool fil = lua_toboolean(L, 2);
	
	CAMU_SetNoiseFilter(cam, fil);
	
	return 0;
}

/***
Play a shutter sound.
@function playShutterSound
@tparam number sound shutter sound type (`SHUTTER_x`)
*/
static int cam_playShutterSound(lua_State *L) {
	u8 shut = luaL_checkinteger(L, 1);
	
	CAMU_PlayShutterSound(shut);
	
	return 0;
}

/***
Set the size of the base camera image.
@function setSize
@tparam number camera (`SELECT_x`)
@tparam number size (`SIZE_x`)
@tparam number context (`CONTEXT_x`)
*/
static int cam_setSize(lua_State *L) {
	u8 cam = luaL_checkinteger(L, 1);
	u8 size = luaL_checkinteger(L, 2);
	u8 context = luaL_checkinteger(L, 3);
	
	CAMU_SetSize(cam, size, context);
	
	return 0;
}

/***
Set the effect applied on the image.
@function setEffect
@tparam number camera (`SELECT_x`)
@tparam number effect (`EFFECT_x`)
@tparam number context (`CONTEXT_x`)
*/
static int cam_setEffect(lua_State *L) {
	u8 cam = luaL_checkinteger(L, 1);
	u8 effect = luaL_checkinteger(L, 2);
	u8 context = luaL_checkinteger(L, 3);
	
	CAMU_SetEffect(cam, effect, context);
	
	return 0;
}

/***
Take a picture and put it in a texture.
@function takePicture
@tparam number camera should be PORT_CAM1 if you have only 1 camera activated (`PORT_x`)
@tparam number w width of the picture
@tparam number h height of the picture
@tparam[opt=PLACE_RAM] number place where to put the texture
@treturn texture the texture object
*/
static int cam_takePicture(lua_State *L) {
	u8 cam = luaL_checkinteger(L, 1);
	s16 w = luaL_optinteger(L, 2, 640);
	s16 h = luaL_optinteger(L, 3, 480);
	u32 bufSize = 0;
	
	// Take the actual picture
	CAMU_GetMaxBytes(&bufSize, w, h);
	u8* buf = malloc(bufSize);
	CAMU_SetTransferBytes(cam, bufSize, w, h);
	
	Handle camReceiveEvent = 0;
	
	CAMU_ClearBuffer(cam);
	CAMU_SetReceiving(&camReceiveEvent, buf, cam, bufSize, (s16)bufSize);
	CAMU_StartCapture(cam);
	svcWaitSynchronization(camReceiveEvent, 300000000ULL);
	CAMU_SetReceiving(&camReceiveEvent, buf, cam, bufSize, (s16)bufSize); // I don't know why you have to put it twice ...
	svcWaitSynchronization(camReceiveEvent, 300000000ULL);
	CAMU_StopCapture(cam);
	
	// Load it in a texture
	u8 place = luaL_optinteger(L, 4, SF2D_PLACE_RAM);
	
	texture_userdata *texture;
	texture = (texture_userdata *)lua_newuserdata(L, sizeof(*texture));
	
	luaL_getmetatable(L, "LTexture");
	lua_setmetatable(L, -2);
	
	texture->texture = sf2d_create_texture_mem_RGBA8(buf, w, h, TEXFMT_RGB565, place);
	sf2d_texture_tile32(texture->texture);
	
	texture->scaleX = 1.0f;
	texture->scaleY = 1.0f;
	texture->blendColor = 0xffffffff;
	
	return 1;
}

// Functions
static const struct luaL_Reg cam_lib[] = {
	{"init",                      cam_init                     },
	{"shutdown",                  cam_shutdown                 },
	{"activate",                  cam_activate                 },
	{"setExposure",               cam_setExposure              },
	{"setWhiteBalance",           cam_setWhiteBalance          },
	{"setSharpness",              cam_setSharpness             },
	{"setAutoExposure",           cam_setAutoExposure          },
	{"isAutoExposure",            cam_isAutoExposure           },
	{"setAutoWhiteBalance",       cam_setAutoWhiteBalance      },
	{"isAutoWhiteBalance",        cam_isAutoWhiteBalance       },
	{"setContrast",               cam_setContrast              },
	{"setLensCorrection",         cam_setLensCorrection        },
	{"setAutoExposureWindow",     cam_setAutoExposureWindow    },
	{"setAutoWhiteBalanceWindow", cam_setAutoWhiteBalanceWindow},
	{"setNoiseFilter",            cam_setNoiseFilter           },
	{"playShutterSound",          cam_playShutterSound         },
	{"setSize",                   cam_setSize                  },
	{"setEffect",                 cam_setEffect                },
	{"takePicture",               cam_takePicture              },
	{NULL, NULL}
};

// Constants. Warning: long block
struct { char *name; int value; } cam_constants[] = {
	// port
	/***
	First camera activated.
	@field PORT_CAM1
	*/
	{"PORT_CAM1", PORT_CAM1},
	/***
	Second camera activated.
	@field PORT_CAM2
	*/
	{"PORT_CAM2", PORT_CAM2},
	/***
	The two activated camera. Should not be used ATM.
	@field PORT_BOTH
	*/
	{"PORT_BOTH", PORT_BOTH},
	// camera selection
	/***
	@field SELECT_NONE
	*/
	{"SELECT_NONE",      SELECT_NONE     },
	/***
	@field SELECT_OUT1
	*/
	{"SELECT_OUT1",      SELECT_OUT1     },
	/***
	@field SELECT_IN1
	*/
	{"SELECT_IN1",       SELECT_IN1      },
	/***
	@field SELECT_OUT2
	*/
	{"SELECT_OUT2",      SELECT_OUT2     },
	/***
	@field SELECT_IN1_OUT1
	*/
	{"SELECT_IN1_OUT1",  SELECT_IN1_OUT1 },
	/***
	@field SELECT_OUT1_OUT2
	*/
	{"SELECT_OUT1_OUT2", SELECT_OUT1_OUT2},
	/***
	@field SELECT_IN1_OUT2
	*/
	{"SELECT_IN1_OUT2",  SELECT_IN1_OUT2 },
	/***
	@field SELECT_ALL
	*/
	{"SELECT_ALL",       SELECT_ALL      },
	// context
	/***
	@field CONTEXT_NONE
	*/
	{"CONTEXT_NONE", CONTEXT_NONE},
	/***
	@field CONTEXT_A
	*/
	{"CONTEXT_A",    CONTEXT_A   },
	/***
	@field CONTEXT_B
	*/
	{"CONTEXT_B",    CONTEXT_B   },
	/***
	@field CONTEXT_BOTH
	*/
	{"CONTEXT_BOTH", CONTEXT_BOTH},
	// image flip
	/***
	@field FLIP_NONE
	*/
	{"FLIP_NONE",       FLIP_NONE      },
	/***
	@field FLIP_HORIZONTAL
	*/
	{"FLIP_HORIZONTAL", FLIP_HORIZONTAL},
	/***
	@field FLIP_VERTICAL
	*/
	{"FLIP_VERTICAL",   FLIP_VERTICAL  },
	/***
	@field FLIP_REVERSE
	*/
	{"FLIP_REVERSE",    FLIP_REVERSE   },
	// image size
	/***
	640x480
	@field SIZE_VGA
	*/
	{"SIZE_VGA",            SIZE_VGA           },
	/***
	320x240
	@field SIZE_QVGA
	*/
	{"SIZE_QVGA",           SIZE_QVGA          },
	/***
	160x120
	@field SIZE_QQVGA
	*/
	{"SIZE_QQVGA",          SIZE_QQVGA         },
	/***
	352x288
	@field SIZE_CIF
	*/
	{"SIZE_CIF",            SIZE_CIF           },
	/***
	176x144
	@field SIZE_QCIF
	*/
	{"SIZE_QCIF",           SIZE_QCIF          },
	/***
	256x192
	@field SIZE_DS_LCD
	*/
	{"SIZE_DS_LCD",         SIZE_DS_LCD        },
	/***
	512x384
	@field SIZE_DS_LCDx4
	*/
	{"SIZE_DS_LCDx4",       SIZE_DS_LCDx4      },
	/***
	400x240
	@field SIZE_CTR_TOP_LCD
	*/
	{"SIZE_CTR_TOP_LCD",    SIZE_CTR_TOP_LCD   },
	/***
	320x240
	@field SIZE_BOTTOM_LCD
	*/
	{"SIZE_CTR_BOTTOM_LCD", SIZE_CTR_BOTTOM_LCD},
	// frame rate
	{"FRAME_RATE_15",       FRAME_RATE_15      },
	{"FRAME_RATE_15_TO_5",  FRAME_RATE_15_TO_5 },
	{"FRAME_RATE_15_To_2",  FRAME_RATE_15_TO_2 },
	{"FRAME_RATE_10",       FRAME_RATE_10      },
	{"FRAME_RATE_8_5",      FRAME_RATE_8_5     },
	{"FRAME_RATE_5",        FRAME_RATE_5       },
	{"FRAME_RATE_20",       FRAME_RATE_20      },
	{"FRAME_RATE_20_TO_5",  FRAME_RATE_20_TO_5 },
	{"FRAME_RATE_30",       FRAME_RATE_30      },
	{"FRAME_RATE_30_TO_5",  FRAME_RATE_30_TO_5 },
	{"FRAME_RATE_15_TO_10", FRAME_RATE_15_TO_10},
	{"FRAME_RATE_20_TO_10", FRAME_RATE_20_TO_10},
	{"FRAME_RATE_30_TO_10", FRAME_RATE_30_TO_10},
	// white balance
	/***
	@field WHITe_BALANCE_AUTO
	*/
	{"WHITE_BALANCE_AUTO",  WHITE_BALANCE_AUTO },
	/***
	@field WHITE_BALANCE_3200K
	*/
	{"WHITE_BALANCE_3200K", WHITE_BALANCE_3200K},
	/***
	@field WHITE_BALANCE_4150K
	*/
	{"WHITE_BALANCE_4150K", WHITE_BALANCE_4150K},
	/***
	@field WHITE_BALANCE_5200K
	*/
	{"WHITE_BALANCE_5200K", WHITE_BALANCE_5200K},
	/***
	@field WHITE_BALANCE_6000K
	*/
	{"WHITE_BALANCE_6000K", WHITE_BALANCE_6000K},
	/***
	@field WHITE_BALANCE_7000K
	*/
	{"WHITE_BALANCE_7000K", WHITE_BALANCE_7000K},
	// white balance aliases
	/***
	@field WHITE_BALANCE_MAX
	*/
	{"WHITE_BALANCE_MAX",                     WHITE_BALANCE_MAX                    },
	/***
	@field WHITE_BALANCE_TUNGSTEN
	*/
	{"WHITE_BALANCE_TUNGSTEN",                WHITE_BALANCE_TUNGSTEN               },
	/***
	@field WHITE_BALANCE_WHITE_FLUORESCENT_LIGHT
	*/
	{"WHITE_BALANCE_WHITE_FLUORESCENT_LIGHT", WHITE_BALANCE_WHITE_FLUORESCENT_LIGHT},
	/***
	@field WHITE_BALANCE_DAYLIGHT
	*/
	{"WHITE_BALANCE_DAYLIGHT",                WHITE_BALANCE_DAYLIGHT               },
	/***
	@field WHITE_BALANCE_CLOUDY
	*/
	{"WHITE_BALANCE_CLOUDY",                  WHITE_BALANCE_CLOUDY                 },
	/***
	@field WHITE_BALANCE_HORIZON
	*/
	{"WHITE_BALANCE_HORIZON",                 WHITE_BALANCE_HORIZON                },
	/***
	@field WHITE_BALANCE_SHADE
	*/
	{"WHITE_BALANCE_SHADE",                   WHITE_BALANCE_SHADE                  },
	// photo mode
	{"PHOTO_MODE_NORMAL",    PHOTO_MODE_NORMAL   },
	{"PHOTO_MODE_PORTRAIT",  PHOTO_MODE_PORTRAIT },
	{"PHOTO_MODE_LANDSCAPE", PHOTO_MODE_LANDSCAPE},
	{"PHOTO_MODE_NIGHTVIEW", PHOTO_MODE_NIGHTVIEW},
	{"PHOTO_MODE_LETTER",    PHOTO_MODE_LETTER   },
	// camera special effects
	/***
	@field EFFECT_NONE
	*/
	{"EFFECT_NONE",     EFFECT_NONE    },
	/***
	@field EFFECT_MONO
	*/
	{"EFFECT_MONO",     EFFECT_MONO    },
	/***
	@field EFFECT_SEPIA
	*/
	{"EFFECT_SEPIA",    EFFECT_SEPIA   },
	/***
	@field EFFECT_NEGATIVE
	*/
	{"EFFECT_NEGATIVE", EFFECT_NEGATIVE},
	/***
	@field EFFECT_NEGAFILM
	*/
	{"EFFECT_NEGAFILM", EFFECT_NEGAFILM},
	/***
	@field EFFECT_SEPIA01
	*/
	{"EFFECT_SEPIA01",  EFFECT_SEPIA01 },
	// contrast
	/***
	@field CONTRAST_01
	*/
	{"CONTRAST_01",     CONTRAST_PATTERN_01},
	/***
	@field CONTRAST_02
	*/
	{"CONTRAST_02",     CONTRAST_PATTERN_02},
	/***
	@field CONTRAST_03
	*/
	{"CONTRAST_03",     CONTRAST_PATTERN_03},
	/***
	@field CONTRAST_04
	*/
	{"CONTRAST_04",     CONTRAST_PATTERN_04},
	/***
	@field CONTRAST_05
	*/
	{"CONTRAST_05",     CONTRAST_PATTERN_05},
	/***
	@field CONTRAST_06
	*/
	{"CONTRAST_06",     CONTRAST_PATTERN_06},
	/***
	@field CONTRAST_07
	*/
	{"CONTRAST_07",     CONTRAST_PATTERN_07},
	/***
	@field CONTRAST_08
	*/
	{"CONTRAST_08",     CONTRAST_PATTERN_08},
	/***
	@field CONTRAST_09
	*/
	{"CONTRAST_09",     CONTRAST_PATTERN_09},
	/***
	@field CONTRAST_10
	*/
	{"CONTRAST_10",     CONTRAST_PATTERN_10},
	/***
	@field CONTRAST_11
	*/
	{"CONTRAST_11",     CONTRAST_PATTERN_11},
	/***
	@field CONTRAST_LOW
	*/
	{"CONTRAST_LOW",    CONTRAST_LOW       },
	/***
	@field CONTRAST_NORMAL
	*/
	{"CONTRAST_NORMAL", CONTRAST_NORMAL    },
	/***
	@field CONTRAST_HIGH
	*/
	{"CONTRAST_HIGH",   CONTRAST_HIGH      },
	// lens correction
	/***
	@field LENS_CORRECTION_OFF
	*/
	{"LENS_CORRECTION_OFF",    LENS_CORRECTION_OFF   },
	/***
	@field LENS_CORRECTION_NORMAL
	*/
	{"LENS_CORRECTION_NORMAL", LENS_CORRECTION_NORMAL},
	/***
	@field LENS_CORRECTION_BRIGHT
	*/
	{"LENS_CORRECTION_BRIGHT", LENS_CORRECTION_BRIGHT},
	// shutter sounds
	/***
	@field SHUTTER_NORMAL
	*/
	{"SHUTTER_NORMAL",    SHUTTER_SOUND_TYPE_NORMAL   },
	/***
	@field SHUTTER_MOVIE
	*/
	{"SHUTTER_MOVIE",     SHUTTER_SOUND_TYPE_MOVIE    },
	/***
	@field SHUTTER_MOVIE_END
	*/
	{"SHUTTER_MOVIE_END", SHUTTER_SOUND_TYPE_MOVIE_END},
	// Wow, look, the end of the tunnel !
	// I think I will never do that again
	{NULL, 0}
};

int luaopen_cam_lib(lua_State *L) {
	luaL_newlib(L, cam_lib);
	
	for (int i = 0; cam_constants[i].name; i++) {
		lua_pushinteger(L, cam_constants[i].value);
		lua_setfield(L, -2, cam_constants[i].name);
	}
	
	return 1;
}

void load_cam_lib(lua_State *L) {
	luaL_requiref(L, "ctr.cam", luaopen_cam_lib, false);
}
