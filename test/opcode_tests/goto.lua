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
