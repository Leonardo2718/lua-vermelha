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
  Test for JMP opcode

  The function defined exercises the JMP opcode.
  The `goto` statement generate a JMP that, when executed,
  will skip a variable assignment. The test passes if
  only the first assignment to the returned variabled
  is executed.
  
  Other opcodes exercised by this function:
    - LOADK
    - RETRUN
--]]

local asserts = require "asserts"
local assert_equal = asserts.assert_equal

local function return4not3()
   local a = 4
   goto last
   a = 3
   ::last::
   return a
end

assert_equal(4, return4not3(), "return4not3()")
