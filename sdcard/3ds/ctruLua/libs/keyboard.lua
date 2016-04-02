local ctr = require("ctr")
local hid = require("ctr.hid")
local gfx = require("ctr.gfx")
local hex = gfx.color.hex

-- Options
local config = {}
loadfile(ctr.root .. "config/keyboard.cfg", nil, config)()

-- Variables
local currentModifier = { "default", "sticky" }
local buffer = ""

return {
	draw = function(x, y)
		local hidKeys = hid.keys()
		
		local xTouch, yTouch
		if hidKeys.down.touch then xTouch, yTouch = hid.touch() end
		
		for key, modifier in pairs(config.keys) do
			if hidKeys.down[key] then
				currentModifier = { modifier, "key" }
			elseif hidKeys.up[key] and currentModifier[2] == "key" and currentModifier[1] == modifier then
				currentModifier = { "default", "sticky" }
			end
		end
		
		for row, rowKeys in pairs(config.layout[currentModifier[1]]) do
			for column, key in pairs(rowKeys) do
				local xKey, yKey = x + (column-1)*(config.keyWidth-1), y + (row-1)*(config.keyHeight-1)
			
				gfx.rectangle(xKey, yKey, config.keyWidth, config.keyHeight, 0, hex(0xFFFFFFFF))
				gfx.rectangle(xKey + 1, yKey + 1, config.keyWidth - 2, config.keyHeight - 2, 0, hex(0x000000FF))
				gfx.text(xKey + 2, yKey + 2, key)
				
				if xTouch then
					if xTouch > xKey and xTouch < xKey + config.keyWidth then
						if yTouch > yKey and yTouch < yKey + config.keyHeight then
							gfx.rectangle(xKey, yKey, config.keyWidth, config.keyHeight, 0, hex(0xDDFFFFFF))
							
							local k = config.alias[key] or key
							if config.sticky[k] and config.layout[config.sticky[k]] then
								if currentModifier[1] == config.sticky[k] and currentModifier[2] == "sticky" then
									currentModifier = { "default", "sticky" }
								else
									currentModifier = { config.sticky[k], "sticky" }
								end
							elseif config.layout[k] then
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
