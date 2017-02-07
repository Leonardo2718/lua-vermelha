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
  Tests for the TFORCALL and TFORLOOP opcodes

  A test function is defined that copies the values from one
  table into another. The test passes if the test function is
  successfully compiled and calling the compiled function on
  two tables results in the destenation table having the same
  values as the source table for all keys in the source table.

  Other opcodes exercised by this function:
    - GETTABUP
    - MOVE
    - CALL
    - JMP
    - SETTABLE
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
local copy_into = function(dest, src)
  for k,v in pairs(src) do
    dest[k] = v
  end
end

-- run test
local init_t = {1, 2, 3, 4}
local other_t = {}
assert_compile(copy_into, "copy_into")
copy_into(other_t, init_t)
for k,v in pairs(init_t) do
 assert_equal(other_t[k], v, "other_t[k] == v")
end
