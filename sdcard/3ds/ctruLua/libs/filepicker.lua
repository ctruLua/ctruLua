local ctr = require('ctr')
local keyboard = require('keyboard')

local gfx = ctr.gfx

local externalConfig

local function gfxPrepare()
	local old = {gfx.get3D(), gfx.color.getDefault(), gfx.color.getBackground(),
		gfx.font.getDefault(), gfx.font.getSize()}

	local mono = gfx.font.load(ctr.root .. "resources/VeraMono.ttf")

	gfx.set3D(false)
	gfx.color.setDefault(0xFFFDFDFD)
	gfx.color.setBackground(0xFF333333)
	gfx.font.setDefault(mono)
	gfx.font.setSize(12)

	return old
end

local function gfxRestore(state)
	gfx.set3D(state[1])
	gfx.color.setDefault(state[2])
	gfx.color.setBackground(state[3])
	gfx.font.setDefault(state[4])
	gfx.font.setSize(state[5])
end

local function systemBindings(bindings)
	bindings.__default.up = {
		function(_, selected, ...)
			if selected.inList > 1 then
				selected.inList = selected.inList - 1
				if selected.inList == selected.offset then
					selected.offset = selected.offset - 1
				end
			end
		end
	}

	bindings.__default.down = {
		function(externalConfig, selected, ...)
			if selected.inList < #externalConfig.fileList then
				selected.inList = selected.inList + 1
				if selected.inList - selected.offset >= 16 then
					selected.offset = selected.offset + 1
				end
			end
		end
	}

	bindings.__default.left = {
		function(_, selected, ...)
			selected.inList, selected.offset = 1, 0
		end 
	}

	bindings.__default.right = {
		function(externalConfig, selected, ...)
			selected.inList = #externalConfig.fileList
			if #externalConfig.fileList > 15 then
				selected.offset = #externalConfig.fileList - 16
			end
		end 
	}
end

local function getFileList(workingDirectory)
  local fileList = ctr.fs.list(workingDirectory)

  if workingDirectory ~= "/" and workingDirectory ~= "sdmc:/" then
    table.insert(fileList, {name = "..", isDirectory = true})
  end

  -- Stealy stealing code from original openfile.lua
  table.sort(fileList, function(i, j)
    if i.isDirectory and not j.isDirectory then
      return true
    elseif i.isDirectory == j.isDirectory then
      return string.lower(i.name) < string.lower(j.name)
    end
  end)

  return fileList
end

local function getBinding(selectedFile, bindings)
	if selectedFile.isDirectory then
		return bindings.__directory, "__directory"
	end
  for pattern, values in pairs(bindings) do
    if selectedFile.name:match(pattern) then
      return values, pattern
    end
  end
  return bindings.__default, "__default"
end

local function drawBottom(externalConfig, workingDirectoryScroll, selected)
	local workingDirectory = externalConfig.workingDirectory
	local bindings = externalConfig.bindings
	local selectedFile = externalConfig.fileList[selected.inList]
	gfx.start(gfx.BOTTOM)
		gfx.rectangle(0, 0, gfx.BOTTOM_WIDTH, 16, 0, 0xFF0000B3)
		gfx.text(1 - workingDirectoryScroll.value, 0, workingDirectory)
		if gfx.font.getDefault():width(workingDirectory) > gfx.BOTTOM_WIDTH - 2 then
			workingDirectoryScroll.value = workingDirectoryScroll.value + workingDirectoryScroll.phase
			if workingDirectoryScroll.value == (gfx.BOTTOM_WIDTH - 2) - gfx.font.getDefault():width(workingDirectory) or
			workingDirectoryScroll.value == 0 then
				workingDirectoryScroll.phase = - workingDirectoryScroll.phase
			end
		end

		gfx.text(1, 15, selectedFile.name, 12)
		if not selectedFile.isDirectory then
			gfx.text(1, 45, tostring(selectedFile.size) .. "B", 12, 0xFF727272)
		end

		local binding, pattern = getBinding(selectedFile, bindings)
		if selectedFile.isDirectory then
			gfx.text(1, 30, bindings.__directory.__name, 12, 0xFF727272)
		else
			gfx.text(1, 30, binding.__name, 12, 0xFF727272)
		end

		local bindingNames = {
			{"start", "Start"},         {"select", "Select"},
			{"x", "X"},                 {"y", "Y"},
			{"b", "B"},                 {"a", "A"},
			{"r", "R"},                 {"l", "L"},
			{"zr", "ZR"},               {"zl", "ZL"},
			{"cstickDown", "C Down"},   {"cstickUp", "C Up"},
			{"cstickRight", "C Right"}, {"cstickLeft", "C Left"}
		}
		
		local j = 0

		for i, v in ipairs(bindingNames) do
			if binding[v[1]] and binding[v[1]][2] then
				j = j + 1
				gfx.text(1, gfx.BOTTOM_HEIGHT - 15*j, v[2] .. ": " .. binding[v[1]][2])
			end
		end

		externalConfig.callbacks.drawBottom(externalConfig, selected)
	gfx.stop()
