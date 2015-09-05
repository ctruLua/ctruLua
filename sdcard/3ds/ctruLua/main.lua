local fs = require("ctr.fs")

repeat
	fs.setDirectory("sdmc:/3ds/ctruLua")
	local file = dofile("openfile.lua")("Choose a Lua file to execute", "/3ds/ctruLua/", ".lua", "exist")
	if file then
		fs.setDirectory(file:match("^(.-)[^/]*$"))
		dofile(file)
	end
until not file