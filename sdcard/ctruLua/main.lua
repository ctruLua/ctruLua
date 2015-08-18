local gfx = require("ctr.gfx")
local hid = require("ctr.hid")

local x = 0
local y = 0

while os.run() do
	local keys = hid.read()

	if keys.down.start then return end

	if keys.held.right then x = x + 1 end
	if keys.held.left then x = x - 1 end
	if keys.held.up then y = y - 1 end
	if keys.held.down then y = y + 1 end

	gfx.startFrame()
		gfx.rectangle(x, y, 10, 10)
		gfx.rectangle(240, 150, 120, 10)
	gfx.endFrame()

	gfx.render()
end