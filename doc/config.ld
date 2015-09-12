-- General options
title = "ctrµLua documentation"
project = "ctrµLua"
description = "ctrµLua: Lua homebrewing for 3DS"
not_luadoc = true

-- Generation options
dir = "./html/"
style = "!fixed"
format = "markdown"
plain = true

-- Input files
topics = "../README.md"
file = "../source/"
examples = "../sdcard/3ds/ctruLua/examples/"
manual_url = "file://../libs/lua-5.3.1/doc/manual.html"

-- Custom tags
custom_tags = { { "newonly", hidden = true } }
custom_display_name_handler = function(item, default_handler)
	if item.tags.newonly then
		return default_handler(item).." <sup><img src=../../newonly.png alt='(N3DS only)'></sup>"
	end
	return default_handler(item)
end