end

local function drawTop(externalConfig, selected)
  gfx.start(gfx.TOP)
    gfx.rectangle(0, (selected.inList-selected.offset-1)*15, gfx.TOP_WIDTH, 16, 0, 0xFF0000B9)
    local over = #externalConfig.fileList - selected.offset >= 16 and 16 or #externalConfig.fileList - selected.offset
    for i=selected.offset+1, selected.offset+over do
      local color = externalConfig.fileList[i].isDirectory and 0xFF727272 or 0xFFFDFDFD
      gfx.text(1, (i-selected.offset-1)*15+1, externalConfig.fileList[i].name or "", 12, color)
    end
    externalConfig.callbacks.drawTop(externalConfig, selected)
  gfx.stop()
end

local function eventHandler(externalConfig, selected)
	externalConfig.callbacks.eventHandler(externalConfig, selected)
	ctr.hid.read()
	local state = ctr.hid.keys()
	local binding, pattern = getBinding(externalConfig.fileList[selected.inList], externalConfig.bindings)
	for k, v in pairs(binding) do
		if k ~= "__name" and state.down[k] then
			local a, b, c, key = v[1](externalConfig, selected, pattern, k)
			if key then return a, b, c, key 
			else return end
		end
	end
	for k, v in pairs(externalConfig.bindings.__default) do
		if k ~= "__name" and state.down[k] then
			local a, b, c, key = v[1](externalConfig, selected, pattern, k)
			if key then return a, b, c, key 
			else break end
		end
	end
end

local function nothing(externalConfig, selected, bindingName, bindingKey)
	-- externalConfig = {workingDirectory=string, bindings=table, callbacks=table, additionalArguments=table, fileList=table}
	-- selected = {file=string, inList=number, offset=number}
	-- bindings = {__default/__directory/[regex] = {__name, [keyName] = {(handlingFunction), (name)}}}
	-- callbacks = {drawTop, drawBottom, eventHandler}
end

local function changeDirectory(externalConfig, selected, bindingName, bindingKey)
	if externalConfig.fileList[selected.inList].isDirectory then
		if externalConfig.fileList[selected.inList].name == ".." then
			externalConfig.workingDirectory = externalConfig.workingDirectory:gsub("[^/]+/$", "")
		else
			externalConfig.workingDirectory = externalConfig.workingDirectory .. externalConfig.fileList[selected.inList].name .. "/"
		end
		externalConfig.fileList = getFileList(externalConfig.workingDirectory)
		selected.inList, selected.offset = 1, 0
	end
end

