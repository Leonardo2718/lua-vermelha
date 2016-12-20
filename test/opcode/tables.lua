--[[----------------------------------------------------------------------------
 
  (c) Copyright IBM Corp. 2016, 2016
 
   This program and the accompanying materials are made available
   under the terms of the Eclipse Public License v1.0 and
   Apache License v2.0 which accompanies this distribution.
 
       The Eclipse Public License is available at
       http://www.eclipse.org/legal/epl-v10.html
 
       The Apache License v2.0 is available at
       http://www.opensource.org/licenses/apache2.0.php
 
  Contributors:
     Multiple authors (IBM Corp.) - initial implementation and documentation
 
--]]----------------------------------------------------------------------------

--[[
  Test table opcodes

  A test function is defined the exercises each opcode
  that operates on tables. The function names match
  the opcode they test.
  
  Other opcodes exercised by this function:
    - RETRUN
--]]

-- setup
local asserts = require "asserts"
local jit = require "lvjit"
local assert_equal = asserts.assert_equal
local assert_compile = asserts.assert_compile
jit.setjitflags(assert_equal, jit.JITBLACKLIST)
jit.setjitflags(assert_compile, jit.JITBLACKLIST)

-- define test functions
local function op_newtable() return {} end
local function op_settable(t, i, v)
   t[i] = v
end
local function op_gettable(t, i)
   return t[i]
end
local function op_len(t)
   return #t
end

-- run tests

-- this test passes if:
--   1) `op_newtable` is successfully compiled and
--   2) `op_gettable` is successfully compiled
--   3) `op_len` is successfully compiled and
--   4) `op_settable` is successfully compiled
assert_compile(op_newtable, "op_newtable")
assert_compile(op_gettable, "op_gettable")
assert_compile(op_len, "op_len")
assert_compile(op_settable, "op_settable")

-- this test passes if:
--   1) type of returned value is `"table"`
local t
assert_equal("nil", type(t), "`type(t)` before op_newtable()")
t = op_newtable()
assert_equal("table", type(t), "`type(t)` after op_newtable()")

-- this test passes if:
--   1) indexing a table element before it's assigned by `op_settable()` returns `nil`
--   2) calling `op_gettable()` before `op_settable()` is called to assign the index returns `nil`
--   3) the table length before an index is assigned by `op_settable()` is `0`
--   4) indexing a table element after it's assigned by `op_settable()` returns the assigned value
--   5) calling `op_gettable()` with an index after it's assigned by `op_settable()` returns the assigned value
--   6) the table length after an index (different thatn `1`) is assigned by `op_settable()` is `0`
assert_equal(nil, t[2], "`t[2]` before op_settable(t, 2, \"two\")")
assert_equal(nil, op_gettable(t, 2), "op_gettable(t, 2) before op_settable(t, 2, \"two\")")
assert_equal(0, op_len(t), "op_len(t) before op_settable(t, 2, \"two\")")
op_settable(t, 2, "two")
assert_equal("two", t[2], "`t[2]` after op_settable(t, 2, \"two\")")
assert_equal("two", op_gettable(t, 2), "op_gettable(t, 2) after op_settable(t, 2, \"two\")")
assert_equal(0, op_len(t), "op_len(t) after op_settable(t, 2, \"two\")")

-- this test passes if:
--   1) indexing a table element before it's assigned by `op_settable()` returns `nil`
--   2) calling `op_gettable()` before `op_settable()` is called to assign the index returns `nil`
--   3) the table length before an index is assigned by `op_settable()` is `0` 
--   4) indexing a table element after `op_settable()` is called to assign it returns the assigned value
--   5) calling `op_gettable()` after `op_settable()` is calledto assign the index returns the assigned value
--   6) the table length after indexes `1` and `2` are assigned by `op_settable()` is `2`
assert_equal(nil, t[1], "`t[1]` before op_settable(t, 1, \"one\")")
assert_equal(nil, op_gettable(t, 1), "op_gettable(t, 1) before op_settable(t, 1, \"one\")")
assert_equal(0, op_len(t), "op_len(t) before op_settable(t, 1, \"one\")")
op_settable(t, 1, "one")
assert_equal("one", t[1], "`t[1]` after op_settable(t, 1, \"one\")")
assert_equal("one", op_gettable(t, 1), "op_gettable(t, 1) after op_settable(t, 1, \"one\")")
assert_equal(2, op_len(t), "op_len(t) after op_settable(t, 1, \"one\")")
