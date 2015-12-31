local ctr = require("ctr")
local gfx = require("ctr.gfx")
local map = require("ctr.gfx.map")
local texture = require("ctr.gfx.texture")
local hid = require("ctr.hid")

local tileset = assert(texture.load("tileset.png"))
local map = assert(map.load("map.csv", tileset, 16, 16))

local x,y = 0,0
local s = 0

while ctr.run() do
	hid.read()
	local keys = hid.keys()
	if keys.down.start then break end
	if keys.held.up then
		y = y - 1
	elseif keys.held.down then
		y = y + 1
	end
	if keys.held.left then
		x = x - 1
	elseif keys.held.right then
		x = x + 1
	end
	if keys.held.r then
		s = s + 1
		map:setSpace(s,s)
	elseif keys.held.l then
		s = s - 1
		map:setSpace(s,s)
	end
	
	gfx.start(gfx.TOP)
		map:draw(x,y)
	gfx.stop()
	
	gfx.start(gfx.BOTTOM)
		gfx.text(2, 2, "Map example")
		gfx.text(2, 20, "Press L (-) and R (+) to change the space between the tiles")
		gfx.text(2, 30, "Move the map with the D-pad or the C-pad")
	gfx.stop()
	
	gfx.render()
end

map:unload()
tileset:unload()
