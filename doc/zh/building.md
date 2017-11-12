# 构建和使用Lua Vermelha

每个组件的代码在不同的子目录里面：
- `lua/` 修改后的 Lua VM
- `omr/` Eclipse OMR and JitBuilder，以git 子模块的形式存在
- `lvjit/` JIT 编译器

Lua Vermelha的构建系统设计成可以支持不懂的使用方式。下面详细解说如何构建每个组件。

*注意：当前Lua Vermelha仅仅在64位Linux下测试过，也只支持64位Linux。*

## JitBuilder

JitBuilder是Eclipse OMR项目的一部分。当前仅支持作为静态库构建。在Lua Vermelha项目根目录运行下列脚本就可以构建JitBuilder。

```sh
$ cd omr
$ make -f run_configure.mk SPEC=linux_x86-64 OMRGLUE=./example/glue
$ cd jitbuilder
$ make -j4 PLATFORM=amd64-linux64-gcc
```

JitBuilder静态库构建后位于 `release/libjitbuilder.a`。
对应的C++ 头文件位于 `release/include/`。

## JIT 编译器

JIT编译器可以灵活地构建成两种不同形式的二进制文件。

默认和推荐的JIT构建方式是构建成静态库(static library)。它就既能链接到VM库又能直接链接到应用程序执行文件里面。

当前支持的另一种方式是把JIT作为目标文件（object file）来链接。这种情况下，内部目标文件（intermediate object file，指每个c/c++文件生成的.o文件）被链接成一个单一的大的目标文件。这个单一目标文件可以被其他目标文件再次链接使用。

当前JIT必须直接链接到VM (不管使用什么二进制)，所以构建成共享库当前还不被支持。

在JIT Makefile中提供了下列伪目标（phony target），以用来构建不同的二进制文件。

* `staticlib` (`默认值`) 构建JIT成静态库
* `lvjitobj` 构建JIT成目标文件

在构建JIT的时候，如果VM作为C代码来编译就必须定义宏 `LUA_C_LINKAGE` 。这可以保证当把JIT和VM链接在一起的时候，不会出现命名混乱问题。当调用 `make`的时候带上参数 `CXX_FLAGS_EXTRA='-DLUA_C_LINKAGE'` 就定义了 `LUA_C_LINKAGE` 宏（参考根目录下的Makefile）。

下列表格列出了在构建JIT的时候，`make` 的配置变量。

| 变量                  | 默认值     | 描述            |
|:--------------------:|:-----------------:|:-----------------------|
| `CXX`                | `g++`             | C++ 编译器 |
| `AR`                 | `ar`              | 静态库 .a 文件的创建器 |
| `LD`                 | `ld`              | 代码链接器 |
| `CXX_FLAGS_EXTRA`    | (empty)           | 传递给C++编译器的额外参数 |
| `LD_FLAGS_EXTRA`     | (empty)           | 传递给链接器的额外参数 |
| `ARCH`               | `x` (for x86)     | CPU架构      |
| `SUBARCH`            | `amd64`           | CPU子架构       |
| `STATICLIB_NAME`     | `liblvjit.a`      | JIT静态库名 |
| `LVJITOBJ_NAME`      | `lvjit.o`         | JIT目标文件名 |
| `BIN_DIR`            | `$(PWD)`          | 编译后的二进制文件存放的目录 |
| `STATICLIB_DEST`     | `$(BIN_DIR)`      | JIT静态库存放目录 |
| `LVJITOBJ_DEST`      | `$(BIN_DIR)`      | JIT目标文件存放目录 |
| `OBJS_DIR`           | `$(BIN_DIR)/objs` | 编译生成的中间文件的存放目录 |
| `LUA_DIR`            | `../`             | Lua VM代码目录 |
| `OMR_DIR`            | `../omr`          | Eclipse OMR 代码目录 |
| `JITBUILDER_LIB_DIR` | (见下表)      | JitBuilder二进制文件目录 |

为了最大的灵活性，还提供了其他的变量，即使其他项目都不需要用到它们。

| 变量                   | 默认值                         | 描述   |
|:--------------------------:|:-------------------------------------:|:--------------|
| `JITBUILDER_DIR`           | `$(OMR_DIR)/jitbuilder`               | JitBuilder代码目录 |
| `JITBUILDER_ARCH_DIR`      | `$(JITBUILDER_DIR)/$(ARCH)`           | JitBuilder特定处理器代码目录 |
| `JITBUILDER_SUBARCH_DIR`   | `$(JITBUILDER_ARCH_DIR)/$(SUBARCH)`   | JitBuilder特定处理器子架构代码目录 |
| `JITBUILDER_LIB_DIR`       | `$(JITBUILDER_DIR)/release`           | (见上表)   |
| `JITBUILDER_HEADERS`       | `$(JITBUILDER_LIB_DIR)/include`       | JitBuilder 头文件目录 |
| `OMR_COMPILER_DIR`         | `$(OMR_DIR)/compiler`                 | Eclipse OMR 编译器代码目录 |
| `OMR_COMPILER_ARCH_DIR`    | `$(OMR_COMPILER_DIR)/$(ARCH)`         | Eclipse OMR 编译器特定架构代码目录 |
| `OMR_COMPILER_SUBARCH_DIR` | `$(OMR_COMPILER_ARCH_DIR)/$(SUBARCH)` | Eclipse OMR 编译器特定子架构代码目录 |

