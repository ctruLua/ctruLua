local fs = require("ctr.fs")

-- Set up path
local ldir = fs.getDirectory().."libs/"
package.path = package.path..";".. ldir.."?.lua;".. ldir.."?/init.lua"

repeat
	local file = require("openfile")("Choose a Lua file to execute", nil, ".lua", "exist")
	if file then
		fs.setDirectory(file:match("^(.-)[^/]*$"))
		local success, err = pcall(dofile, file)
		if not success then
			local gfx = require("ctr.gfx")
			local hid = require("ctr.hid")
			gfx.set3D(false)
			gfx.color.setDefault(0xFFFFFFFF)
			gfx.color.setBackground(0xFF000000)
			gfx.font.setDefault()
			while true do
				hid.read()
				if hid.keys().down.start then break end
				gfx.startFrame(gfx.GFX_TOP)
					gfx.wrappedText(0, 0, err, gfx.TOP_WIDTH)
				gfx.endFrame()
				gfx.render()
			end
		end
	end
until not file