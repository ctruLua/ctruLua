--- Open a file explorer to select a file.
-- string title: title of the file explorer.
-- string curdir: the directory to initially open the file explorer in.
-- string exts: the file extensions the user can select, separated by ";". If nil, all extensions are accepted.
-- string type: "exist" to select an existing file, "new" to select an non-existing file or "any" to select a existing
--              or non-existing file name. If nil, defaults to "exist".
-- returns string: the file the user has selected, or nil if the explorer was closed without selecting a file.
--         string: "exist" if the file exist or "new" if it doesn't
return function(title, curdir, exts, type)
	-- Open libs
	local ctr = require("ctr")
	local gfx = require("ctr.gfx")
	
	local keyboard = dofile("sdmc:/3ds/ctruLua/keyboard.lua")
	
	-- Arguments
	local type = type or "exist"
	
	-- Variables
	local sel = 1
	local scroll = 0
	local files = ctr.fs.list(curdir)
	if curdir ~= "/" then table.insert(files, 1, { name = "..", isDirectory = true }) end
	local newFileName = ""
	
	local ret = nil
	
	-- Remember and set defaults
	local was3D = gfx.get3D()
	local wasDefault = gfx.color.getDefault()
	local wasBackground = gfx.color.getBackground()
	local wasFont = gfx.font.getDefault()
	gfx.set3D(false)
	gfx.color.setDefault(0xFFFFFFFF)
	gfx.color.setBackground(0x000000FF)
	gfx.font.setDefault()

	while ctr.run() do
		ctr.hid.read()
		local keys = ctr.hid.keys()
		if keys.down.start then break end
		
		-- Keys input
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
					table.insert(files, 1, { name = "..", isDirectory = true })
				end
			elseif type == "exist" or type == "any" then
				if exts then
					for ext in (exts..";"):gmatch("[^;]+") do
						if f.name:match("%..+$") == ext then
							ret = { curdir..f.name, "exist" }
							break
						end
					end
				else
					ret = { curdir..f.name, "exist" }
				end
				if ret then break end
			end
		end
		
		-- Keyboard input
		if type == "new" or type == "any" then
			local input = keyboard.read()
			if input then
				if input == "BACK" then
					newFileName = newFileName:sub(1, (utf8.offset(newFileName, -1) or 0)-1)
				elseif input == "\n" then
					local fileStatus = "new"
					local f = io.open(curdir..newFileName)
					if f then fileStatus = "exist" f:close() end
					ret = { curdir..newFileName, fileStatus }
					break
				else
					newFileName = newFileName..input
				end
			end
		end
		
		-- Draw
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
		
		gfx.startFrame(gfx.GFX_BOTTOM)
	
			gfx.text(5, 5, title)
			gfx.text(5, 20, "Accepted file extensions: "..(exts or "all"))
			
			if type == "new" or type == "any" then
				gfx.text(5, 90, newFileName)
				keyboard.draw(5, 115)
			end
		
		gfx.endFrame()

		gfx.render()
	end
	
	-- Reset defaults
	gfx.set3D(was3D)
	gfx.color.setDefault(wasDefault)
	gfx.color.setBackground(wasBackground)
	gfx.font.setDefault(wasFont)
	
	if ret then
		return table.unpack(ret)
	else
		return ret
	end
end
