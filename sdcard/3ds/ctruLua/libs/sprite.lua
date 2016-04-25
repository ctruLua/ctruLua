local gfx = require("ctr.gfx")
local ctr = require("ctr")
local mod = {}

-- Module functions
local function getBox(tx, ty, i, sx, sy)
	x = ((i*sx)%tx)
	y = math.floor((i*sx)/tx)*sy

	return x, y
end

-- Sprite object methods
local function draw(self, x, y, rad)
	if not self.animations[self.currentAnimation] then return 0 end

	if (ctr.time()-self.frameTimer) >= self.animations[self.currentAnimation].delay then
		self.currentFrame = (self.currentFrame+1)
		self.frameTimer = ctr.time()
		if self.currentFrame > #self.animations[self.currentAnimation].animation then
			self.currentFrame = 1
		end
	end

	local frame = self.animations[self.currentAnimation].animation[self.currentFrame]
	local tsx, tsy = self.texture:getSize()

	local sx, sy = getBox(tsx, tsy, frame, self.frameSizeX, self.frameSizeY)
	self.texture:drawPart(x, y, sx, sy, self.frameSizeX, self.frameSizeY, rad, self.offsetX, self.offsetY)

	return frame
end

local function addAnimation(self, anim, delay)
	self.animations[#self.animations+1] = {animation=anim, delay=delay}
	return #self.animations
end

-- Set to 0 to hide the sprite
local function setAnimation(self, anim)
	self.currentAnimation = anim
	if not self.animations[anim] then
		return false
	end
	if not self.animations[anim].animation[self.currentFrame] then
		self.currentFrame = 1
	end
	return true
end

local function resetTimer(self)
	self.frameTimer = ctr.time()
end

local function setOffset(self, x, y)
	self.offsetX = x or 0
	self.offsetY = y or self.offsetX
end

-- Sprite object constructor
function mod.new(texture, fsx, fsy)
	return {
		texture = texture,
		frameSizeX = fsx,
		frameSizeY = fsy,
		offsetX = 0,
		offsetY = 0,
		animations = {},
		currentAnimation = 0,
		currentFrame = 1,
		frameTimer = ctr.time(),

		draw = draw,
		addAnimation = addAnimation,
		setAnimation = setAnimation,
		setOffset = setOffset,
		resetTimer = resetTimer,
	}
end

return mod
