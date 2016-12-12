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
