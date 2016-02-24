-- LSH version 0.1
-- ctrµLua official shell

local ctr = require("ctr")
local gfx = require("ctr.gfx")

local function saveGraphicsState()
	local old = {gfx.get3D(), gfx.color.getDefault(), gfx.color.getBackground(),
		gfx.font.getDefault()}

	local mono = gfx.font.load(ctruLua.root .. "resources/VeraMono.ttf")

	gfx.set3D(false)
	gfx.color.setDefault(0xFFFDFDFD)
	gfx.color.setBackground(0xFF333333)
	gfx.font.setDefault(mono)

	return old
end

local function restoreGraphicsState(state)
	gfx.set3D(state[1])
	gfx.color.setDefault(state[2])
	gfx.color.setBackground(state[3])
	gfx.font.setDefault(state[4])
end

local function getExtension(sel, bindings)
	for _, ext in ipairs(bindings) do
		if ext.ext == sel:match("%..+$") then
			return ext
		end
	end
end

local function getFilelist(cur)
	local files = ctr.fs.list(cur)

	if cur ~= "/" and cur ~= "sdmc:/" then
		table.insert(files, {name = "..", isDirectory = true})
	end

	-- Stealy stealing code from original openfile.lua
	table.sort(files, function(i, j)
		if i.isDirectory and not j.isDirectory then
			return true
		elseif i.isDirectory == j.isDirectory then
			return string.lower(i.name) < string.lower(j.name)
		end
	end)

	return files
end

local function drawBottom(cur, selFile, bindings)
	local ext = getExtension(selFile.name, bindings)

	gfx.start(gfx.BOTTOM)
		gfx.rectangle(0, 0, gfx.BOTTOM_WIDTH, 16, 0, 0xFF0000B3)
		gfx.text(1, 0, cur, 12)
		gfx.text(1, 15, selFile.name, 12)
		if not selFile.isDirectory then
			gfx.text(1, 45, selFile.fileSize, 12)
		end

		local keys = {"X: Quit/Cancel"}
		if selFile.isDirectory then 
			gfx.text(1, 30, "Directory", 12, 0xFF727272)
			gfx.text(1, gfx.BOTTOM_HEIGHT - 30, "A: Open", 12)
			gfx.text(1, gfx.BOTTOM_HEIGHT - 15, keys[1], 12)
		elseif ext then
			local lines = 1

			-- Keys
			if ext.y then
				lines = lines + 1
				table.insert(keys, "Y: " .. ext.y)
			end
			if ext.a then
				lines = lines + 1
				table.insert(keys, "A: " .. ext.a)
			end

			-- Drawing
			for i=lines, 1, -1 do
				gfx.text(1, gfx.BOTTOM_HEIGHT - 15*i, keys[i], 12)
			end
			gfx.text(1, 30, ext.name, 12, 0xFF727272)
			gfx.text(1, 45, tostring(selFile.fileSize) .. "B", 12, 0xFF727272)
		else
			gfx.text(1, 30, "File", 12, 0xFF727272)
			gfx.text(1, 45, tostring(selFile.fileSize) .. "B", 12, 0xFF727272)
			gfx.text(1, gfx.BOTTOM_HEIGHT - 15, keys[1], 12)
		end
	gfx.stop()
end

local function drawTop(files, sel, scr)
	gfx.start(gfx.TOP)
		gfx.rectangle(0, (sel-scr-1)*15, gfx.TOP_WIDTH, 16, 0, 0xFF0000B3)
		local over = #files - scr >= 16 and 16 or #files - scr
		for i=scr+1, scr+over do
			local color = files[i].isDirectory and 0xFF727272 or 0xFFFDFDFD
			gfx.text(1, (i-scr-1)*15+1, files[i].name or "", 12, color)
		end
	gfx.stop()
end

local function runA(cur, selFile, bindings)
	if not selFile.isDirectory then
		local ext = getExtension(selFile.name, bindings)
		if not ext then return end
		if ext.a then return cur .. selFile.name, ext.ext end
	end
end

local function runY(cur, selFile, bindings)
	if not selFile.isDirectory then
		local ext = getExtension(selFile.name, bindings)
		if not ext then return end
		if ext.y then return cur .. selFile.name, ext.ext end
	end
end

--- Open a file browser to allow the user to select a file.
-- It will save current graphical settings and set them back after ending. Press up or down to move one element at a time, and press left or right to go at the beginning or at the end of the list. This function is the return result of requiring filepicker.lua
-- @name filePicker
-- @param bindings A table of the extensions the user can select in the format {{name, ext, a, y},...}, name will show up instead of "File" or "Directory" on the bottom screen, a and y set the action names for those keys on the bottom screen (and also enable them, so if there's neither a or y, the file will have a custom type name but won't be effectively selectable), and ext is the extension to search for, dot included. Everything must be strings.
-- @param workdir Optional, current working directory will be used if not specified, otherwise, sets the path at which the file browser first shows up, a string.
-- @returns The absolute path to the file, nil in case no file was picked.
-- @returns The extension of the file, this might be helpful in cases were multiple file types could be expected, nil in case no file was picked.
-- @returns The "mode", which indicates which key was used to select the file, "A" or "Y". "X" in case no file was picked.
return function(bindings, workdir)
	-- Initialization
	local old = saveGraphicsState()
	local cur = workdir or ctr.fs.getDirectory()
	if cur:sub(-1) ~= "/" then
		cur = cur .. "/"
	end
	local bindings = bindings or {}

	local files = getFilelist(cur) or {{name = "- Empty -"}}
	local sel = 1
	local scr = 0

	while ctr.run() do
		drawBottom(cur, files[sel], bindings)
		drawTop(files, sel, scr)
		gfx.render()

		ctr.hid.read()
		local state = ctr.hid.keys()
		if (state.down.dDown or state.down.cpadDown) and sel < #files then
			sel = sel + 1
			if sel - scr >= 16 then
				scr = scr + 1
			end
		elseif (state.down.dUp or state.down.cpadUp) and sel > 1 then
			sel = sel - 1
			if sel == scr then
				scr = scr - 1
			end
		elseif state.down.dLeft or state.down.cpadLeft then
			sel = 1
			scr = 0
		elseif state.down.dRight or state.down.cpadRight then
			sel = #files
			if #files > 15 then
				scr = #files - 16
			end

		elseif state.down.a then
			local selFile = files[sel]
			if selFile.isDirectory then
				if selFile.name == ".." then
					cur = cur:gsub("[^/]+/$", "")
				else
					cur = cur .. selFile.name .. "/"
				end
				files, sel, scr = getFilelist(cur), 1, 0
			else
				local file, ext = runA(cur, selFile, bindings)
				if file then
					restoreGraphicsState(old)
					return file, ext, "A"
				end
			end
		elseif state.down.y then
			local file, ext = runY(cur, files[sel], bindings)
			if file then
				restoreGraphicsState(old)
				return file, ext, "Y"
			end
		elseif state.down.x then 
			restoreGraphicsState(old)
			return nil, nil, "X" 
		end
	end
end