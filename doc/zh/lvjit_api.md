# Lua Vermelha JIT API

Lua Vermelha 提供了一个特有的API使得用户代码可以容易地控制JIT。API有两个组件：C语言API和Lua库。
这两者都提供了相同的功能。

## JIT 标志

Lua Vermelha 定义了一些 “JIT标志”，可以用来标注Lua函数。这些标志能修改VM如何控制JIT来处理标注函数的方式。下表说明了当前支持的标志：

| 标志名称          | 说明                              |
|:-----------------|:-----------------------------------------|
| `LUA_NOJITFLAGS` | 没有标志 (默认值)                   |
| `LUA_BLACKLIST`  | 不允许函数被JIT编译 |

这些标志用bit字段实现，可以用bit操作符来组合使用。

## C语言 API

Lua Vermelha 扩展了标准的Lua C语言API，为用户代码提供了很多控制JIT的方法。
下面详细说明API提供的每个函数。

### `int lua_checkjitflags(lua_State* L, int index, int flags)`

检查指定栈上的函数是否设置了JIT标志。返回结果是函数标志和参数标志的掩码。
伪代码就是：`return function.flags & flags`。

### `void lua_setjitflags(lua_State* L, int index, int flags)`

给指定栈上的函数设置JIT标志。其他的没有指定的标志不改变原有的状态。

### `void lua_clearjitflags(lua_State* L, int index, int flags)`

清除指定栈上的指定JIT标志。其他的没有指定的标志不会改变原有的状态。

### `int lua_initcallcounter(lua_State* L)`

返回函数在被解释器试图调用JIT编译之前需要被调用的次数。

### `int lua_iscompiled(lua_State* L, int index)`

如果指定栈上的代码被编译过（例如：存在被编译过的代码）就返回 1 ，否则返回 0 。

### `int lua_compile(lua_State* L, int index)`

开始编译指定栈上的函数代码。编译成功返回 1 ， 否则返回 0 。

注意：这个函数忽略所有的JIT标志。也就是说，即使 设置了`LUA_BLACKLIST`，它仍然会编译该函数。

## Lua API

Lua API提供了和C语言API一样的功能。它打包成一个名叫 `lvjit` 的库里面。
下表说明了 `lvjit` 模块里面各个函数和C语言API函数的对应关系。

| lvjit                   | C API                 |
|:------------------------|:----------------------|
| `lvjit.checkjitflags`   | `lua_checkjitflags`   |
| `lvjit.setjitflags`     | `lua_setjitflags`     |
| `lvjit.clearjitflags`   | `lua_clearjitflags`   |
| `lvjit.initcallcounter` | `lua_initcallcounter` |
| `lvjit.iscompiled`      | `lua_iscompiled`      |
| `lvjit.compile`         | `lua_compile`         |
