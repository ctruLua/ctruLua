repeat
	local file = dofile("sdmc:/3ds/ctruLua/openfile.lua")("Choose a Lua file to execute", "/3ds/ctruLua/", ".lua", "exist")
	if file then dofile(file) end
until not file