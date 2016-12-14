# Lua Vermelha

Lua Vermelha is an implementation of [Lua](http://www.lua.org) 5.3 with a
Just-In-Time (JIT) compiler built from
[Eclipse OMR](https://github.com/eclipse/omr) compiler technology.

It is designed to integrate into the [PUC-Rio Lua](http://www.lua.org) virtual
machine with only minor modifications done to it.

*(Please note that Lua Vermelha is under active development and may be unstable and definitely is buggy)*

## Cloning

Lua Vermelha includes the Eclipse OMR project as a submodule so I recommend cloning
this repo recursively:

```shell
$ git clone --recursive https://github.com/Leonardo2718/lua-vermelha.git
```

## Building

Currently, the only tested and supported platform is Linux on x86-64.
Support for other platforms will come later on. Until then, if you need to
build Lua Vermelha on a different platform, you should be able to easily
modify the makefiles manually to suit your needs.

**IMPORTANT**: Before JitBuilder can be built (and by extension Lua Vermelha),
it's important to configure OMR for your platform (see the
[Eclipse OMR project page](https://github.com/eclipse/omr) for details):

```shell
$ cd lua-vermelha/omr
$ make -f run_configure.mk SPEC=linux_x86-64 OMRGLUE=./example/glue
```

The top level Makefile can then be used to build the Lua Vermelha executable
interpreter frontend `luav`.

```shell
$ cd ..     # back to `lua-vermelha/`
$ make -j4  # will recursively build JitBuilder, the JIT, the VM, and luav
```

## Running

The `luav` executable can be used the same way as the PUC-Rio Lua
interpreter frontend (usually `lua`):

```shell
$ luav           # starts REPL
$ luav file.lua  # executes the Lua script `file.lua`
$ luav luac.out  # executes compiled Lua bytecode (output of `luac`)
```

## Licensing

Lua Vermelha has three components, each with their own licensing: 

* The (slightly) modified PUC-Rio Lua VM keeps its original license (MIT)
and copy-right holders
* The Lua Vermelha JIT compiler is distributed under two licenses:
  * [Eclipse Public License](http://www.eclipse.org/legal/epl-v10.html)
  * [Apache License v2.0](http://www.opensource.org/licenses/apache2.0.php)
* The Eclipse OMR component is provided as a submodule pointing to the main
project repository and is under the same licenses as the Lua Vermelha JIT
