local asserts = require "asserts"
local assert_equal = asserts.assert_equal

local function empty() end
local function justReturn() return end

assert_equal(nil, empty(), "empty()")
assert_equal(nil, justReturn(), "justReturn()")

