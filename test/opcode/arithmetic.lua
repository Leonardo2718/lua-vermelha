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
  Tests for arithmetic and bitwise opcodes

  A test function is defined that exercises each arithmetic
  and bitwise opcode. The function names match the opcode
  they test. A test passes if the test function is successfully
  compiled and the returne value of the compiled function equals
  the expected value of the computation represented by the
  call to the test function.
  
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
local function op_add(x, y)  return x + y end
local function op_sub(x, y)  return x - y end
local function op_mul(x, y)  return x * y end
local function op_mod(x, y)  return x % y end
local function op_pow(x, y)  return x ^ y end
local function op_div(x, y)  return x / y end
local function op_idiv(x, y) return x // y end
local function op_band(x, y) return x & y end
local function op_bor(x, y)  return x | y end
local function op_bxor(x, y) return x ~ y end
local function op_shl(x, y)  return x << y end
local function op_shr(x, y)  return x >> y end
local function op_unm(x)     return -x end
local function op_bnot(x)    return ~x end
local function op_not(x)     return not x end

-- run tests
assert_compile(op_add, "op_add")
assert_equal(7, op_add(3,4), "op_add(3,4)")
assert_equal(7.3, op_add(3.3,4), "op_add(3.3,4)")
assert_equal(7.4, op_add(3,4.4), "op_add(3,4.4)")
assert_equal(7.7, op_add(3.3,4.4), "op_add(3.3,4.4)")

assert_compile(op_sub, "op_sub")
assert_equal(-1, op_sub(3,4), "op_sub(3,4)")

assert_compile(op_mul, "op_mul")
assert_equal(12, op_mul(3,4), "op_mul(3,4)")

assert_compile(op_mod, "op_mod")
assert_equal(1, op_mod(15,2), "op_mod(15,2)")

assert_compile(op_pow, "op_pow")
assert_equal(256, op_pow(2,8), "op_pow(2,8)")

assert_compile(op_div, "op_div")
assert_equal(0.5, op_div(1,2), "op_div(1,2)")

assert_compile(op_idiv, "op_idiv")
assert_equal(2, op_idiv(8,4), "op_idiv(8,4)")

assert_compile(op_band, "op_band")
assert_equal(2, op_band(3,6), "op_band(3,6)")

assert_compile(op_bor, "op_bor")
assert_equal(7, op_bor(3,6), "op_bor(3,6)")

assert_compile(op_bxor, "op_bxor")
assert_equal(5, op_bxor(3,6), "op_bxor(3,6)")

assert_compile(op_shl, "op_shl")
assert_equal(16, op_shl(4,2), "op_shl(4,2)")

assert_compile(op_shr, "op_shr")
assert_equal(1, op_shr(4,2), "op_shr(4,2)")

assert_compile(op_unm, "op_unm")
assert_equal(-3, op_unm(3), "op_unm(3)")

assert_compile(op_bnot, "op_bnot")
assert_equal(~15, op_bnot(15), "op_bnot(15)")

assert_compile(op_not, "op_not")
assert_equal(false, op_not(true), "op_not(true)")
