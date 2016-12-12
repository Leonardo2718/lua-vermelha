local asserts = require "asserts"
local assert_equal = asserts.assert_equal

local function return4not3()
   a = 4
   goto last
   a = 3
   ::last::
   return a
end

assert_equal(4, return4not3(), "return4not3()")
