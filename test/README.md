# Lua Vermelha Testing

This directory contains tests cases for Lua Vermelha. These are further
divided into sub-directories.

## opcode

The `opcode` directory contains small test cases that exercise
each implemented opcode.

Each test case is a small Lua script. A test case can be executed by
running a specific test file using `luav`.

For convenience there is a `all.lua` file that executes all other
test files in the directory.

## lvjit

The `lvjit` directory contains test cases that exercise the JIT control
functions exposed by the Lua Vermelha C API.

## control

The `control` directory contains test cases that exercise the `lvjit`
library for controlling the JIT compiler from Lua code.

