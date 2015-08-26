local ctr = require("ctr")
local hid = require("ctr.hid")
local gfx = require("ctr.gfx")

-- Open libs
local keyboard = dofile("sdmc:/3ds/ctruLua/keyboard.lua")
local openfile = dofile("sdmc:/3ds/ctruLua/openfile.lua")
local color = dofile("sdmc:/3ds/ctruLua/editor/color.lua")

-- Open file
local path, status = openfile("Choose a file to edit", "/3ds/ctruLua/", nil, "any")
if not path then return end
local lines = {}
if status == "exist" then
	for line in io.lines(path) do table.insert(lines, line) end
else
	lines = { "" }
end

-- Variables
local lineHeight = 10
local cursorX, cursorY = 1, 1
local scrollX, scrollY = 0, 0

-- Set defaults
gfx.set3D(false)
gfx.color.setDefault(color.default)
gfx.color.setBackground(color.background)

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
				gfx.startFrame(gfx.GFX_TOP)
				gfx.text(3, 3, "Can't open file in write mode")
				gfx.endFrame()
				gfx.render()
			until t + 5 < os.time()
		else
			for i = 1, #lines, 1 do
				file:write(lines[i].."\n")
				gfx.startFrame(gfx.GFX_TOP)
				gfx.rectangle(0, 0, math.ceil(i/#lines*gfx.TOP_WIDTH), gfx.TOP_HEIGHT, 0, 0xFFFFFFFF)
				gfx.color.setDefault(0x000000FF)
				gfx.text(gfx.TOP_WIDTH/2, gfx.TOP_HEIGHT/2, math.ceil(i/#lines*100).."%")
				gfx.color.setDefault(color.default)
				gfx.endFrame()
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
	gfx.startFrame(gfx.GFX_TOP)

		local sI = math.floor(scrollY / lineHeight)
		if sI < 1 then sI = 1 end
		
		local eI = math.ceil((scrollY + gfx.TOP_HEIGHT) / lineHeight)
		if eI > #lines then eI = #lines end
		
		for i = sI, eI, 1 do
			local line = lines[i]
			local y = -scrollY+ (i-1)*lineHeight
			
			if cursorY == i then
				gfx.color.setDefault(color.cursor)
				gfx.text(-scrollX, y, line:sub(1, (utf8.offset(line, cursorX) or 0)-1):gsub("\t", "    ").."|", nil) -- TODO: color doesn't work
				gfx.color.setDefault(color.default)
			end
			
			gfx.text(-scrollX, y, line:gsub("\t", "    "), nil)
		end

	gfx.endFrame()
	
	gfx.startFrame(gfx.GFX_BOTTOM)
		
		keyboard.draw(5, 115)
		
	gfx.endFrame()

	gfx.render()
end
