# Lua Vermelha

*Please note that Lua Vermelha is under active development and is not ready for a production environment.*

## Introduction

Lua Vermelha is designed to behave the same way as PUC-Rio Lua. Any project
that uses the PUC-Rio Lua as a library can also use Lua Vermelha. The main
benefit it provides is improved performance, thanks to the JIT compiler, at
the cost of a larger footprint.

The JIT compiler is built using Eclipse OMR JitBuilder. JitBuilder is a
framework that simplifies the creation of JIT compilers by leveraging
the Eclipse OMR compiler technology. See the
[Eclipse OMR GitHub page](https://github.com/eclipse/omr) for more details
about the project.

## Architecture overview

There are three main components that make up Lua Vermelha:

1. The minimally modified PUC-Rio Lua virtual machine
2. The Eclipse OMR JitBuilder library
3. The JIT compiler itself

### Lua VM

The Lua VM provides the core functionality of the language implementation.
It includes major components such as the interpreter and the garbage
collector. The minimal changes done to it implement the necessary mechanism
for dispatching the JIT compiler.

When the Lua VM is instantiated, it initializes the JIT and starts the
interpreter. When the interpreter encounters a function call, it dispatches
the JIT, which will attempt to compile the function. If the compilation is
successful, the interpreter calls the compiled function body. Subsequent 
calls to the function will also dispatch the JITed code. Otherwise,
the function is simply interpreted and execution proceeds normally.

This is by no means an optimal approach to JIT dispatching. However, it
makes it easier to find and debug problems in the JIT.

#### Lua patch file

For convenience, this repo contains a git branch called `luavm` which contains
the unchanged PUC-Rio Lua VM code. Each commit on this branch is tagged with
the version of Lua it contains.

The top-level Makefile on the `master` and `devel` branches has a special
phony target called `lua-patch`. It can be used to generate a git patch file
with the changes done applied to the VM between two commits. By default,
it will use the current `HEAD` and `luavm` commits. Running

```sh
$ make lua-patch
```

will generated a patch file called `luavm-HEAD.patch`. The `LUA` variable
can be set to point to whatever commit contains the unchanged Lua code.
Likewise, the `COMMIT` variable can be set to point to whatever commit
contains the modified Lua VM code.

### Eclipse OMR JitBuilder

The [Eclipse OMR Project](http://www.eclipse.org/omr) provides a collection
of reusable cross-platform components for building language runtimes. One
of these components is the compiler technology, which can be used to create
JIT compilers.

Although the compiler technology is very powerful and flexible, it is also
very complex. The Eclipse OMR project therefore also provides a component
called JitBuilder.

JitBuilder is a framework that simplifies leveraging the Eclipse OMR
compiler to build JITs. It reduces boiler plated code by provide sane
default implementations for major tasks such as code optimization and
code generation. It also provides a declarative interface for generating
the Eclipse OMR Compiler intermediate language (IL).

The JIT compiler in Lua Vermelha is built using JitBuilder. 

### Lua Vermelha JIT

The JIT compiler is the major components that makes Lua Vermelha what it is.

When the JIT is invoked by the interpreter, it takes as parameter an
object that contains the bytecode implementation of the function. It then
iterates over each bytecode, generating IL that represents what the
interpreter would do if it encountered the specific bytecode. If IL is
successfully generated for each bytecode, the intermediate representation
of the function is handed over to JitBuilder for optimization and
code generation.

If compilation is successful, JitBuilder will cache the compiled code
in executable memory and return a pointer to it. The interpreter can
then jump to the compiled body instead of interpreting the function.

## luav executable front end

The top level `Makefile` can be used to create an executable Lua VM. It
simply builds JitBuilder, builds the JIT (as a static library), builds
the VM (also as a static library), builds the frontend, and links them
all together. The VM and the frontend are built with `gcc` while JitBuilder
and the JIT itself are compiled with `g++`. Linking is also done by `g++`.
This Makefile serves as an example of how to build and consume Lua Vermelha.