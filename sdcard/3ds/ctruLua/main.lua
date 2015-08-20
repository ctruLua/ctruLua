local ctr = require("ctr")
local gfx = require("ctr.gfx")

local sel = 1
local curdir = "/"
local files = ctr.fs.list(curdir)

while ctr.run() do
	ctr.hid.read()
	local keys = ctr.hid.keys()
	if keys.down.start then break end

	if keys.down.down and sel < #files then sel = sel + 1
	elseif keys.down.up and sel > 1 then sel = sel - 1 end

	if keys.down.a then
		local f = files[sel]

		if f.isDirectory then
			if f.name == ".." then curdir = curdir:gsub("[^/]+/$", "")
			else curdir = curdir..f.name.."/" end

			sel = 1
			files = ctr.fs.list(curdir)

			if curdir ~= "/" then
				table.insert(files, 1, { name = "..", isDirectory = true, fileSize = "parent directory" })
			end
		else
			if f.name:match("%..+$") == ".lua" then
				dofile(curdir..f.name)
			end
		end
	end

	gfx.startFrame(gfx.GFX_TOP)

		gfx.text(3, 9, curdir)

		for i,f in pairs(files) do
			local name = f.isDirectory and "["..f.name.."]" or f.name.." ("..f.fileSize.."b)"
			if not f.isHidden then gfx.text(5, 9+i*9, name) end
		end

		gfx.text(0, 9+sel*9, ">")

	gfx.endFrame()

	gfx.render()
end