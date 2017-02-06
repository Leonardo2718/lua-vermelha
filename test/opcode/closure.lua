--[[----------------------------------------------------------------------------
 
  (c) Copyright IBM Corp. 2017, 2017
 
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
  Tests for the CLOSURE opcode

  A test function is defined that returns another function that
  returns the interger value 3. The test passes if the first function
  is successfully compiled and the function returned by the compiled
  function returns 3 when called.

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

-- define test function
local op_closure = function()
  return function() return 3 end
end

-- run test
assert_compile(op_closure, "op_closure")
assert_equal(3, op_closure()(), "op_closure()()")
