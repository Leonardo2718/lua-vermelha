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
  Tests for the SELF opcode

  A table is defined with two fields. One field is treated as a
  simple variable, the other is a setter function for that variable.
  The test function defined takes an instance of the table, calls
  the setter on it using OO style (with `:`), and returns the value
  of the variable. The test passes if the function is successfully
  compiled and it returns the value given to the setter.
  
  Other opcodes exercised by this function:
    - LOADK
    - CALL
    - GETTABLE
    - RETRUN
--]]

-- setup
local asserts = require "asserts"
local jit = require "lvjit"
local assert_equal = asserts.assert_equal
local assert_compile = asserts.assert_compile
jit.setjitflags(assert_equal, jit.JITBLACKLIST)
jit.setjitflags(assert_compile, jit.JITBLACKLIST)

-- define test table and function
local table = {var = 0}
table.setvar = function(self, v) 
  self.var = v
end

local op_self = function(t, v)
  t:setvar(v)
  return t.var
end

-- run test
assert_compile(op_self, "op_self") 
assert_equal(3, op_self(table, 3), "t:getvar()")
