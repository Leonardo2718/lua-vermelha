# Lua Vermelha

[![Build Status](https://travis-ci.org/Leonardo2718/lua-vermelha.svg?branch=devel)](https://travis-ci.org/Leonardo2718/lua-vermelha)

Lua Vermelha是基于 [Eclipse OMR](https://github.com/eclipse/omr) JIT(Just-In-Time)编译器技术的一个 [Lua](http://www.lua.org) 5.3实现。

Lua Vermelha的设计目标是：轻微修改 [PUC-Rio Lua](http://www.lua.org)虚拟机的代码，从而实现和Lua虚拟机的集成。

*(注意：Lua Vermelha当前处于活跃的开发状态，可能还不太稳定还有bug)*

## 用git获取代码库

Lua Vermelha把Eclipse OMR项目作为自己的一个子模块，所以推荐用 `--recursive` 参数来clone代码库。

```shell
$ git clone --recursive https://github.com/Leonardo2718/lua-vermelha.git
```

## 构建

当前唯一支持的平台是64位x86架构下的Linux。其他平台的支持敬请期待。如果你希望在其他平台下构建Lua Vermelha，只需要手工修改Makefile即可。

**注意**：为了正确构建JitBuilder，配置好OMR是一件十分重要的事情（详细信息请参考[Eclipse OMR project page](https://github.com/eclipse/omr)）。

```shell
$ cd lua-vermelha/omr
$ make -f run_configure.mk SPEC=linux_x86-64 OMRGLUE=./example/glue
```

顶层目录里面的Makefile用来构建Lua Vermelha的执行前端程序 `luav` 。

```shell
$ cd ..     # 返回顶层目录 `lua-vermelha/`
$ make -j4  # 依次构建 JitBuilder, the JIT, the VM, and luav
```

## 运行

执行程序 `luav`的用法和 PUC-Rio Lua解释器（即 `lua`）的用法一样：

```shell
$ luav           # 启动交互式运行环境
$ luav file.lua  # 执行Lua脚本 `file.lua`
$ luav luac.out  # 执行Lua字节码 (由 `luac` 生成)
```

## 许可

Lua Vermelh有三个组件，各自使用各自的许可： 

* 微做修改的PUC-Rio Lua VM 仍使用其原始的许可（MIT许可）
* Lua Vermelha JIT编译器用下列两种许可:
  * [Eclipse Public License 2](https://www.eclipse.org/legal/epl-2.0/)
  * [Apache License v2.0](http://www.opensource.org/licenses/apache2.0.php)
* Eclipse OMR 组件的许可和 Lua Vermelha JIT一样
