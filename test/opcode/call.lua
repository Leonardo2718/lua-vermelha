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
  Tests for the CALL opcode

  Three test functions are defined. One calls a compiled Lua function,
  one calls a blacklisted Lua function (that must therefore be interpreted),
  and the last calls a C function. The first two simply return
  the values returned by the function they call. The last returns
  whether the JITBLACKLIST flag is set on the blacklisted function.
  A test passes if the test function is successfully compiled and
  the expected value is returned when the compiled test function
  is called.
  
  Other opcodes exercised by this function:
    - LOADK
    - GETTABUP
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
function getval_compiled() return 3 end -- not local to avoid generating GETUPVAL
assert_compile(getval_compiled, "getval_compiled")

function getval_interpreted() return 3 end -- not local to avoid generating GETUPVAL
jit.setjitflags(getval_interpreted, jit.JITBLACKLIST)

local function call_compiled()
  local r = getval_compiled()
  return r
end

local function call_interpreted()
  local r = getval_interpreted()
  return r
end

local function call_cfunction()
  local r = jit.checkjitflags(getval_interpreted, jit.JITBLACKLIST)
  return r
end

-- run tests
assert_compile(call_compiled, "call_compiled")
assert_equal(3, call_compiled(), "call_compiled()")

assert_compile(call_interpreted, "call_interpreted")
assert_equal(3, call_interpreted(), "call_interpreted()")

assert_compile(call_cfunction, "call_cfunction")
assert_equal(jit.JITBLACKLIST, call_cfunction(), "call_cfunction()")