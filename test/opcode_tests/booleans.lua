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
  Tests for the TEST and TESTSET opcodes

  The TEST and TESTSET opcodes are generated when
  performing boolean opertations. Each test function
  has a boolean operation that generates a different
  invocation to one of the two opcodes. A test passes
  if the test function is successfully compiled and the
  value returned by the compiled function equals the
  expected value of the computation represented by the
  call to the test function.
  
  Other opcodes exercised by this function:
    - JMP
    - MOVE
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
local function op_and(a,b) return a and b end
  -- generates TESTSET with ARG_C = 0
local function op_or(a,b) return a or b end
  -- generates TESTSET with ARG_C = 1
local function ternary(a,b,c) return a and b or c end
  -- generates TEST and TESTSET

-- run tests
assert_compile(op_and, "op_and")
assert_equal(false, op_and(false,false), "op_and(false,false)")
assert_equal(false, op_and(false,true), "op_and(false,true)")
assert_equal(false, op_and(true,false), "op_and(true,false)")
assert_equal(true, op_and(true,true), "op_and(true,true)")

assert_compile(op_or, "op_or")
assert_equal(false, op_or(false,false), "op_or(false,false)")
assert_equal(true, op_or(false,true), "op_or(false,true)")
assert_equal(true, op_or(true,false), "op_or(true,false)")
assert_equal(true, op_or(true,true), "op_or(true,true)")

assert_compile(ternary, "ternary")
assert_equal(1, ternary(true,1,2), "ternary(true,1,2)")
assert_equal(2, ternary(false,1,2), "ternary(false,1,2)")
