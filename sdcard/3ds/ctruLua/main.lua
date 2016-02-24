local ctr = require("ctr")
local fs  = require("ctr.fs")
local gfx = require("ctr.gfx")

-- Initializing "constants"
ctruLua = {}

--- The ctruLua root directory's absolute path
ctruLua.root = fs.getDirectory()

-- Set up path
local ldir = fs.getDirectory().."libs/"
package.path = package.path..";".. ldir.."?.lua;".. ldir.."?/init.lua"

-- Erroring
local function displayError(err)
	gfx.set3D(false)
	gfx.color.setBackground(0xFF0000B3)
	gfx.color.setDefault(0xFFFDFDFD)
	gfx.font.setDefault(gfx.font.load(ctruLua.root .. "resources/VeraMono.ttf"))

	while ctr.run() do
		gfx.start(gfx.TOP)
			gfx.text(1, 1, "An error has occured.", 12)
			gfx.wrappedText(1, 30, err, gfx.TOP_WIDTH-2, 12)
			gfx.text(1, gfx.TOP_HEIGHT-15, "Press Start to continue.", 12)
		gfx.stop()
		gfx.start(gfx.BOTTOM)
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
	local file, ext, mode = require("filepicker")({{name="Lua Script", ext=".lua", a="Execute"}})
	if file and mode == "A" then
		fs.setDirectory(file:match("^(.-)[^/]*$"))
		local ok, err = pcall(dofile, file)
		if not ok then displayError(err) end
	else
		break
	end
end

error("Main process has exited.\nPlease reboot.\nPressing Start does not work yet.")