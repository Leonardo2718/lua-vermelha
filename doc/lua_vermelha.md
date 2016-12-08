# Lua Vermelha

*Please note that Lua Vermelha is under active development and may not be ready for a production environment.*

## Introduction

Lua Vermelha is designed to behave the same way as PUC-Rio Lua. It's intended to
be used seamlessly in place of PUC-Rio Lua. The main benefit it provides is 
improved performance at the cost of slightly larger footprint.

Projects that consume the Lua VM as a library can also consume Lua Vermelha. The
build system should be flexible enough to support most use cases with
minimal changes.

## Architecture overview

There are three main components that make up Lua Vermelha:

1. The minimally modified PUC-Rio Lua virtual machine
2. The Eclipse OMR JitBuilder library used to create the JIT compiler
3. The JIT compiler itself

The Lua VM provides the core functionality of the language implementation. The
minimal changes done on it implement the necessary mechanism for dispatching
the JIT compiler. Consumption of the Eclipse OMR compiler technology is
encapsulated in JitBuilder, which is in turn consumed by the JIT.

## Building and Consuming

The source code for each of these components is in a different sub-directory:

- `lua/` contains the modified Lua VM
- `omr/` is a git submodule containing the source code for Eclipse OMR and JitBuilder
- `vermelha/` contains the JIT compiler

The build system for Lua Vermelha was designed to support different consumption
strategies. The following explains how to build the different components.

### JitBuilder

Being part of the Eclipse OMR project, JitBuilder can be used as is. Currently,
only building it as a static library (archive) is supported. See the Eclipse
OMR project page for details.

### JIT compiler

The default and recommended way of building the JIT is as a static library.
It can then either be linked into the VM library or as part of the final
application.

Another currently supported strategy is to link the JIT as an object file.
In this case, the intermediate object files are linked into a single
(large) object file, which can then be linked as any other object
file would be.

The following phony targets are provided by the JIT Makefile to build the
different binaries:

* `staticlib` (`default`) to build the JIT as a static archive
* `vermelhaobj` to build the JIT as an object file

Currently, the JIT must be linked directly into the VM (regardless of the
binary used), so building it as a shared library is not currently supported.

When building the JIT, the `LUA_C_LINKAGE` macro must be defined if the VM
is compiled as C code. This ensures that no name-mangling issues occur when
linking the JIT and the VM together. This macro can be defined when invoking
`make` by specifying `CXX_FLAGS_EXTRA='-DLUA_C_LINKAGE'` (see top level
Makefile for an example).

The following table lists the configuration variables that can be specified
when calling `make` to build the JIT:

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


### Lua VM

The build system for the Lua VM is mostly unchanged and should work as
the same as that of PUC-Rio Lua.

### Putting it together

Once the three components have bee built separately, they can be linked
into the final application. In this case, it's important that linking be
done either by a C++ compiler or by explicitly linking the C++ standard
library.

Alternatively, the JIT and JitBuilder can be linked together (once again
by a C++ compiler) and then linked into a C application.

## Building the Standard Lua VM

The top level Makefile can be used to create an executable Lua VM. It
simply builds JitBuilder, builds the JIT (as a static library), builds
the VM (also as a static library), and links them all together. The VM
is built with gcc and the final linking is done with g++. This Makefile
can be used as an example of how to link Lua Vermelha into other projects.