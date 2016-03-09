# ctrµLua

Everything is in the Wiki.

Warning: the 'u' in the repo's name is a 'µ', not a 'u'.

#### Builds ![build status](http://ci.reuh.tk/ctrulua.png)

* Most recent working build: [here](http://ci.reuh.tk/ctrulua/builds/latest/artifacts/ctruLua.3dsx) (warning: only tested with Citra, sometimes on real hardware).
* See http://ci.reuh.tk/ctrulua for all the builds.

#### Build instructions

* Setup your environment as shown here : http://3dbrew.org/wiki/Setting_up_Development_Environment
* Clone this repository and run the command `make build-all` to build all the dependencies.
* If you only made changes to ctrµLua, run `make` to rebuild ctµLua without rebuilding all the dependencies.

May not work under Windows.

#### Lua API Documentation

* An online version of the documentation can be found here : http://reuh.tk/ctrulua
* To build the documentation, run `make build-doc` (requires [LDoc](https://github.com/stevedonovan/LDoc)).

#### Based on ctrulib by smealum: [https://github.com/smealum/ctrulib](https://github.com/smealum/ctrulib)
