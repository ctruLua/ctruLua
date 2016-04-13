local ctr = require("ctr")
local hid = require("ctr.hid")
local gfx = require("ctr.gfx")
local qtm = require("ctr.qtm")

qtm.init()

if not qtm.checkInitialized() then
	while ctr.run() do
		hid.read()
		if hid.keys().down.start then
			break
		end
		gfx.start(gfx.TOP)
			gfx.text(2, 2, "Couldn't initialize the QTM module.")
			gfx.text(2, 12, "You need a New 3DS in order to use this.")
		gfx.stop()
		gfx.render()
	end
	return
end

while ctr.run() do
	hid.read()
	local keys = hid.keys()
	if keys.down.start then break end
	
	local infos = qtm.getHeadtrackingInfo()
	
	gfx.start(gfx.TOP)
		if infos:checkHeadFullyDetected() then
			for i=1, 4 do
				gfx.point(infos:convertCoordToScreen(i, 400, 240))
			end
		end
	gfx.stop()
	
	gfx.start(gfx.BOTTOM)
		gfx.text(0, 0, "QTM example")
		for i=1, 4 do
			local x,y = infos[i]
			gfx.text(0, 10*i, i..": "..x..";"..y)
		end
	gfx.stop()
	
	gfx.render()
end

