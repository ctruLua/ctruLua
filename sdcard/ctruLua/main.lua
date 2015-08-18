local gfx = require("ctr.gfx")
local hid = require("ctr.hid")

local x = 0
local y = 0

gfx.color.setBackground(gfx.color.RGBA8(200, 200, 200))

while os.run() do
	local keys = hid.read()

	if keys.down.start then return end

	if keys.held.right then x = x + 1 end
	if keys.held.left then x = x - 1 end
	if keys.held.up then y = y - 1 end
	if keys.held.down then y = y + 1 end

	gfx.startFrame()
	
		gfx.color.setDefault(0xFF0000FF)
		gfx.rectangle(x, y, 10, 10)

		gfx.color.setDefault(0x00FFFFFF)
		gfx.rectangle(240, 150, 120, 10)

	gfx.endFrame()

	gfx.render()
end