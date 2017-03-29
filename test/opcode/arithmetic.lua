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
  Tests for arithmetic and bitwise opcodes

  Two test function are defined that exercises each arithmetic
  and bitwise opcode. One is compiled and used for testing, the
  other is used as control. The two sets of functions are in
  different tables. Table fields are strings that match the opcode
  tested by the function they map to. A test passes if the test
  function is successfully compiled and its returne value is the
  same as that of the controle (uncompiled) function when called
  with the same arguments.
  
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

-- table of test functions
local testfunc = {}
testfunc.op_add  = function (x, y) return x + y end
testfunc.op_sub  = function (x, y) return x - y end
testfunc.op_mul  = function (x, y) return x * y end
testfunc.op_mod  = function (x, y) return x % y end
testfunc.op_pow  = function (x, y) return x ^ y end
testfunc.op_div  = function (x, y) return x / y end
testfunc.op_idiv = function (x, y) return x // y end
testfunc.op_band = function (x, y) return x & y end
testfunc.op_bor  = function (x, y) return x | y end
testfunc.op_bxor = function (x, y) return x ~ y end
testfunc.op_shl  = function (x, y) return x << y end
testfunc.op_shr  = function (x, y) return x >> y end
testfunc.op_unm  = function (x)    return -x end
testfunc.op_bnot = function (x)    return ~x end
testfunc.op_not  = function (x)    return not x end

-- table of control functions (return expected values)
local expfunc = {}
expfunc.op_add  = function (x, y) return x + y end
expfunc.op_sub  = function (x, y) return x - y end
expfunc.op_mul  = function (x, y) return x * y end
expfunc.op_mod  = function (x, y) return x % y end
expfunc.op_pow  = function (x, y) return x ^ y end
expfunc.op_div  = function (x, y) return x / y end
expfunc.op_idiv = function (x, y) return x // y end
expfunc.op_band = function (x, y) return x & y end
expfunc.op_bor  = function (x, y) return x | y end
expfunc.op_bxor = function (x, y) return x ~ y end
expfunc.op_shl  = function (x, y) return x << y end
expfunc.op_shr  = function (x, y) return x >> y end
expfunc.op_unm  = function (x)    return -x end
expfunc.op_bnot = function (x)    return ~x end
expfunc.op_not  = function (x)    return not x end

-- test input data
local num_args = {
  {3,     4},
  {3.3,   4},
  {3,     4.4},
  {3.3,   4.4}
}

local int_args = {
  {3,     4},
  {3.0,   4},
  {3,     4.0},
  {3.0,   4.0}
}

-- test runner
local function setup_test(opname)
  assert_compile(testfunc[opname], opname)
  jit.setjitflags(expfunc[opname], jit.BLACKLIST)
  assert_equal(false, jit.iscompiled(expfunc[opname]), "expected (control) "..opname.." got compiled")
end

local function test_runner(opname, argslist)
  setup_test(opname)
  for _,arguments in pairs(argslist) do
    expected = expfunc[opname](arguments[1],arguments[2])
    actual = testfunc[opname](arguments[1],arguments[2])
    assert_equal(expected, actual, opname.."("..arguments[1]..", "..arguments[2]..")")
  end
end

jit.setjitflags(setup_test, jit.BLACKLIST)
jit.setjitflags(test_runner, jit.BLACKLIST)

-- run tests
test_runner("op_add", num_args)
test_runner("op_sub", num_args)
test_runner("op_mul", num_args)
test_runner("op_mod", num_args)
test_runner("op_pow", num_args)
test_runner("op_div", num_args)

test_runner("op_idiv", int_args)
test_runner("op_band", int_args)
test_runner("op_bor", int_args)
test_runner("op_bxor", int_args)
test_runner("op_shl", int_args)
test_runner("op_shr", int_args)

setup_test("op_unm")
assert_equal(expfunc.op_unm(5), testfunc.op_unm(5), "op_unm(5)")
assert_equal(expfunc.op_unm(3.14159), testfunc.op_unm(3.14159), "op_unm(3.14159)")

setup_test("op_bnot")
assert_equal(expfunc.op_bnot(15), testfunc.op_bnot(15), "op_bnot(15)")
assert_equal(expfunc.op_bnot(31.0), testfunc.op_bnot(31.0), "op_bnot(31.0)")

setup_test("op_not")
assert_equal(expfunc.op_not(true), testfunc.op_not(true), "op_not(true)")
assert_equal(expfunc.op_not(false), testfunc.op_not(false), "op_not(false)")
