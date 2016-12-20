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
  Test the loop opcodes

  A test function is defined that exercises the FORPREP
  and FORLOOP opcodes. A single function is used to
  test both of these because a well formed function
  cannot have one without the other. The function
  uses a loop to perform a computation and returns
  the result. The test passes if the test function is
  successfully compiled and a call to the compiled
  function returns the expected result of the
  computation represented by the function.
  
  Other opcodes exercised by this function:
    - LOADK
    - MOVE
    - AND
    - RETRUN
--]]

-- setup
local asserts = require "asserts"
local jit = require "lvjit"
local assert_equal = asserts.assert_equal
local assert_compile = asserts.assert_compile
jit.setjitflags(assert_equal, jit.JITBLACKLIST)
jit.setjitflags(assert_compile, jit.JITBLACKLIST)

-- define test function
local function sum(a,b,inc)
   local r = 0
   for i = a,b,inc do
      r = r + i
   end
   return r
end

-- run test
assert_compile(sum, "sum")
assert_equal(30, sum(0,10,2), "sum(0,10,2)")