local function newFile(externalConfig, selected, bindingName, bindingKey)
	local name = ""
	while ctr.run() do
		gfx.start(gfx.BOTTOM)
			gfx.rectangle(0, 0, gfx.BOTTOM_WIDTH, 16, 0, 0xFF0000B3)
			gfx.text(1, 0, externalConfig.workingDirectory)
			keyboard.draw(4, 115)
		gfx.stop()

		gfx.start(gfx.TOP)
			gfx.rectangle(0, 0, gfx.TOP_WIDTH, 16, 0, 0xFF0000B3)
			gfx.text(1, 0, "Creating new file")
			gfx.rectangle(4, gfx.TOP_HEIGHT // 2 - 15, gfx.TOP_WIDTH - 8, 30, 0, 0xFF727272)
			gfx.text(5, gfx.TOP_HEIGHT // 2 - 6, name, 12)
		gfx.stop()
		gfx.render()

		local char = (keyboard.read() or ""):gsub("[\t%/%?%<%>%\\%:%*%|%”%^]", "")
		ctr.hid.read()
		local keys = ctr.hid.keys()

		if char ~= "" and char ~= "\b" and char ~= "\n" then
			name = name .. char
		elseif char ~= "" and char == "\b" then
			name = name:sub(1, (utf8.offset(name, -1) or 0)-1)
		elseif (char ~= "" and char == "\n" or keys.down.a) and name ~= "" then
			local b, p = getBinding({name=name}, externalConfig.bindings)
			return externalConfig.workingDirectory .. name, p, "new", b
		elseif keys.down.b then
			break
		end
	end
end

local function openFile(externalConfig, selected, bindingName, bindingKey)
	return externalConfig.workingDirectory .. externalConfig.fileList[selected.inList].name, 
		bindingName, "open", bindingKey
end

local function filePicker(workingDirectory, bindings, callbacks, ...)
	-- Argument sanitization
	local additionalArguments = { ... }
	workingDirectory = workingDirectory or ctr.fs.getDirectory()
	bindings = bindings or {}
	callbacks = callbacks or {}
	for _, v in ipairs({"drawTop", "drawBottom", "eventHandler"}) do
		if not callbacks[v] then
			callbacks[v] = function(...) end
		end
	end

	if workingDirectory:sub(utf8.offset(workingDirectory, -1) or -1) ~= "/" then
		workingDirectory = workingDirectory .. "/"
	end

	-- Default Bindings
	bindings.__default = bindings.__default or {}
	bindings.__default.__name = bindings.__default.__name or "File"
	bindings.__default.x = bindings.__default.x or { 
		function(externalConfig, ...)
			return externalConfig.workingDirectory, "__directory", nil, "x"
		end, "Quit"
	}

	bindings.__directory = bindings.__directory or {}
	bindings.__directory.__name = bindings.__directory.__name or "Directory"
	bindings.__directory.a = bindings.__directory.a or {
		changeDirectory, "Open"
	}

	local movementKeys = {
		"up"    , "down"    , "left"    , "right"    ,
		"cpadUp", "cpadDown", "cpadLeft", "cpadRight",
		"dUp"   , "dDown"   , "dLeft"   , "dRight"
	}

	for k, v in pairs(bindings) do
		if k ~= "__default" then
			setmetatable(bindings[k], {__index = bindings.__default})
		end

		for _, w in ipairs(movementKeys) do
			if v[w] then bindings[k][w] = nil end
		end
	end

	systemBindings(bindings)

	-- Other Initialization
	local selected = {inList = 1, offset = 0}
	local workingDirectoryScroll = { value = 0, phase = -1 }
	local gfxState = gfxPrepare()

	-- Main Loop
	externalConfig = {workingDirectory=workingDirectory, bindings=bindings,
			callbacks=callbacks, additionalArguments=additionalArguments, 
			fileList=getFileList(workingDirectory)}
	while ctr.run() do
		drawBottom(externalConfig, workingDirectoryScroll, selected)
		drawTop(externalConfig, selected)
		gfx.render()

		local file, binding, mode, key = eventHandler(externalConfig, selected)

		if key then
			gfxRestore(gfxState)
			return file, binding, mode, key
		end
	end
end

local returnTable = {filePicker = filePicker, openFile = openFile, 
	newFile = newFile, changeDirectory = changeDirectory}
setmetatable(returnTable, {__call = function(self, ...) return self.filePicker(...) end})

return returnTable