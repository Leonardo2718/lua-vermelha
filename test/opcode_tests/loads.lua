--[[
  Test the load opcodes

  A function is defined that exersise the different load
  opcodes. Values are loaded from the constant table into
  registers as return values to each function. A test passes
  if the value(s) returned by a function equals the one
  the function is expected to load.
  
  Other opcodes exercised by this function:
    - RETRUN
--]]

local asserts = require "asserts"
local assert_equal = asserts.assert_equal

local function loadconst_3() return 3 end
  -- exercises LOADK
local function return2vals() return 1, 2 
  -- exercises LOADK (mostly to test returning more than one value)
local function loadnil() return nil end
  -- exercises LOADNIL
local function loadbool_true() return true end
  -- exercises LOADBOOL

assert_equal(3, loadconst_3(), "loadconst_3()")
local a,b = return2vals()
assert_equal(1, a, "first value of return2vals()")
assert_equal(2, b, "second value of return2vals()")
assert_equal(nil, loadnil(), "loadnil()")
assert_equal(true, loadbool_true(), "loadbool_true()")
