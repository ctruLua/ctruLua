local gfx = require("ctr.gfx")

while os.run() do
	gfx.rectangle(10, 10, 56, 120)
	gfx.rectangle(240, 150, 120, 10)

	gfx.render()
end