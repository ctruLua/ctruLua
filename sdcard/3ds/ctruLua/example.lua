local gfx = require("ctr.gfx")
local hid = require("ctr.hid")
local ctr = require("ctr")

local x = 50
local y = 50
local dMul = 1

local angle = 0

local texture1 = gfx.texture.load("sdmc:/3ds/ctruLua/icon.png", gfx.texture.TYPE_PNG);

gfx.color.setBackground(gfx.color.RGBA8(200, 200, 200))
gfx.set3D(true)

local function drawStuffIn3D(depth)
	gfx.text(2, 5, "Depth multiplicator: "..dMul)

	-- 3D stuff
	local depth = math.floor(depth * dMul)

	gfx.color.setDefault(0x00FFFFFF)
	gfx.rectangle(240 + depth*5, 150, 120, 10)

	gfx.point(10 + depth*3, 20, 0xFF0000FF)

	gfx.color.setDefault(0xFF0000FF)
	gfx.rectangle(x + depth*math.ceil(5*math.sin(ctr.time()/500)), y, 20, 20, angle)

	gfx.line(50 - depth*3, 50, 75 + depth*2, 96, gfx.color.RGBA8(52, 10, 65))

	gfx.circle(125 - depth*4, 125, 16)
end

while ctr.run() do
	hid.read()
	local keys = hid.keys()

	if keys.down.start then return end

	if keys.held.right then x = x + 1 end
	if keys.held.left then x = x - 1 end
	if keys.held.up then y = y - 1 end
	if keys.held.down then y = y + 1 end
	
	if keys.held.r then dMul = dMul + 0.05 end
	if keys.held.l then dMul = dMul - 0.05 end

	gfx.startFrame(gfx.GFX_TOP, gfx.GFX_LEFT)

		drawStuffIn3D(-1)

	gfx.endFrame()
	
	gfx.startFrame(gfx.GFX_TOP, gfx.GFX_RIGHT)
		
		drawStuffIn3D(1)
		
	gfx.endFrame()
	
	gfx.startFrame(gfx.GFX_BOTTOM)

		gfx.color.setDefault(0, 0, 0)
		gfx.text(5, 7, "FPS: "..math.ceil(gfx.getFPS()))
		gfx.text(5, 20, "Hello world, from Lua !", 20)
		gfx.text(5, 30, "Time: "..os.date())
		
		texture1:draw(240, 10, angle);

		local cx, cy = hid.circle()
		gfx.rectangle(40, 90, 60, 60, 0, 0xDDDDDDFF)
		gfx.circle(70 + math.ceil(cx/156 * 30), 120 - math.ceil(cy/156 * 30), 10, 0x000000FF)

	gfx.endFrame()

	angle = angle + 0.05
	if angle > 2*math.pi then angle = angle - 2*math.pi end

	gfx.render()
end

texture1:unload()
