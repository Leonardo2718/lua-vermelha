--[[
  Tests for the TEST and TESTSET opcodes

  The TEST and TESTSET opcodes are generated when
  performing boolean opertations. Each test function
  has a boolean operation that generates a different
  invocation to one of the two opcodes. A test passes
  if the value returned by a function equals the
  expected value of the computation.
  
  Other opcodes exercised by this function:
    - JMP
    - MOVE
    - RETRUN
--]]

local asserts = require "asserts"
local assert_equal = asserts.assert_equal

local function op_and(a,b) return a and b end
  -- generates TESTSET with ARG_C = 0
local function op_or(a,b) return a or b end
  -- generates TESTSET with ARG_C = 1
local function ternary(a,b,c) return a and b or c end
  -- generates TEST and TESTSET

assert_equal(false, op_and(false,false), "op_and(false,false)")
assert_equal(false, op_and(false,true), "op_and(false,true)")
assert_equal(false, op_and(true,false), "op_and(true,false)")
assert_equal(true, op_and(true,true), "op_and(true,true)")

assert_equal(false, op_or(false,false), "op_or(false,false)")
assert_equal(true, op_or(false,true), "op_or(false,true)")
assert_equal(true, op_or(true,false), "op_or(true,false)")
assert_equal(true, op_or(true,true), "op_or(true,true)")

assert_equal(1, ternary(true,1,2), "ternary(true,1,2)")
assert_equal(2, ternary(false,1,2), "ternary(false,1,2)")
