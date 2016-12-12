local asserts = require "asserts"
local assert_equal = asserts.assert_equal

local function op_settabup_a_3() a = 3 end
local function op_gettabup_a() return a end

assert_equal(nil, a, "`a` before op_settabup_a_3()")
op_settabup_a_3()
assert_equal(3, a, "`a` after op_settabup_a_3()")
assert_equal(3, op_gettabup_a(), "op_gettabup_a()")
