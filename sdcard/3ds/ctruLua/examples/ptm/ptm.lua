local ctr = require("ctr")
local gfx = require("ctr.gfx")
local hid = require("ctr.hid")
local ptm = require("ctr.ptm")

while ctr.run() do
	hid.read()
	local keys = hid.keys()
	if keys.down.start then break end
	
	gfx.start(gfx.TOP)
		gfx.text(2, 2, "PTM example")
		local level = ptm.getBatteryLevel()
		gfx.text(2, 20, "Battery level: ["..string.rep("|", level)..string.rep(" ", 5-level).."]")
		gfx.text(2, 30, "Charging: "..((ptm.getBatteryChargeState() and "Yes") or "No"))
		gfx.text(2, 40, "You walked: "..ptm.getTotalStepCount().." steps")
		gfx.text(2, 50, "Counting: "..((ptm.getPedometerState() and "Yes") or "No"))
	gfx.stop()
	
	gfx.render()
end
