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
  Test for comparison opcodes

  A function is defined that exercises each compoarison
  opcode. The function names match the opcode they are
  intended to test. A test passes if the value returned
  by a function equals the expected value of the computation.
  
  Other opcodes exercised by this function:
    - JMP
    - LOADBOOL
    - RETRUN
--]]

local asserts = require "asserts"
local assert_equal = asserts.assert_equal

local function op_eq(a, b) return a == b end
local function op_lt(a, b) return a < b end
local function op_le(a, b) return a <= b end

assert_equal(false, op_eq(1,2), "op_eq(1,2)")
assert_equal(true, op_eq(1,1), "op_eq(1,1)")
assert_equal(false, op_lt(1,1), "op_lt(1,1)")
assert_equal(true, op_lt(1,2), "op_lt(1,2)")
assert_equal(false, op_le(2,1), "op_le(2,1)")
assert_equal(true, op_le(1,1), "op_le(1,1)")
