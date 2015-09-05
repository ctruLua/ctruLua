-- Each pattern should return 3 captures : start position, the string to colorize, and the end position.
local syntax = {
	{ "comment", { "()(%-%-.*)()$" } },

	--["string"] = { "()(%'.*%f[%\\]%')()", "()(%\".*%f[%\\]%\")()" },
	{ "string", { "()(%'[^%']*%')()", "()(%\"[^%\"]*%\")()" } },

	{ "constant.numeric", {
		"%f[%d%w%.]()(0x[a-fA-F%d]+)()%f[^%d%w%.]",
		"%f[%d%w%.]()([%d% ]+%.[%d% ]+)()%f[^%d%w%.]",
		"%f[%d%w%.]()([%d% ]+)()%f[^%d%w%.]"
		}
	},

	{ "constant.language", {
		"%f[%w]()(false)()%f[%W]", "%f[%w]()(nil)()%f[%W]", "%f[%w]()(true)()%f[%W]", "%f[%w]()(_G)()%f[%W]",
		"%f[%w]()(_VERSION)()%f[%W]", "%f[%w]()(math.pi)()%f[%W]", "%f[%w]()(math.huge)()%f[%W]", "%f[%w]()(%.%.%.)()%f[%W]"
		}
	},

	{ "keyword.control", {
		"%f[%w]()(break)()%f[%W]", "%f[%w]()(goto)()%f[%W]", "%f[%w]()(do)()%f[%W]", "%f[%w]()(else)()%f[%W]",
		"%f[%w]()(for)()%f[%W]", "%f[%w]()(if)()%f[%W]", "%f[%w]()(elseif)()%f[%W]", "%f[%w]()(return)()%f[%W]",
		"%f[%w]()(then)()%f[%W]", "%f[%w]()(repeat)()%f[%W]", "%f[%w]()(while)()%f[%W]", "%f[%w]()(until)()%f[%W]",
		"%f[%w]()(end)()%f[%W]", "%f[%w]()(function)()%f[%W]", "%f[%w]()(local)()%f[%W]", "%f[%w]()(in)()%f[%W]"
		}
	},

	{ "keyword.operator", {
		"%f[%w]()(and)()%f[%W]", "%f[%w]()(or)()%f[%W]", "%f[%w]()(not)()%f[%W]",
		"()(%+)()", "()(%-)()", "()(%%)()", "()(%#)()", "()(%*)()", "()(%/%/?)()", "()(%^)()", "()(%=%=?)()", "()(%~%=?)()",
		"()(%.%.)()", "()(%<%=?)()", "()(%>%=?)()", "()(%&)()", "()(%|)()", "()(%<%<)()", "()(%>%>)()", 
		}
	},

	{ "support.function", {
		"[^%.%:]()(assert)()[%( %{]", "[^%.%:]()(collectgarbage)()[%( %{]", "[^%.%:]()(dofile)()[%( %{]",
		"[^%.%:]()(error)()[%( %{]", "[^%.%:]()(getfenv)()[%( %{]", "[^%.%:]()(getmetatable)()[%( %{]",
		"[^%.%:]()(ipairs)()[%( %{]", "[^%.%:]()loadfile)()[%( %{]", "[^%.%:]()(loadstring)()[%( %{]",
		"[^%.%:]()(module)()[%( %{]", "[^%.%:]()(next)()[%( %{]", "[^%.%:]()(pairs)()[%( %{]",
		"[^%.%:]()(pcall)()[%( %{]", "[^%.%:]()(print)()[%( %{]", "[^%.%:]()(rawequal)()[%( %{]",
		"[^%.%:]()(rawget)()[%( %{]", "[^%.%:]()(rawset)()[%( %{]", "[^%.%:]()(require)()[%( %{]",
		"[^%.%:]()(select)()[%( %{]", "[^%.%:]()(setfenv)()[%( %{]", "[^%.%:]()(setmetatable)()[%( %{]",
		"[^%.%:]()(tonumber)()[%( %{]", "[^%.%:]()(tostring)()[%( %{]", "[^%.%:]()(type)()[%( %{]", 
		"[^%.%:]()(unpack)()[%( %{]", "[^%.%:]()(xpcall)()[%( %{]"
		}
	}
}

return function(lines, color)
	local ret = {}

	for _,line in ipairs(lines) do
		local colored = { { line, color.default } }

		for _, patterns in ipairs(syntax) do
			local name = patterns[1]

			for _, pattern in ipairs(patterns[2]) do
				local i = 1
				while i <= #colored do
					local oldcolor = colored[i][2]

					if oldcolor == color.default then
						local part = colored[i][1]
						
						local starti, match, endi = part:match(pattern)
						if starti then
							table.remove(colored, i)
							if starti > 1 then
								table.insert(colored, i, { part:sub(1, starti-1), oldcolor })
								i = i + 1
							end
							table.insert(colored, i, { match, color[name] or color.default })
							if endi <= #part then
								table.insert(colored, i+1, { part:sub(endi, -1), oldcolor })
							end
						end
					end

					i = i + 1
				end
			end
		end

		table.insert(ret, colored)
	end

	return ret
end