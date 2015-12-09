local hid = require("ctr.hid")
local gfx = require("ctr.gfx")

-- Options
local keyWidth, keyHeight = 25, 25
local layout = {
	["default"] = {
		{ "&",     "é", "\"", "'", "(", "-", "è", "_", "ç", "à", ")", "=", "Back"  },
		{ "a",     "z", "e", "r", "t", "y", "u", "i", "o", "p", "^", "$",  "Enter" },
		{ "q",     "s", "d", "f", "g", "h", "j", "k", "l", "m", "ù", "*",  "Enter" },
		{ "Shift", "<", "w", "x", "c", "v", "b", "n", ",", ";", ":", "!",  "Tab"   },
		{ "CpLck", ">", "+", "/", " ", " ", " ", " ", " ", "{", "}", ".",  "AltGr" }
	},
	["Shift"] = {
		{ "1",     "2", "3", "4", "5", "6", "7", "8", "9", "0", "°", "+", "Back"  },
		{ "A",     "Z", "E", "R", "T", "Y", "U", "I", "O", "P", "¨", "£", "Enter" },
		{ "Q",     "S", "D", "F", "G", "H", "J", "K", "L", "M", "%", "µ", "Enter" },
		{ "Shift", ">", "W", "X", "C", "V", "B", "N", "?", ".", "/", "§", "Tab"   },
		{ "CpLck", "~", "#", "[", " ", " ", " ", " ", " ", "]", "|", "@", "AltGr" }
	},
	["AltGr"] = {
		{ "²",     "~", "#", "{", "[", "|", "`", "\\", "^", "@", "]", "}", "Back"  },
		{ "a",     "z", "€", "r", "t", "y", "u", "i",  "o", "p", "",  "¤", "Enter" },
		{ "q",     "s", "d", "f", "g", "h", "j", "k",  "l", "m", "",  "",  "Enter" },
		{ "Shift", "",  "w", "x", "c", "v", "b", "n",  "",  "",  "",  "",  "Tab"   },
		{ "CpLck", "",  "",  "",  " ", " ", " ", " ",  " ", "",  "",  "",  "AltGr" }
	},
}
local alias = {
	["Tab"] = "\t",
	["Enter"] = "\n",
	["Back"] = "BACK"
}
local sticky = {
	["CpLck"] = "Shift"
}
local keys = {
	["l"] = "Shift",
	["r"] = "Shift"
}

-- Variables
local currentModifier = { "default", "sticky" }
local buffer = ""

return {
	draw = function(x, y)
		local hidKeys = hid.keys()
		
		local xTouch, yTouch
		if hidKeys.down.touch then xTouch, yTouch = hid.touch() end
		
		for key, modifier in pairs(keys) do
			if hidKeys.down[key] then
				currentModifier = { modifier, "key" }
			elseif hidKeys.up[key] and currentModifier[2] == "key" and currentModifier[1] == modifier then
				currentModifier = { "default", "sticky" }
			end
		end
		
		for row, rowKeys in pairs(layout[currentModifier[1]]) do
			for column, key in pairs(rowKeys) do
				local xKey, yKey = x + (column-1)*(keyWidth-1), y + (row-1)*(keyHeight-1)
			
				gfx.rectangle(xKey, yKey, keyWidth, keyHeight, 0, 0xFFFFFFFF)
				gfx.rectangle(xKey + 1, yKey + 1, keyWidth - 2, keyHeight - 2, 0, 0x000000FF)
				gfx.text(xKey + 2, yKey + 2, key)
				
				if xTouch then
					if xTouch > xKey and xTouch < xKey + keyWidth then
						if yTouch > yKey and yTouch < yKey + keyHeight then
							gfx.rectangle(xKey, yKey, keyWidth, keyHeight, 0, 0xDDFFFFFF)
							
							local k = alias[key] or key
							if sticky[k] and layout[sticky[k]] then
								if currentModifier[1] == sticky[k] and currentModifier[2] == "sticky" then
									currentModifier = { "default", "sticky" }
								else
									currentModifier = { sticky[k], "sticky" }
								end
							elseif layout[k] then
								if currentModifier[1] == k and currentModifier[2] == "normal" then
									currentModifier = { "default", "sticky" }
								else
									currentModifier = { k, "normal" }
								end
							else
								buffer = buffer .. k
								if currentModifier[1] ~= "default" and currentModifier[2] == "normal" then
									currentModifier = { "default", "sticky" }
								end
							end
						end
					end
				end
			end
		end
	end,
	
	read = function()
		local ret = buffer
		buffer = ""
		
		return ret ~= "" and ret or nil
	end
}
