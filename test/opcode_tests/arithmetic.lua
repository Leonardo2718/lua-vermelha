local asserts = require "asserts"
local assert_equal = asserts.assert_equal

local function op_add(x, y)  return x + y end
local function op_sub(x, y)  return x - y end
local function op_mul(x, y)  return x * y end
local function op_mod(x, y)  return x % y end
local function op_pow(x, y)  return x ^ y end
local function op_div(x, y)  return x / y end
local function op_idiv(x, y) return x // y end
local function op_band(x, y) return x & y end
local function op_bor(x, y)  return x | y end
local function op_bxor(x, y) return x ~ y end
local function op_shl(x, y)  return x << y end
local function op_shr(x, y)  return x >> y end
local function op_unm(x)     return -x end
local function op_bnot(x)    return ~x end
local function op_not(x)     return not x end

assert_equal(7, op_add(3,4), "op_add(3,4)")
assert_equal(-1, op_sub(3,4), "op_sub(3,4)")
assert_equal(12, op_mul(3,4), "op_mul(3,4)")
assert_equal(1, op_mod(15,2), "op_mod(15,2)")
assert_equal(256, op_pow(2,8), "op_pow(2,8)")
assert_equal(0.5, op_div(1,2), "op_div(1,2)")
assert_equal(2, op_idiv(8,4), "op_idiv(8,4)")
assert_equal(2, op_band(3,6), "op_band(3,6)")
assert_equal(7, op_bor(3,6), "op_bor(3,6)")
assert_equal(5, op_bxor(3,6), "op_bxor(3,6)")
assert_equal(16, op_shl(4,2), "op_shl(4,2)")
assert_equal(1, op_shr(4,2), "op_shr(4,2)")
assert_equal(-3, op_unm(3), "op_unm(3)")
assert_equal(~15, op_bnot(15), "op_bnot(15)")
assert_equal(false, op_not(true), "op_not(true)")

