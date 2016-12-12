local asserts = require "asserts"
local assert_equal = asserts.assert_equal

local function loadconst_3() return 3 end
local function return2vals() return 1, 2 end
local function loadnil() return nil end
local function loadbool_true() return true end

assert_equal(3, loadconst_3(), "loadconst_3()")
local a,b = return2vals()
assert_equal(1, a, "first value of return2vals()")
assert_equal(2, b, "second value of return2vals()")
assert_equal(nil, loadnil(), "loadnil()")
assert_equal(true, loadbool_true(), "loadbool_true()")

