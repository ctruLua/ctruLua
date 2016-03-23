local ctr = require("ctr")
local hid = require("ctr.hid")
local gfx = require("ctr.gfx")
local audio = require("ctr.audio")

local test = assert(audio.load("test.wav")) -- Load audio file

local channel = -1
local speed = 1
local leftBalance = 0.5

while true do
	hid.read()
	local keys = hid.keys()
	if keys.down.start then break end

	if keys.down.a then
		channel = test:play() -- Play audio (an audio can be played multiple times at the same time)
	end

	if keys.down.up then
		speed = speed + 0.01
		test:speed(speed) -- Set the audio object default speed, will affect the next time it is played
		audio.speed(nil, speed) -- Set the speed of all the currently playing channels
	end
	if keys.down.down then
		speed = speed - 0.01
		test:speed(speed) -- See above
		audio.speed(nil, speed)
	end

	if keys.down.left then
		leftBalance = leftBalance + 0.1
		test:mix(1-leftBalance, leftBalance) -- Set the audio mix: left speaker will be at leftBalance% and right speaker 1-leftBalance%.
		-- Like with test:speed(), it won't affect the currently playing audios but the nexts test:play() will have theses mix paramters.
		audio.mix(nil, leftBalance, 1-leftBalance) -- Set the mix of all currently playing channels
	end
	if keys.down.right then
		leftBalance = leftBalance - 0.1
		test:mix(1-leftBalance, leftBalance) -- See above
		audio.mix(nil, leftBalance, 1-leftBalance)
	end

	gfx.start(gfx.TOP)
		gfx.text(5, 5, "Audio test! "..tostring(test:time()).."/"..tostring(test:duration()).."s")
		gfx.text(5, 25, "Last audio played on channel "..tostring(channel))
		gfx.text(5, 65, "Speed: "..(speed*100).."% - Left balance: "..(leftBalance*100).."%")
	gfx.stop()

	audio.update()
	gfx.render()
end

test:unload() -- Unload audio file