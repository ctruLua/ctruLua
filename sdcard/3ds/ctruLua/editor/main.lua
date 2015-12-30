local ctr = require("ctr")
local hid = require("ctr.hid")
local gfx = require("ctr.gfx")

-- Open libs
local keyboard = require("keyboard")
local openfile = require("openfile")
local color = dofile("color.lua")
local syntax = dofile("syntax.lua")

-- Load data
local font = gfx.font.load("VeraMono.ttf")

-- Open file
local path, status = openfile("Choose a file to edit", nil, nil, "any")
if not path then return end
local lineEnding
local lines = {}
if status == "exist" then
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
local cursorX, cursorY = 1, 1
local scrollX, scrollY = 0, 0

-- Helper functions
local function displayedText(text)
	return text:gsub("\t", "    ")
end

-- Set defaults
gfx.set3D(false)
gfx.color.setDefault(color.default)
gfx.color.setBackground(color.background)
gfx.font.setDefault(font)

while ctr.run() do
	hid.read()
	local keys = hid.keys()
	
	-- Keys input
	if keys.down.start then return end
	
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
				gfx.start(gfx.GFX_TOP)
				gfx.text(3, 3, "Can't open file in write mode")
				gfx.stop()
				gfx.render()
			until t + 5 < os.time()
		else
			for i = 1, #lines, 1 do
				file:write(lines[i]..lineEnding)
				gfx.start(gfx.GFX_TOP)
				gfx.rectangle(0, 0, math.ceil(i/#lines*gfx.TOP_WIDTH), gfx.TOP_HEIGHT, 0, 0xFFFFFFFF)
				gfx.color.setDefault(color.background)
				gfx.text(gfx.TOP_WIDTH/2, gfx.TOP_HEIGHT/2, math.ceil(i/#lines*100).."%")
				gfx.color.setDefault(color.default)
				gfx.stop()
				gfx.render()
			end 
			file:flush()
			file:close()
		end
	end
	
	-- Keyboard input
	local input = keyboard.read()
	if input then
		if input == "BACK" then
			if cursorX > utf8.len(lines[cursorY])+1 then cursorX = utf8.len(lines[cursorY])+1 end
			if cursorX > 1 then
				lines[cursorY] = lines[cursorY]:sub(1, utf8.offset(lines[cursorY], cursorX-1)-1)..
				                 lines[cursorY]:sub(utf8.offset(lines[cursorY], cursorX), -1)
				cursorX = cursorX - 1
			elseif cursorY > 1 then
				cursorX, cursorY = utf8.len(lines[cursorY-1])+1, cursorY - 1
				lines[cursorY] = lines[cursorY]..lines[cursorY+1]
				table.remove(lines, cursorY+1)
			end
		elseif input == "\n" then
			local newline = lines[cursorY]:sub(utf8.offset(lines[cursorY], cursorX), -1)
			local whitespace = lines[cursorY]:match("^%s+")
			if whitespace then newline = whitespace .. newline end
			
			lines[cursorY] = lines[cursorY]:sub(1, utf8.offset(lines[cursorY], cursorX)-1)
			table.insert(lines, cursorY + 1, newline)
			
			cursorX, cursorY = whitespace and #whitespace+1 or 1, cursorY + 1
		else
			lines[cursorY] = lines[cursorY]:sub(1, utf8.offset(lines[cursorY], cursorX)-1)..input..
			                 lines[cursorY]:sub(utf8.offset(lines[cursorY], cursorX), -1)
			cursorX = cursorX + 1
		end
	end
	
	-- Draw
	gfx.start(gfx.GFX_TOP)

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

				gfx.color.setDefault(colored[2])
				gfx.text(x, y, str)
				gfx.color.setDefault(color.default)

				x = x + font:width(str)
			end
		end

		-- Cursor
		local curline = lines[cursorY]
		gfx.rectangle(-scrollX+ font:width(displayedText(curline:sub(1, (utf8.offset(curline, cursorX) or 0)-1))), 
		              -scrollY+ (cursorY-1)*lineHeight, 1, lineHeight, 0, color.cursor)

	gfx.stop()
	
	gfx.start(gfx.GFX_BOTTOM)

		gfx.text(3, 3, "FPS: "..math.ceil(gfx.getFPS()))
		
		keyboard.draw(5, 115)
		
	gfx.stop()

	gfx.render()
end

font:unload()