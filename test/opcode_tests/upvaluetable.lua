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
  Test upvalue table opcodes

  A test function is defined the exercises each
  opcode that operates on the upvalue table. The
  test function names match the opcode they test.
  For SETTABUP, the test passes if the test function
  is successfully compiled and the expected value is
  in the table after executing the compiled function.
  For GETTABUP, the test passes if the test function
  is successfully compiled and the value from the
  upvalue table is returned after executing the opcode.
  
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
local function op_settabup_a_3() a = 3 end
local function op_gettabup_a() return a end

-- run tests
assert_equal(nil, a, "`a` before op_settabup_a_3()")
assert_compile(op_settabup_a_3, "op_settabup_a_3")
op_settabup_a_3()
assert_equal(3, a, "`a` after op_settabup_a_3()")

assert_compile(op_gettabup_a, "op_gettabup_a")
assert_equal(3, op_gettabup_a(), "op_gettabup_a()")
