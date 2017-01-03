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
  Test for JMP opcode

  The test function defined exercises the JMP opcode.
  The `goto` statement generate a JMP that, when executed,
  will skip a variable assignment. The test passes if
  the test function is successfully compiled and only the
  first assignment to the returned variable is executed
  when the compiled function is called.
  
  Other opcodes exercised by this function:
    - LOADK
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
local function return4not3()
   local a = 4
   goto last
   a = 3
   ::last::
   return a
end

-- run test
assert_compile(return4not3, "return4not3")
assert_equal(4, return4not3(), "return4not3()")
