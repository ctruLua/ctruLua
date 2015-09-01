local gfx = require("ctr.gfx")
local ctr = require("ctr")
local mod = {}

-- Module functions
local function getBox(tx, ty, i, sx, sy)
	x = (i%tx)
	y = math.floor(i/tx)

	return (x*sx), (y*sy)
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
	self.texture:drawPart(x, y, sx, sy, self.frameSizeX, self.frameSizeY, rad)

	return frame
end

local function addAnimation(self, anim, delay)
	self.animations[#self.animations+1] = {animation=anim, delay=delay}
	return #self.animations
end

-- Set to 0 to hide the sprite
local function setAnimation(self, anim)
	self.currentAnimation = anim
	self.currentFrame = 1
	if not self.animations[anim] then
		return false
	end
	return true
end

local function resetTimer(self)
	self.frameTimer = ctr.time()
end

-- Sprite object constructor
function mod.new(texture, fsx, fsy)
	return {
		texture = texture,
		frameSizeX = fsx,
		frameSizeY = fsy,
		animations = {},
		currentAnimation = 0,
		currentFrame = 1,
		frameTimer = 0,

		draw = draw,
		addAnimation = addAnimation,
		setAnimation = setAnimation,
		resetTimer = resetTimer,
	}
end

return mod
