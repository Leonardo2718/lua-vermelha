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

  A test function is defined that returns a table constructed using
  a brace enclosed list. The values in the table are equal to their
  corresponding key. The test passes if the test function is
  successfully compiled and the values in the table returned by a
  call to the compiled function are equal to their corresponding
  key.

  Other opcodes exercised by this function:
    - NEWTABLE
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
local op_setlist = function()
  return {1, 2, 3, 4}
end

-- run test
assert_compile(op_setlist, "op_setlist")
table = op_setlist()
for k,v in ipairs(table) do
  assert_equal(k,v, "checking key and value pair of table")
end
