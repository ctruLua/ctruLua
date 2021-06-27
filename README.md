# ctrµLua
![banner](https://www.dropbox.com/s/cqmtoohyx6t7q7c/banner.png?raw=1)

Warning: the 'u' in the repo's name is a 'µ', not a 'u'.

### Users part

#### How to install

* Download ctruLua.zip from the [releases](https://github.com/ctruLua/ctruLua/releases) (for something stable) or the [CI server](https://reuh.eu/ctrulua/ci/ctrulua) (for more features)
* Unzip it on your SD card, in a folder inside your homebrews folder; if you use HBL, extract it in `/3ds/ctrulua/`
* Put some scripts wherever you want, just remember where
* Launch CtrµLua from your homebrew launcher
* Use the shell to run your scripts

### Homebrewers part

#### Builds ![build status](https://reuh.eu/ctrulua/ci/ctrulua.png)

* Most recent working build: [ctruLua.3dsx](https://reuh.eu/ctrulua/ci/ctrulua/builds/latest/artifacts/ctruLua.3dsx)
* See [https://reuh.eu/ctrulua/ci/ctrulua](https://reuh.eu/ctrulua/ci/ctrulua) for all the builds.

#### Hello world

```Lua
local ctr = require("ctr")
local gfx = require("ctr.gfx")
local hid = require("ctr.hid")

while ctr.run() do
	hid.read()
	local keys = hid.keys()
	if keys.held.start then break end
	
	gfx.start(gfx.TOP)
		gfx.text(2, 2, "Hello, world !")
	gfx.stop()
	
	gfx.render()
end
```
This script will print "Hello, world !" on the top screen, and will exit if the user presses Start.
This is the "graphical" version; there's also a text-only version, based on the console:
```Lua
local ctr = require("ctr")
local gfx = require("ctr.gfx")
local hid = require("ctr.hid")

gfx.console()
print("Hello, world !")

while ctr.run() do
	hid.read()
	local keys = hid.keys()
	if keys.held.start then break end
	
	gfx.render()
end

gfx.disableConsole()
```

#### Lua API Documentation

* An online version of the documentation can be found [here](https://reuh.eu/ctrulua/latest/html/)
* To build the documentation, run `make build-doc-html` (requires [LDoc](https://github.com/stevedonovan/LDoc)).

### Developers part

#### Build instructions

* Setup your environment as shown here : http://3dbrew.org/wiki/Setting_up_Development_Environment
* Clone this repository and run the command `make build-all` to build all the dependencies.
* If you only made changes to ctrµLua, run `make` to rebuild ctrµLua without rebuilding all the dependencies.

May not work under Windows.

### Credits

* __Smealum__ and everyone who worked on the ctrulib: [https://github.com/smealum/ctrulib](https://github.com/smealum/ctrulib)
* __Xerpi__ for the [sf2dlib](https://github.com/xerpi/sf2dlib), [sftdlib](https://github.com/xerpi/sftdlib) and [sfillib](https://github.com/xerpi/sfillib)
* __All the [Citra](https://citra-emu.org/) developers__
* __Everyone who worked on [DevKitARM](http://devkitpro.org/)__
* __Nothings__ for the [stb](https://github.com/nothings/stb) libs
* Everyone who worked on the other libs we use
 
