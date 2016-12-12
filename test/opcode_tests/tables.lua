local asserts = require "asserts"
local assert_equal = asserts.assert_equal

local function op_newtable() return {} end
local function op_settable(t, i, v)
   t[i] = v
end
local function op_gettable(t, i)
   return t[i]
end
function op_len(t)
   return #t
end

local t
assert_equal("nil", type(t), "`type(t)` before op_newtable()")
t = op_newtable()
assert_equal("table", type(t), "`type(t)` after op_newtable()")

assert_equal(nil, t[2], "`t[2]` before op_settable(t, 2, \"two\")")
assert_equal(nil, op_gettable(t, 2), "op_gettable(t, 2) before op_settable(t, 2, \"two\")")
assert_equal(0, op_len(t), "op_len(t) before op_settable(t, 2, \"two\")")
op_settable(t, 2, "two")
assert_equal("two", t[2], "`t[2]` after op_settable(t, 2, \"two\")")
assert_equal("two", op_gettable(t, 2), "op_gettable(t, 2) after op_settable(t, 2, \"two\")")
assert_equal(0, op_len(t), "op_len(t) after op_settable(t, 2, \"two\")")

assert_equal(nil, t[1], "`t[1]` before op_settable(t, 1, \"one\")")
assert_equal(nil, op_gettable(t, 1), "op_gettable(t, 1) before op_settable(t, 1, \"one\")")
assert_equal(0, op_len(t), "op_len(t) before op_settable(t, 1, \"one\")")
op_settable(t, 1, "one")
assert_equal("one", t[1], "`t[1]` after op_settable(t, 1, \"one\")")
assert_equal("one", op_gettable(t, 1), "op_gettable(t, 1) after op_settable(t, 1, \"one\")")
assert_equal(2, op_len(t), "op_len(t) after op_settable(t, 1, \"one\")")