## Lua VM

Lua VM的构建系统几乎没有修改过，应该和PUC-Rio Lua的构建方式一样。构建VM静态库可以使用下面的命令：

```sh
$ cd lua
$ make liblua.a SYSCFLAGS="-DLUA_USE_LINUX" 
```

## 把所有的组件组合在一起

一旦这三个组件分别构建成功，他们就可以链接成一个应用程序。重要提示：这个链接即可以由C++编译完成又可以通过显式链接C++标准库完成。

另外一种方式就是，把JIT和JitBuilder链接在一起（再强调一次，是用C++编译器链接），然后和C应用程序链接在一起。

示例链接命令如下（假设要链接 `lua.o`）：

```sh
$ g++ lua/lua.o lua/liblua.a lvjit/liblvjit.a omr/jitbuilder/release/libjitbuilder.a -ldl -lm -Wl,-E -lreadline
```

## 完整的例子

下面两个例子都演示了如何构建 `luav`。注意：通过调用根目录下的makefile，同样可以得到例子中的构建结果。

把JIT作为静态库构建：

```sh
$ lua
$ make -j4 lua.o SYSCFLAGS="-DLUA_USE_LINUX"
$ make -j4 liblua.a SYSCFLAGS="-DLUA_USE_LINUX" MYCFLAGS="-O2 -g"
$ cd ../omr
$ make -f run_configure.mk SPEC=linux_x86-64 OMRGLUE=./example/glue
$ cd jitbuilder
$ make -j4 CXX="g++" PLATFORM=amd64-linux64-gcc
$ cd ../../lvjit
$ make -j4 staticlib CXX="g++" CXX_FLAGS_EXTRA="-fpermissive -DLUA_C_LINKAGE -O3 -g"
$ cd ../
$ g++ lua/lua.o lua/liblua.a lvjit/liblvjit.a omr/jitbuilder/release/libjitbuilder.a -ldl -lm -Wl,-E -lreadline
```

另外一种方式，把JIT作为目标文件构建

```sh
$ lua
$ make -j4 lua.o SYSCFLAGS="-DLUA_USE_LINUX"
$ make -j4 liblua.a SYSCFLAGS="-DLUA_USE_LINUX" MYCFLAGS="-O2 -g"
$ cd ../omr
$ make -f run_configure.mk SPEC=linux_x86-64 OMRGLUE=./example/glue
$ cd jitbuilder
$ make -j4 CXX="g++" PLATFORM=amd64-linux64-gcc
$ cd ../../lvjit
$ make -j4 lvjitobj CXX="g++" CXX_FLAGS_EXTRA="-fpermissive -DLUA_C_LINKAGE -O3 -g"
$ cd ../
$ g++ lua/lua.o lua/liblua.a lvjit/lvjit.o omr/jitbuilder/release/libjitbuilder.a -ldl -lm -Wl,-E -lreadline
```

## 根目录下的Makefile

根目录下的Makefile自动完成 `luav` 的构建过程，除了需要按上述方式配置OMR。配置OMR仍然需要用户手工完成。

构建过程可以通过下列变量来定制：

| 变量              | 默认值                   | 描述            |
|:----------------:|:-----------------------:|:-----------------------|
| `CC`             | `gcc`                   | C编译器   |
| `CXX`            | `g++`                   | C++编译器 |
| `BUILD_CONFIG`   | `opt`                   | 值可以是 `debug`, `opt`, 和 `prod` 之一|
| `LUAV`           | `luav`                  | 最终应用程序名     |
| `LUA_APP`        | `lua.o`                 | 应用程序目标文件名 |
| `LUA_LIB`        | `liblua.a`              | Lua VM静态库文件名            |
| `LVJIT_LIB`      | `liblvjit.a`            | Lua Vermelha JIT静态库文件名  |
| `JITBUILDER_LIB` | `libjitbuilder.a`       | JitBuilder静态库文件名        |
| `LUA_DIR`        | `$(PWD)/lua`            | Lua VM 代码目录  |
| `LVJIT_DIR`      | `$(PWD)/lvjit`          | JIT代码目录     |
| `JITBUILDER_DIR` | `$(PWD)/omr/jitbuilder` | JitBuilder代码目录 |

`BUILD_CONFIG` 的值指定了构建结构的类型：

* `debug` 构建结果带调试符号，代码没有优化
* `opt` 构建结果带调试符号，但是代码被优化
* `prod` 构建结果不带调试符合，并且代码被优化

下面的命令将会使用JIT的静态库来构建 `luav`：

```sh
$ make -j4
```

下面的命令将会构建一个调试版本的 `luav`，同时使用的是JIT的目标文件。
```sh
$ make -j4 LVJIT_LIB=lvjit.o BUILD_CONFIG=debug
```
