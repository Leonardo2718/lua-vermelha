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

  The swap function uses three MOVE opcodes to
  swap the values of its parameters. The test
  passes if, after calling `swap()` on two variables,
  the original value of each variable is stored
  in the other.
  
  Other opcodes exercised by this function:
    - RETRUN
--]]

local asserts = require "asserts"
local assert_equal = asserts.assert_equal

local function swap(a,b) a,b = b,a end

local val1 = 3
local val2 = 4
swap(val1, val2)
assert_equal(4, val1, "val1 after swap")
assert_equal(3, val2, "val2 after swap")
