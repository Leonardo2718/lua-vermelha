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
  Run all test files
  
  Each test file is intended to a specific opcode
  or family of opcodes
--]]

local test_files = { "arithmetic.lua"
                   , "asserts.lua"
                   , "booleans.lua"
                   , "call.lua"
                   , "comparisons.lua"
                   , "concat.lua"
                   , "goto.lua"
                   , "loads.lua"
                   , "loops.lua"
                   , "move.lua"
                   , "return.lua"
                   , "self.lua"
                   , "tables.lua"
                   , "upvalue.lua"
                   , "upvaluetable.lua"
                   , "varargs.lua"
                   }

for _,file in ipairs(test_files) do
   dofile(file)
end
