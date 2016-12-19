# Lua Vermelha JIT API

Lua Vermelha provides a special API that makes some JIT controls
accessible to user code. This API has two components: the C API and
the Lua library. Both offer the same functionality.

## JIT flags

Lua Vermelha defines some "JIT flags" that can be used to "annotate" Lua
functions. These flags modify how the VM controls the JIT with regards to
the annotated functions. The following table describes the currently
supported flags:

| Flag Name        | Description                              |
|:-----------------|:-----------------------------------------|
| `LUA_NOJITFLAGS` | No flags set (default)                   |
| `LUA_BLACKLIST`  | Prevent function from being JIT compiled |

These flags are implemented as bit fields and can be combined using
bit-wise operations.

## C API

Lua Vermelha extends the standard Lua C API that provides some amount
of the control of the JIT to user code. The following list describes
each function provided by this API.

### `int lua_checkjitflags(lua_State* L, int index, int flags)`

Checks if the given JIT flags are set on the function at the given stack
index. The returned value is the result of masking the function's flags
with the specified flags. In pseudo code `return function.flags & flags`.

### `void lua_setjitflags(lua_State* L, int index, int flags)`

Sets the given flags on the function at the given stack index. Does not
change the state (set or unset) of flags that are not specified.

### `void lua_clearjitflags(lua_State* L, int index, int flags)`

Clears (unsets) the given flags on the function at the given stack index.
Does not change the state (set or unset) of flags that are not specified.

### `int lua_initcallcounter(lua_State* L)`

Returns the number of times a function must be called by the interpreter
before it will attempt to JIT compile the function.

### `int lua_iscompiled(lua_State* L, int index)`

Returns 1 if the function at the given stack index has been compiled (i.e.
if a compiled body exists for it), 0 otherwise.

### `int lua_compile(lua_State* L, int index)`

Will attempt to compile the function at the given stack index. Returns 1,
if the compilation is successful, 0 otherwise.

Note that this function ignores all JIT flags. That is, it will attempt
to compile a function even if its `LUA_BLACKLIST` flag is set.

## Lua API

The Lua API provides the same functionality as the C API. It is packaged
in a library module called `lvjit`. The following table maps functions
in the `lvjit` module to the corresponding C API functions.

| lvjit                   | C API                 |
|:------------------------|:----------------------|
| `lvjit.checkjitflags`   | `lua_checkjitflags`   |
| `lvjit.setjitflags`     | `lua_setjitflags`     |
| `lvjit.clearjitflags`   | `lua_clearjitflags`   |
| `lvjit.initcallcounter` | `lua_initcallcounter` |
| `lvjit.iscompiled`      | `lua_iscompiled`      |
| `lvjit.compile`         | `lua_compile`         |
