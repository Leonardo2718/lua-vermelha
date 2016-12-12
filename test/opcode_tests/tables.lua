--[[
  Test table opcodes

  A function is defined the exercises each opcode
  that operates on tables. The function names match
  the opcode they test.
  
  Other opcodes exercised by this function:
    - RETRUN
--]]

local asserts = require "asserts"
local assert_equal = asserts.assert_equal

local function op_newtable() return {} end
local function op_settable(t, i, v)
   t[i] = v
end
local function op_gettable(t, i)
   return t[i]
end
local function op_len(t)
   return #t
end

local t
assert_equal("nil", type(t), "`type(t)` before op_newtable()")
t = op_newtable()
assert_equal("table", type(t), "`type(t)` after op_newtable()")
-- test passes if type of returned value is `"table"`

assert_equal(nil, t[2], "`t[2]` before op_settable(t, 2, \"two\")")
assert_equal(nil, op_gettable(t, 2), "op_gettable(t, 2) before op_settable(t, 2, \"two\")")
assert_equal(0, op_len(t), "op_len(t) before op_settable(t, 2, \"two\")")
op_settable(t, 2, "two")
assert_equal("two", t[2], "`t[2]` after op_settable(t, 2, \"two\")")
assert_equal("two", op_gettable(t, 2), "op_gettable(t, 2) after op_settable(t, 2, \"two\")")
assert_equal(0, op_len(t), "op_len(t) after op_settable(t, 2, \"two\")")
-- test passes if:
--    - indexing a table element before it's assigned by `op_settable()` returns `nil`
--    - calling `op_gettable()` before `op_settable()` is called to assign the index returns `nil`
--    - the table length before an index is assigned by `op_settable()` is `0`
--    - indexing a table element after it's assigned by `op_settable()` returns the assigned value
--    - calling `op_gettable()` with an index after it's assigned by `op_settable()` returns the assigned value
--    - the table length after an index (different thatn `1`) is assigned by `op_settable()` is `0`

assert_equal(nil, t[1], "`t[1]` before op_settable(t, 1, \"one\")")
assert_equal(nil, op_gettable(t, 1), "op_gettable(t, 1) before op_settable(t, 1, \"one\")")
assert_equal(0, op_len(t), "op_len(t) before op_settable(t, 1, \"one\")")
op_settable(t, 1, "one")
assert_equal("one", t[1], "`t[1]` after op_settable(t, 1, \"one\")")
assert_equal("one", op_gettable(t, 1), "op_gettable(t, 1) after op_settable(t, 1, \"one\")")
assert_equal(2, op_len(t), "op_len(t) after op_settable(t, 1, \"one\")")
-- test passes if:
--    - indexing a table element before it's assigned by `op_settable()` returns `nil`
--    - calling `op_gettable()` before `op_settable()` is called to assign the index returns `nil`
--    - the table length before an index is assigned by `op_settable()` is `0`
--    - indexing a table element after `op_settable()` is called to assign it returns the assigned value
--    - calling `op_gettable()` after `op_settable()` is calledto assign the index returns the assigned value
--    - the table length after indexes `1` and `2` are assigned by `op_settable()` is `2`
