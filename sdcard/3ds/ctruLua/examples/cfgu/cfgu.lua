local ctr = require("ctr")
local gfx = require("ctr.gfx")
local hid = require("ctr.hid")
local cfgu = require("ctr.cfgu")

local regions = {
	[cfgu.REGION_JPN] = "Japan",
	[cfgu.REGION_USA] = "America",
	[cfgu.REGION_EUR] = "Europe",
	[cfgu.REGION_AUS] = "Australia",
	[cfgu.REGION_CHN] = "China",
	[cfgu.REGION_KOR] = "Korea",
	[cfgu.REGION_TWN] = "Taiwan"
}

local languages = {
	[cfgu.LANGUAGE_JP] = "Japanese",
	[cfgu.LANGUAGE_EN] = "English",
	[cfgu.LANGUAGE_FR] = "French",
	[cfgu.LANGUAGE_DE] = "Deutch",
	[cfgu.LANGUAGE_IT] = "Italian",
	[cfgu.LANGUAGE_ES] = "Spanish",
	[cfgu.LANGUAGE_ZH] = "Chinese",
	[cfgu.LANGUAGE_KO] = "Korean",
	[cfgu.LANGUAGE_NL] = "Dutch",
	[cfgu.LANGUAGE_PT] = "Portuguese",
	[cfgu.LANGUAGE_RU] = "Russian",
	[cfgu.LANGUAGE_TW] = "Taiwanese"
}

local models = {
	[cfgu.MODEL_3DS] = "3DS",
	[cfgu.MODEL_3DSXL] = "3DS XL",
	[cfgu.MODEL_N3DS] = "New 3DS",
	[cfgu.MODEL_2DS] = "2DS",
	[cfgu.MODEL_N3DSXL] = "New 3DS XL"
}

cfgu.init()
while ctr.run() do
	hid.read()
	keys = hid.keys()
	if keys.down.start then break end
	
	gfx.start(gfx.BOTTOM)
		gfx.text(2, 2, "CFGU example")
		gfx.text(2, 20, "Region: "..regions[cfgu.getRegion()])
		gfx.text(2, 30, "Model: "..models[cfgu.getModel()])
		gfx.text(2, 40, "Language: "..languages[cfgu.getLanguage()])
		gfx.text(2, 50, "Username: "..cfgu.getUsername())
		local m,d = cfgu.getBirthday()
		gfx.text(2, 60, "Birthday: "..d.."/"..m)
		gfx.text(2, 70, "0 Hash: "..cfgu.genHash(0))
	gfx.stop()
	
	gfx.render()
end
