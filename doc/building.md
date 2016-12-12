# Building and Consuming Lua Vermelha

The source code for each of these components is in a different sub-directory:

- `lua/` contains the modified Lua VM
- `omr/` is a git submodule containing the source code for Eclipse OMR and JitBuilder
- `vermelha/` contains the JIT compiler

The build system for Lua Vermelha was designed to support different consumption
strategies. The following explains how to build each component.

*Please note that Lua Vermelha is currently only tested and supported on 64-bit Linux.*

## JitBuilder

Being part of the Eclipse OMR project, JitBuilder can be used as is. Currently,
only building it as a static library (archive) is supported. To build it, run
the following from the Lua Vermelha project directory:

```sh
$ cd omr
$ make -f run_configure.mk SPEC=linux_x86-64 OMRGLUE=./example/glue
$ cd jitbuilder
$ make -j4 PLATFORM=amd64-linux64-gcc
```

This will produce the JitBuilder static library `release/libjitbuilder.a`.
It will also copy the JitBuilder development headers into `release/include/`.

## JIT compiler

For flexibility, the JIT can be built into a few different kinds of binaries.

The default and recommended way of building the JIT is as a static library.
It can then either be linked into the VM library or as part of a larger
application.

Another currently supported strategy is to link the JIT as an object file.
In this case, the intermediate object files are linked into a single
(large) object file. This file can then be linked as any other object
file would be.

Currently, the JIT must be linked directly into the VM (regardless of the
binary used), so building it as a shared library is not supported.

The following phony targets are provided by the JIT Makefile to build the
different binaries:

* `staticlib` (`default`) to build the JIT as a static archive
* `vermelhaobj` to build the JIT as an object file

When building the JIT, the `LUA_C_LINKAGE` macro must be defined if the VM
is compiled as C code. This ensures that no name-mangling issues occur when
linking the JIT and the VM together. This macro can be defined when invoking
`make` by specifying `CXX_FLAGS_EXTRA='-DLUA_C_LINKAGE'` (see top level
Makefile for an example).

The following table lists the `make` configuration variables that can be
specified when building the JIT:

| Variable             | Default Value     | Description            |
|:--------------------:|:-----------------:|:-----------------------|
| `CXX`                | `g++`             | The C++ build compiler |
| `AR`                 | `ar`              | The archive generator  |
| `LD`                 | `ld`              | The linker             |
| `CXX_FLAGS_EXTRA`    | (empty)           | Extra flags to pass to the C++ compiler |
| `LD_FLAGS_EXTRA`     | (empty)           | Extra flags to pass to the linker |
| `ARCH`               | `x` (for x86)     | Main architecture      |
| `SUBARCH`            | `amd64`           | Sub-architecture       |
| `STATICLIB_NAME`     | `libvermelha.a`   | Name of generated JIT static library |
| `VERMELHAOBJ_NAME`   | `vermelha.o`      | Name of generated JIT object file |
| `BIN_DIR`            | `$(PWD)`          | Target directory for generated binaries |
| `STATICLIB_DEST`     | `$(BIN_DIR)`      | Target directory for generated JIT static library |
| `VERMELHAOBJ_DEST`   | `$(BIN_DIR)`      | Target directory for generated JIT object file |
| `OBJS_DIR`           | `$(BIN_DIR)/objs` | Target directory for intermediate object files |
| `LUA_DIR`            | `../`             | Directory containing the Lua VM directory |
| `OMR_DIR`            | `../omr`          | Directory containing Eclipse OMR source code |
| `JITBUILDER_LIB_DIR` | (see bellow)      | Directory containing the JitBuilder binary |

Other variables are provided for maximum flexibility, though it is unlikely
any project will need to use them:

| Variable                   | Default Value                         | Description   |
|:--------------------------:|:-------------------------------------:|:--------------|
| `JITBUILDER_DIR`           | `$(OMR_DIR)/jitbuilder`               | Directory containing JitBuilder source code |
| `JITBUILDER_ARCH_DIR`      | `$(JITBUILDER_DIR)/$(ARCH)`           | Directory containing architecture specific JitBuilder source code |
| `JITBUILDER_SUBARCH_DIR`   | `$(JITBUILDER_ARCH_DIR)/$(SUBARCH)`   | Directory containing sub-architecture specific JitBuilder source code |
| `JITBUILDER_LIB_DIR`       | `$(JITBUILDER_DIR)/release`           | (see above)   |
| `JITBUILDER_HEADERS`       | `$(JITBUILDER_LIB_DIR)/include`       | Directory containing the JitBuilder headers |
| `OMR_COMPILER_DIR`         | `$(OMR_DIR)/compiler`                 | Directory containing Eclipse OMR compiler source code |
| `OMR_COMPILER_ARCH_DIR`    | `$(OMR_COMPILER_DIR)/$(ARCH)`         | Directory containing architecture specific Eclipse OMR compiler source code
| `OMR_COMPILER_SUBARCH_DIR` | `$(OMR_COMPILER_ARCH_DIR)/$(SUBARCH)` | Directory containing sub-architecture specific Eclipse OMR compiler source code

