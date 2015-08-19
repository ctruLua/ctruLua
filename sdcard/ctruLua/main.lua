local gfx = require("ctr.gfx")
local hid = require("ctr.hid")

local x = 0
local y = 0

local angle = 0

gfx.color.setBackground(gfx.color.RGBA8(200, 200, 200))

while os.run() do
	local keys = hid.read()

	if keys.down.start then return end

	if keys.held.right then x = x + 1 end
	if keys.held.left then x = x - 1 end
	if keys.held.up then y = y - 1 end
	if keys.held.down then y = y + 1 end

	gfx.startFrame(gfx.GFX_TOP)
	
		gfx.color.setDefault(0xFF0000FF)
		gfx.rectangle(x, y, 10, 10, angle)

		gfx.color.setDefault(0x00FFFFFF)
		gfx.rectangle(240, 150, 120, 10)

		gfx.line(50, 50, 75, 96, gfx.color.RGBA8(52, 255, 65))

		gfx.point(10, 10, 0xFF0000FF)

		gfx.circle(125, 125, 16)

	gfx.endFrame()

	gfx.startFrame(gfx.GFX_BOTTOM)

		gfx.color.setDefault(0, 0, 0)
		gfx.text(5, 10, "Hello world, from Lua !", 15)

	gfx.endFrame()

	angle = angle + 0.05
	if angle > 2*math.pi then angle = angle - 2*math.pi end

	gfx.render()
end
