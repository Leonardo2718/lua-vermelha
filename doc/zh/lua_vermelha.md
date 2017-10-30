# Lua Vermelha

*请注意：Lua Vermelha当前处于活跃的开发状态，还不适用于生产环境。*

## 简介

Lua Vermelha被设计成与PUC-Rio Lua有相同的运行表现。任何使用PUC-Rio Lua作为库的项目也可以使用
Lua Vermelha。归功于JIT编译器，在使用了JIT编译器后能提供更好的性能，但是也付出了文件体积更大的代价。

JIT编译器使用Eclipse OMR JitBuilder来构建。 JitBuilder是一个基于Eclipse OMR编译器技术用来简化创建JIT编译器的框架。
更多Eclipse OMR信息请参考[Eclipse OMR GitHub 主页](https://github.com/eclipse/omr)。

## 架构概述

Lua Vermelha主要由下面三个组件构成：

1. 轻微修改过的PUC-Rio Lua 虚拟机
2. Eclipse OMR JitBuilder 库
3. JIT编译器

### Lua虚拟机

Lua虚拟机提供了Lua语言核心功能的实现。它主要包括解释器和垃圾回收器。轻微修改后的代码实现了分发调用给JIT编译器的必要机制。

当Lua虚拟机实例化后，它初始化JIT编译器并开始运行解释器。当解释器遇到函数调用的时候，它就分发调用给JIT，JIT就尝试去编译该函数。
如果编译成功，解释器就调用编译后的函数代码。随后的函数调用也会被分发给JIT。如果编译失败，该函数依旧按Lua虚拟机原有的方式解释执行。

这里没有使用任何方式来优化JIT的调用分发。但是这种方式能更容易发现和调试JIT本身的问题。

#### Lua patch 文件

为了方便起见，代码库里面有个git分支 `luavm` 用来存放更改之前的PUC-Rio Lua虚拟机代码。这个分支上每次提交都标记了其对应的Lua版本。

分支 `master`和`devel`的根目录Makefile里面有一个特殊的伪目标(phony target) `lua-patch`。它可以用来生成git patch文件，该patch文件含有两次提交之间Lua虚拟机代码的改动。默认情况下，这两次提交是当前的 `HEAD` 和 `luavm`。

运行下面的命令

```sh
$ make lua-patch
```
将会生成名为 `luavm-HEAD.patch` patch文件。`LUA` 变量可以用来指向没有改动过的Lua代码的提交。
类似地，`COMMIT` 变量用来指向改动了的Lua代码的提交。

### Eclipse OMR JitBuilder

[Eclipse OMR 项目](http://www.eclipse.org/omr) 提供了一组用于构建语言运行时的可重用跨平台的组件。
其中一个组件就是编译器技术，它可以用来创建JIT编译器。

尽管OMR编译器技术非常得强大和灵活，但是它也十分复杂。因此Eclipse OMR项目又提供了一个名叫JitBuilder的组件。

JitBuilder是一个利用Eclipse OMR编译器来简化构建JIT的框架。它通过提供诸如代码优化和代码生成这些主要任务的默认实现从而减少了对样本代码的编写工作量。它也提供了生成Eclipse OMR编译器中间语言（IL）的声明接口。

在Lua Vermelha中，JIT编译器使用JitBuilder构建。 

### Lua Vermelha JIT

JIT编译器是使Lua Vermelha成为当前样子的主要组件。

当JIT被解释器调用的时候，它获得一个参数，该参数是一个函数字节码实现的对象。然后它开始遍历字节码，
生成IL，这些IL模拟了解释器遇到这些字节码的时候的处理行为。如果每个字节码都成功地生成了IL，那么这些
函数的中间格式就传递给JitBuilder以用于代码优化和生成。

如果编译成功，JitBuilder将会在内存里面缓存编译后的代码并返回一个指向它的指针。解释器就能替换掉解释代码从而跳转到编译后代码处。

## luav 执行程序前端

根目录下的 `Makefile` 可以用来生成可以运行的Lua VM。它简单地构建JitBuilder、JIT（静态库）、VM（静态库）和前端，然后把这些链接起来。VM和前端代码使用 `gcc` 编译，而JitBuilder和JIT使用 `g++` 编译。链接工具也使用 `g++`。这个Makefile可以作为一个例子来演示如何构建和使用Lua Vermelha。