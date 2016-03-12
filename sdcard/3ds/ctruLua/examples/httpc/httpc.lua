local ctr = require("ctr")
local gfx = require("ctr.gfx")
local hid = require("ctr.hid")
local httpc = require("ctr.httpc")

local err = 0
local addr = "https://wtfismyip.com/text"
local dls = 0

local context = assert(httpc.context())

assert(context:open(addr))
assert(context:beginRequest())

local data = assert(context:downloadData())

while ctr.run() do
  hid.read()
  keys = hid.keys()
  if keys.held.start then break end
  if keys.down.b then
    assert(context:open(addr))  
    assert(context:beginRequest())
    data = assert(context:downloadData())
    dls = dls + 1
  end
  
  gfx.start(gfx.TOP)
    gfx.text(0, 0, data)
    gfx.text(0, 20, "Downloaded "..dls.." times.")
  gfx.stop()
  
  gfx.start(gfx.BOTTOM)
  	gfx.text(2, 2, "HTTP Contexts example")
  	gfx.text(2, 20, "The data is downloaded from '"..addr.."'.")
  	gfx.text(2, 30, "Press [B] to redownload.")
  gfx.stop()
  
  gfx.render()
end

context:close()
