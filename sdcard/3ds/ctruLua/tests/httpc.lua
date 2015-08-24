local ctr = require("ctr")
local gfx = require("ctr.gfx")
local hid = require("ctr.hid")
local httpc = require("ctr.httpc")

local err = 0

--assert(httpc.init())

local context = assert(httpc.context())

assert(context:open("http://firew0lf.github.io/"))
assert(context:beginRequest())

local data = assert(context:downloadData())

while ctr.run() do
  hid.read()
  keys = hid.keys()
  if keys.held.start then break end
  if keys.down.b then
    assert(context:open("http://firew0lf.github.io/"))  
    assert(context:beginRequest())
    data = assert(context:downloadData())
    data = (data.."!")
  end
  
  gfx.startFrame(gfx.GFX_TOP)
    gfx.text(0, 0, data)
  gfx.endFrame()
  
  gfx.render()
end


context:close()
--httpc.shutdown()
