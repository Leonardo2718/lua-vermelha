--[[----------------------------------------------------------------------------
 
  (c) Copyright IBM Corp. 2017,2017
 
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
  Tests for varargs opcode.
  
  Other opcodes exercised by this function:
    - CALL, GETTABUP, LOADK, MOVE, RETURN
--]]

-- setup
local asserts = require "asserts"
local jit = require "lvjit"
local assert_equal = asserts.assert_equal
local assert_compile = asserts.assert_compile
jit.setjitflags(assert_equal, jit.JITBLACKLIST)
jit.setjitflags(assert_compile, jit.JITBLACKLIST)

local function varargs1(...)
   local arg1_1 = select(1,...)
   local arg2_1 = select(2,...)
   local arg1_2 = select(1,...)
   local arg3_1 = select(3,...)
   local arg2_2 = select(2,...)
   local arg4_1 = select(4,...)
   local arg_size = select("#", ...)
   return arg1_1,arg2_1,arg1_2,arg3_1,arg2_2,arg4_1,arg_size
end

assert_compile(varargs1, "varargs1")
jit.setjitflags(varargs1, jit.BLACKLIST)
local arg1_1,arg2_1,arg1_2,arg3_1,arg2_2,arg4_1,arg_size = varargs1(7,4,9)
assert_equal(7, arg1_1, "first value of varargs1()")
assert_equal(4, arg2_1, "second value of varargs1()")
assert_equal(7, arg1_2, "third value of varargs1()")
assert_equal(9, arg3_1, "fourth value of varargs1()")
assert_equal(4, arg2_2, "fifth value of varargs1()")
assert_equal(nil, arg4_1, "sixth value of varargs1()")
assert_equal(3, arg_size, "seventh value of varargs1()")
