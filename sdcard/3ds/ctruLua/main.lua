local ctr = require("ctr")
local gfx = require("ctr.gfx")

local sel = 1
local scroll = 0
local curdir = "/"
local files = ctr.fs.list(curdir)

while ctr.run() do
	ctr.hid.read()
	local keys = ctr.hid.keys()
	if keys.down.start then break end

	if keys.down.down and sel < #files then
		sel = sel + 1
		if sel > scroll + 14 then scroll = scroll + 1 end
	elseif keys.down.up and sel > 1 then
		sel = sel - 1
		if sel <= scroll then scroll = scroll - 1 end
	end

	if keys.down.a then
		local f = files[sel]

		if f.isDirectory then
			if f.name == ".." then curdir = curdir:gsub("[^/]+/$", "")
			else curdir = curdir..f.name.."/" end

			sel = 1
			scroll = 0
			files = ctr.fs.list(curdir)

			if curdir ~= "/" then
				table.insert(files, 1, { name = "..", isDirectory = true, fileSize = "parent directory" })
			end
		else
			if f.name:match("%..+$") == ".lua" then
				dofile(curdir..f.name)
				-- reset things the script could have changed
				gfx.color.setDefault(0xFFFFFFFF)
				gfx.color.setBackground(0x000000FF)
			end
		end
	end

	gfx.startFrame(gfx.GFX_TOP)

		gfx.rectangle(0, 10+(sel-scroll)*15, gfx.TOP_WIDTH, 15, 0, gfx.color.RGBA8(0, 0, 200))

		for i = scroll+1, scroll+14, 1 do
			local f = files[i]
			if not f then break end
			local name = f.isDirectory and "["..f.name.."]" or f.name.." ("..f.fileSize.."b)"
			if not f.isHidden then gfx.text(5, 12+(i-scroll)*15, name) end
		end

		gfx.rectangle(0, 0, gfx.TOP_WIDTH, 25, 0, gfx.color.RGBA8(200, 200, 200))
		gfx.text(3, 3, curdir, 13, gfx.color.RGBA8(0, 0, 0))

	gfx.endFrame()

	gfx.render()
end