## Lua VM

The build system for the Lua VM is mostly unchanged and should work
the same way as in PUC-Rio Lua. Building the VM static library can be
done using:

```sh
$ cd lua
$ make liblua.a SYSCFLAGS="-DLUA_USE_LINUX" 
```

## Putting it all together

Once the three components have bee built separately, they can be linked
into an application. It's important that linking be done either by a
C++ compiler or by explicitly linking the C++ standard library.

Alternatively, the JIT and JitBuilder can be linked together (once again
by a C++ compiler) and then linked into a C application.

A simple linking command can be (assuming there is a `lua.o`):

```sh
$ g++ lua/lua.o lua/liblua.a vermelha/libvermelha.a omr/jitbuilder/release/libjitbuilder.a -ldl -lm -Wl,-E -lreadline
```

## Complete examples

The following are two examples of how to built `luav`. Note that these
commands accomplish essentially the same things as simply calling the
top level make file.

With the JIT as a static library:

```sh
$ lua
$ make -j4 lua.o SYSCFLAGS="-DLUA_USE_LINUX"
$ make -j4 liblua.a SYSCFLAGS="-DLUA_USE_LINUX" MYCFLAGS="-O2 -g"
$ cd ../omr
$ make -f run_configure.mk SPEC=linux_x86-64 OMRGLUE=./example/glue
$ cd jitbuilder
$ make -j4 CXX="g++" PLATFORM=amd64-linux64-gcc
$ cd ../../vermelha
$ make -j4 staticlib CXX="g++" CXX_FLAGS_EXTRA="-fpermissive -DLUA_C_LINKAGE -O3 -g"
$ cd ../
$ g++ lua/lua.o lua/liblua.a vermelha/libvermelha.a omr/jitbuilder/release/libjitbuilder.a -ldl -lm -Wl,-E -lreadline
```

Alternatively, with the JIT as an object file

```sh
$ lua
$ make -j4 lua.o SYSCFLAGS="-DLUA_USE_LINUX"
$ make -j4 liblua.a SYSCFLAGS="-DLUA_USE_LINUX" MYCFLAGS="-O2 -g"
$ cd ../omr
$ make -f run_configure.mk SPEC=linux_x86-64 OMRGLUE=./example/glue
$ cd jitbuilder
$ make -j4 CXX="g++" PLATFORM=amd64-linux64-gcc
$ cd ../../vermelha
$ make -j4 vermelhaobj CXX="g++" CXX_FLAGS_EXTRA="-fpermissive -DLUA_C_LINKAGE -O3 -g"
$ cd ../
$ g++ lua/lua.o lua/liblua.a vermelha/vermelha.o omr/jitbuilder/release/libjitbuilder.a -ldl -lm -Wl,-E -lreadline
```

## Top-level Makefile

The top-level Makefile automates the `luav` build process described above with the
exception of configuring OMR. This step must still be done manually by the user.

A build can be customized by specifying the following variables:

| Variable         | Default Value           | Description            |
|:----------------:|:-----------------------:|:-----------------------|
| `CC`             | `gcc`                   | The C build compiler   |
| `CXX`            | `g++`                   | The C++ build compiler |
| `BUILD_CONFIG`   | `opt`                   | Can be one of `debug`, `opt`, and `prod` |
| `LUAV`           | `luav`                  | Name of the final application binary     |
| `LUA_APP`        | `lua.o`                 | Name of application frontend object file |
| `LUA_LIB`        | `liblua.a`              | Name of Lua VM static library            |
| `VERMELHA_LIB`   | `libvermelha.a`         | Name of Lua Vermelha JIT static library  |
| `JITBUILDER_LIB` | `libjitbuilder.a`       | Name of JitBuilder static library        |
| `LUA_DIR`        | `$(PWD)/lua`            | Directory containing Lua VM source code  |
| `VERMELHA_DIR`   | `$(PWD)/vermelha`       | Directory containing JIT source code     |
| `JITBUILDER_DIR` | `$(PWD)/omr/jitbuilder` | Directory containing JitBuilder source code |

The possible values of `BUILD_CONFIG` specify the kind of built to produce:

* `debug` builds with debug symbols and no optimizations
* `opt` builds with debug symbols and optimizations enabled
* `prod` builds with no debug symbols and optimizations enabled

The following will build `luav` with the JIT as a static library:

```sh
$ make -j4
```

The following produce a debug build of `luav` with the JIT as an object file:

```sh
$ make -j4 VERMELHA_LIB=vermelha.o BUILD_CONFIG=debug
```
