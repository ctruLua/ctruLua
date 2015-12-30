local gfx = require("ctr.gfx")
local hid = require("ctr.hid")
local ctr = require("ctr")

local x = 50
local y = 50
local dMul = 1

local angle = 0

local texture1 = gfx.texture.load("sdmc:/3ds/ctruLua/icon.png");
if not texture1 then error("Giants ducks came from another planet") end

gfx.color.setBackground(gfx.color.RGBA8(200, 200, 200))
gfx.set3D(true)

-- eye : -1 = left, 1 = right
local function drawStuffIn3D(eye)
	gfx.text(2, 5, "Depth multiplicator: "..dMul)

	-- 3D stuff
	local function d(depth) return math.ceil(eye * depth * dMul) end

	gfx.color.setDefault(0xFFFFFF00)
	gfx.rectangle(240 + d(10), 150, 120, 10)

	gfx.point(10 + d(6), 20, 0xFF0000FF)

	gfx.color.setDefault(0xFF0000FF)
	gfx.rectangle(x + d(10*math.sin(ctr.time()/500)), y, 20, 20, angle)

	gfx.line(50 + d(-6), 50, 75 + d(4), 96, gfx.color.RGBA8(52, 10, 65))

	gfx.circle(125 + d(-8), 125, 16)
end

while ctr.run() do
	hid.read()
	local keys = hid.keys()

	if keys.down.start then return end

	if keys.held.right then x = x + 1 end
	if keys.held.left then x = x - 1 end
	if keys.held.up then y = y - 1 end
	if keys.held.down then y = y + 1 end
	
	dMul = hid.pos3d()

	gfx.start(gfx.TOP, gfx.LEFT)

		drawStuffIn3D(-1)

	gfx.stop()
	
	gfx.start(gfx.TOP, gfx.RIGHT)
		
		drawStuffIn3D(1)
		
	gfx.stop()
	
	gfx.start(gfx.BOTTOM)

		gfx.color.setDefault(gfx.color.RGBA8(0, 0, 0))
		gfx.text(5, 5, "FPS: "..math.ceil(gfx.getFPS()))
		gfx.text(5, 17, "Hello world, from Lua ! éàçù", 20, gfx.color.RGBA8(0, 0, 0))
		gfx.text(5, 50, "Time: "..os.date())
		
		texture1:draw(280, 80, angle);

		local cx, cy = hid.circle()
		gfx.rectangle(40, 90, 60, 60, 0, 0xDDDDDDFF)
		gfx.circle(70 + math.ceil(cx/156 * 30), 120 - math.ceil(cy/156 * 30), 10, 0xFF000000)

	gfx.stop()

	angle = angle + 0.05
	if angle > 2*math.pi then angle = angle - 2*math.pi end

	gfx.render()
end

texture1:unload()
