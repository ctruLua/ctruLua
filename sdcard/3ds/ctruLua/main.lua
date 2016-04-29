local ctr = require("ctr")
local fs  = require("ctr.fs")
local gfx = require("ctr.gfx")

-- Set up path
local ldir = ctr.root.."libs/"
package.path = package.path..";".. ldir.."?.lua;".. ldir.."?/init.lua"
local filepicker = require("filepicker")

-- Erroring
local function displayError(err, trace)
	gfx.set3D(false)
	gfx.color.setBackground(0xFF0000B3)
	gfx.color.setDefault(0xFFFDFDFD)
	gfx.font.setSize(12)
	gfx.font.setDefault(gfx.font.load(ctr.root .. "resources/VeraMono.ttf"))
	gfx.disableConsole()

	while ctr.run() do
		gfx.start(gfx.BOTTOM)
			gfx.text(1, 1, "An error has occured.")
			gfx.wrappedText(1, 30, err, gfx.BOTTOM_WIDTH-2)
			gfx.text(1, gfx.BOTTOM_HEIGHT-15, "Press Start to continue.")
		gfx.stop()
		gfx.start(gfx.TOP)
			gfx.wrappedText(2, 6, trace, gfx.TOP_WIDTH - 2)
		gfx.stop()

		gfx.render()
		ctr.hid.read()
		if ctr.hid.keys().down.start then break end
	end
end

-- Main loop
while ctr.run() do
	gfx.set3D(false)
	gfx.font.setDefault()
	gfx.color.setDefault(0xFFFDFDFD)
	gfx.color.setBackground(0xFF333333)
	local file, ext, mode, key = filepicker(ctr.root, {
		["%.lua$"] = {
			a = {filepicker.openFile, "Run"},
			__name = "Lua Script"
		}
	})
	if mode then
		fs.setDirectory(file:match("^(.-)[^/]*$"))
		xpcall(dofile, function(err) displayError(err, debug.traceback()) end, file)
	else
		break
	end
end

error("Main process has exited.\nPlease reboot.\nPressing Start does not work yet.")
