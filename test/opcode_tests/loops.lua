--[[
  Test the loop opcodes

  A function is defined that exercises the FORPREP
  and FORLOOP opcodes. A single function is used to
  test both of these because a well formed function
  cannot have one without the other. The function
  uses a loop to perform a computation and returns
  the result. The test passes if the function returns
  the expected result of the computation.
  
  Other opcodes exercised by this function:
    - LOADK
    - MOVE
    - AND
    - RETRUN
--]]

local asserts = require "asserts"
local assert_equal = asserts.assert_equal

local function sum(a,b,inc)
   local r = 0
   for i = a,b,inc do
      r = r + i
   end
   return r
end

assert_equal(30, sum(0,10,2), "sum(0,10,2)")
