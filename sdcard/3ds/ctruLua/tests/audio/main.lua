local ctr = require("ctr")
local hid = require("ctr.hid")
local gfx = require("ctr.gfx")
local audio = require("ctr.audio")

local test = assert(audio.load("test.wav"))

local channel = -1
local speed = 1
local leftBalance = 0.5

while true do
	hid.read()
	local keys = hid.keys()
	if keys.down.start then break end

	if keys.down.a then
		channel = test:play()
	end

	if keys.down.up then
		speed = speed + 0.01
		test:speed(speed)
		audio.speed(nil, speed)
	end
	if keys.down.down then
		speed = speed - 0.01
		test:speed(speed)
		audio.speed(nil, speed)
	end

	if keys.down.left then
		leftBalance = leftBalance + 0.1
		test:mix(1-leftBalance, leftBalance)
		audio.mix(nil, leftBalance, 1-leftBalance)
	end
	if keys.down.right then
		leftBalance = leftBalance - 0.1
		test:mix(1-leftBalance, leftBalance)
		audio.mix(nil, leftBalance, 1-leftBalance)
	end

	gfx.start(gfx.GFX_TOP)
		gfx.text(5, 5, "Audio test! "..tostring(test:time()).."/"..tostring(test:duration()).."s")
		gfx.text(5, 25, "Last audio played on channel "..tostring(channel))
		gfx.text(5, 65, "Speed: "..(speed*100).."% - Left balance: "..(leftBalance*100).."%")
	gfx.stop()

	gfx.render()
end

test:unload()