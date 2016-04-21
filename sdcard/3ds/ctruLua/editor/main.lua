local ctr = require("ctr")
local hid = require("ctr.hid")
local gfx = require("ctr.gfx")

-- Open libs
local keyboard = require("keyboard")
local filepicker = require("filepicker")
local color = dofile("color.lua")
local syntax = dofile("syntax.lua")

-- Load data
local font = gfx.font.load(ctr.root .. "resources/VeraMono.ttf")

-- Open file
local path, binding, mode, key = filepicker(nil, {__default = {
		a = {filepicker.openFile, "Open"},
		y = {filepicker.newFile, "New File"}
	}
})
if not mode then return end
local lineEnding
local lines = {}
if mode == "open" then
	for line in io.lines(path, "L") do
		if not lineEnding then lineEnding = line:match("([\n\r]+)$") end
		table.insert(lines, line:match("^(.-)[\n\r]*$"))
	end
else
	lineEnding = "\n"
	lines = { "" }
end

-- Syntax coloring
local coloredLines = syntax(lines, color)

-- Variables
local lineHeight = 10
local fontSize = 9
local cursorX, cursorY = 1, 1
local scrollX, scrollY = 0, 0
local fileModified = false

-- Helper functions
local function displayedText(text)
	return text:gsub("\t", "    "), nil
end

-- Set defaults
gfx.set3D(false)
gfx.color.setDefault(color.default)
gfx.color.setBackground(color.background)
gfx.font.setDefault(font)
gfx.font.setSize(fontSize)

