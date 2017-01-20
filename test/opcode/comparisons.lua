--[[----------------------------------------------------------------------------
 
  (c) Copyright IBM Corp. 2016, 2017
 
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
  Test for comparison opcodes

  A test function is defined to exercises each compoarison
  opcode. The function names match the opcode they are
  intended to test. A test passes if the test function is
  successfully compiled and the value returned by the
  compiled function equals the expected value of the
  computation represented by the call to the test function.
  
  Other opcodes exercised by this function:
    - JMP
    - LOADBOOL
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
local function op_eq(a, b) return a == b end
local function op_lt(a, b) return a < b end
local function op_le(a, b) return a <= b end

-- run tests
assert_compile(op_eq, "op_eq")
assert_equal(false, op_eq(1,2), "op_eq(1,2)")
assert_equal(true, op_eq(1,1), "op_eq(1,1)")
assert_equal(false, op_eq(1.0,2), "op_eq(1.0,2)")
assert_equal(true, op_eq(1.0,1), "op_eq(1.0,1)")
assert_equal(false, op_eq(1,2.0), "op_eq(1,2.0)")
assert_equal(true, op_eq(1,1.0), "op_eq(1,1.0)")
assert_equal(false, op_eq(1,1.1), "op_eq(1,1.1)")
assert_equal(false, op_eq(1.0,2.0), "op_eq(1.0,2.0)")
assert_equal(true, op_eq(1.0,1.0), "op_eq(1.0,1.0)")
assert_equal(false, op_eq(1.0,1.1), "op_eq(1.0,1.1)")
assert_equal(false, op_eq("1","2"), "op_eq('1','2')")
assert_equal(true, op_eq("1","1"), "op_eq('1','1')")

assert_compile(op_lt, "op_lt")
assert_equal(false, op_lt(1,1), "op_lt(1,1)")
assert_equal(true, op_lt(1,2), "op_lt(1,2)")
assert_equal(false, op_lt(1.0,1), "op_lt(1.0,1)")
assert_equal(true, op_lt(1.0,2), "op_lt(1.0,2)")
assert_equal(false, op_lt(1,1.0), "op_lt(1,1.0)")
assert_equal(true, op_lt(1,1.1), "op_lt(1,1.1)")
assert_equal(true, op_lt(1,2.0), "op_lt(1,2.0)")
assert_equal(false, op_lt(1.0,1.0), "op_lt(1.0,1.0)")
assert_equal(true, op_lt(1.0,2.0), "op_lt(1.0,2.0)")
assert_equal(true, op_lt(1.0,1.1), "op_lt(1.0,1.1)")
assert_equal(false, op_lt("1","1"), "op_lt('1','1')")
assert_equal(true, op_lt("1","2"), "op_lt('1','2')")

assert_compile(op_le, "op_le")
assert_equal(false, op_le(2,1), "op_le(2,1)")
assert_equal(true, op_le(1,1), "op_le(1,1)")
assert_equal(false, op_le(2.0,1), "op_le(2.0,1)")
assert_equal(true, op_le(1.0,1), "op_le(1.0,1)")
assert_equal(false, op_le(2,1.0), "op_le(2,1.0)")
assert_equal(true, op_le(1,1.0), "op_le(1,1.0)")
assert_equal(true, op_le(1,1.1), "op_le(1,1.1)")
assert_equal(false, op_le(2.0,1.0), "op_le(2.0,1.0)")
assert_equal(true, op_le(1.0,1.0), "op_le(1.0,1.0)")
assert_equal(true, op_le(1.0,1.1), "op_le(1.0,1.1)")
assert_equal(false, op_le("2","1"), "op_le('2','1')")
assert_equal(true, op_le("1","1"), "op_le('1','1')")
