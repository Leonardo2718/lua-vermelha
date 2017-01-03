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
  Test the load opcodes

  A test function is defined that exersise the different
  load opcodes. Values are loaded from the constant table
  into registers as return values to each function. A test
  passes if the test function is successfully compiled and
  the value(s) returned by the compiled function equal the
  ones the function is expected to load.
  
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
local function loadconst_3() return 3 end
  -- exercises LOADK
local function return2vals() return 1, 2 end
  -- exercises LOADK (mostly to test returning more than one value)
local function loadnil() return nil end
  -- exercises LOADNIL
local function loadbool_true() return true end
  -- exercises LOADBOOL

-- run tests
assert_compile(loadconst_3, "loadconst_3")
assert_equal(3, loadconst_3(), "loadconst_3()")

assert_compile(return2vals, "return2vals")
local a,b = return2vals()
assert_equal(1, a, "first value of return2vals()")
assert_equal(2, b, "second value of return2vals()")

assert_compile(loadnil, "loadnil")
assert_equal(nil, loadnil(), "loadnil()")

assert_compile(loadbool_true, "loadbool_true")
assert_equal(true, loadbool_true(), "loadbool_true()")
