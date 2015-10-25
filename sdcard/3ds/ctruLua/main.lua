local fs = require("ctr.fs")

-- Set up path
local ldir = fs.getDirectory().."libs/"
package.path = package.path..";".. ldir.."?.lua;".. ldir.."?/init.lua"

repeat
	local file = require("openfile")("Choose a Lua file to execute", nil, ".lua", "exist")
	if file then
		fs.setDirectory(file:match("^(.-)[^/]*$"))
		dofile(file)
	end
until not file