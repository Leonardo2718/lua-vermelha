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

