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
  Test for the MOVE opcode

  The swap test function uses three MOVE opcodes to
  swap the values of its parameters. The test
  passes if the test function is successfully compiled
  and calling the compiled `swap()` on two values does not
  crash (good enough for now).
  
  Other opcodes exercised by this function:
    - RETRUN
--]]

-- setup
local asserts = require "asserts"
local jit = require "lvjit"
local assert_compile = asserts.assert_compile
jit.setjitflags(assert_compile, jit.JITBLACKLIST)

-- define test function
local function swap(a,b) a,b = b,a end

-- run test
assert_compile(swap, "swap")
swap(3, 4)
