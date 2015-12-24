local hex = require("ctr.gfx.color").hex

-- Colors based on the Monokai theme
return {
	-- General
	["background"] = hex(0x272822FF),
	["cursor"] = hex(0xF8F8F0FF),
	["default"] = hex(0xF8F8F2FF),

	-- Syntax
	["comment"] = hex(0x75715EFF),
	["string"] = hex(0xE6DB74FF),
	["constant.numeric"] = hex(0xAE81FFFF),
	["constant.language"] = hex(0xAE81FFFF),
	["keyword.control"] = hex(0xF92672FF),
	["keyword.operator"] = hex(0xF92672FF),
	["support.function"] = hex(0x66D9EFFF)
}