while ctr.run() do
	hid.read()
	local keys = hid.keys()
	
	-- Keys input
	if keys.down.start then
		local exit = not fileModified
		if fileModified then
			while ctr.run() do
				hid.read()
				local keys = hid.keys()
				if keys.down.b then
					exit = false
					break
				elseif keys.down.a then
					exit = true
					break
				end
				gfx.start(gfx.TOP)
					gfx.text(3, 3, "The file was modified but not saved!")
					gfx.text(3, 3 + lineHeight, "Are you sure you want to exit without saving?")
					gfx.text(3, 3 + lineHeight*2, "Press A to exit and discard the modified file")
					gfx.text(3, 3 + lineHeight*3, "Press B to return to the editor")
				gfx.stop()
				gfx.render()
			end
		end
		if exit then break end
	end
	
	if keys.down.dRight then
		cursorX = cursorX + 1
		if cursorX > utf8.len(lines[cursorY])+1 then
			if cursorY < #lines then
				cursorX, cursorY = 1, cursorY + 1
			else
				cursorX = cursorX - 1
			end
		end
	end
	if keys.down.dLeft then
		if cursorX > utf8.len(lines[cursorY])+1 then cursorX = utf8.len(lines[cursorY])+1 end
		cursorX = cursorX - 1
		if cursorX < 1 then
			if cursorY > 1 then
				cursorX, cursorY = utf8.len(lines[cursorY-1])+1, cursorY - 1
			else
				cursorX = 1
			end
		end
	end
	if keys.down.dUp and cursorY > 1 then cursorY = cursorY - 1 end
	if keys.down.dDown and cursorY < #lines then cursorY = cursorY + 1 end
	
	if keys.held.cpadRight or keys.held.a then scrollX = scrollX + 3 end
	if keys.held.cpadLeft or keys.held.y then scrollX = scrollX - 3 end
	if keys.held.cpadUp or keys.held.x then scrollY = scrollY - 3 end
	if keys.held.cpadDown or keys.held.b then scrollY = scrollY + 3 end
	
	if keys.down.select then
		local file = io.open(path, "w")
		if not file then
			local t = os.time()
			repeat
				gfx.start(gfx.TOP)
				gfx.text(3, 3, "Can't open file in write mode")
				gfx.stop()
				gfx.render()
			until t + 5 < os.time()
		else
			for i = 1, #lines, 1 do
				file:write(lines[i]..lineEnding)
				gfx.start(gfx.TOP)
					gfx.rectangle(0, 0, math.ceil(i/#lines*gfx.TOP_WIDTH), gfx.TOP_HEIGHT, 0, 0xFFFFFFFF)
					gfx.color.setDefault(color.background)
					gfx.text(gfx.TOP_WIDTH/2, gfx.TOP_HEIGHT/2, math.ceil(i/#lines*100).."%")
					gfx.color.setDefault(color.default)
				gfx.stop()
				gfx.render()
			end 
			file:flush()
			file:close()
			fileModified = false
		end
	end
	
	-- Keyboard input
	local input = keyboard.read()
	if input then
		if input == "\b" then
			if cursorX > utf8.len(lines[cursorY])+1 then cursorX = utf8.len(lines[cursorY])+1 end
			if cursorX > 1 then
				lines[cursorY] = lines[cursorY]:sub(1, utf8.offset(lines[cursorY], cursorX-1)-1)..
				                 lines[cursorY]:sub(utf8.offset(lines[cursorY], cursorX), -1)
				cursorX = cursorX - 1
			elseif cursorY > 1 then
				cursorX, cursorY = utf8.len(lines[cursorY-1])+1, cursorY - 1
				lines[cursorY] = lines[cursorY]..lines[cursorY+1]
				table.remove(lines, cursorY+1)
				table.remove(coloredLines, cursorY+1)
			end

			coloredLines[cursorY] = syntax(lines[cursorY], color)

		elseif input == "\n" then
			local newline = lines[cursorY]:sub(utf8.offset(lines[cursorY], cursorX), -1)
			local whitespace = lines[cursorY]:match("^%s+")
			if whitespace then newline = whitespace .. newline end
			
			lines[cursorY] = lines[cursorY]:sub(1, utf8.offset(lines[cursorY], cursorX)-1)
			coloredLines[cursorY] = syntax(lines[cursorY], color)
			table.insert(lines, cursorY + 1, newline)
			table.insert(coloredLines, cursorY + 1, syntax(newline, color))
			
			cursorX, cursorY = whitespace and #whitespace+1 or 1, cursorY + 1

		else
			lines[cursorY] = lines[cursorY]:sub(1, utf8.offset(lines[cursorY], cursorX)-1)..input..
			                 lines[cursorY]:sub(utf8.offset(lines[cursorY], cursorX), -1)
			coloredLines[cursorY] = syntax(lines[cursorY], color)
			cursorX = cursorX + 1
		end
		fileModified = true
	end
	
	-- Draw
	gfx.start(gfx.TOP)

		-- Lines
		local sI = math.floor(scrollY / lineHeight)
		if sI < 1 then sI = 1 end
		
		local eI = math.ceil((scrollY + gfx.TOP_HEIGHT) / lineHeight)
		if eI > #lines then eI = #lines end
		
		for i = sI, eI, 1 do
			local x = -scrollX
			local y = -scrollY+ (i-1)*lineHeight

			for _,colored in ipairs(coloredLines[i]) do
				local str = displayedText(colored[1])
				gfx.text(x, y, str, fontSize, colored[2])
				x = x + font:width(str)
			end
		end

		-- Cursor
		local curline = lines[cursorY]
		gfx.rectangle(-scrollX+ font:width(displayedText(curline:sub(1, (utf8.offset(curline, cursorX) or 0)-1))), 
		              -scrollY+ (cursorY-1)*lineHeight, 1, lineHeight, 0, color.cursor)

	gfx.stop()
	
	gfx.start(gfx.BOTTOM)

		gfx.text(3, 3, "FPS: "..math.ceil(gfx.getFPS()))
		gfx.text(3, 3 + lineHeight, "Press select to save.")
		gfx.text(3, 3 + lineHeight*2, "Press start to exit.")
		
		keyboard.draw(4, 115)
		
	gfx.stop()

	gfx.render()
end

font:unload()
