--[[----------------------------------------------------------------------------
 
  (c) Copyright IBM Corp. 2016, 2017
 
   This program and the accompanying materials are made available
   under the terms of the Eclipse Public License v1.0 and
   Apache License v2.0 which accompanies this distribution.
 
       The Eclipse Public License is available at
       http://www.eclipse.org/legal/epl-v10.html
 
       The Apache License v2.0 is available at
       http://www.opensource.org/licenses/apache2.0.php
 
  Contributors:
     Multiple authors (IBM Corp.) - initial implementation and documentation
 
--]]----------------------------------------------------------------------------

local asserts = require "asserts"
local jit = require "lvjit"
local assert_equal = asserts.assert_equal
local assert_compile = asserts.assert_compile
jit.setjitflags(assert_equal, jit.JITBLACKLIST)
jit.setjitflags(assert_compile, jit.JITBLACKLIST)

-- define test function
local function sum_int()
   local r = 0
   for index = 0,200,1 do
   for index2 = 0,500000,1 do
   r = r + 1
   r = r + 2
   r = r + 3
   r = r + 4
   r = r + 5
   r = r + 6
   r = r + 7
   r = r + 8
   r = r + 9
   r = r + 10
   r = r + 11
   r = r + 12
   r = r + 13
   r = r + 14
   r = r + 15
   r = r + 16
   r = r + 17
   r = r + 18
   r = r + 19
   r = r + 20
   r = r + 21
   r = r + 22
   r = r + 23
   r = r + 24
   r = r + 25
   end
   end
   return r
end

local function sum()
   local r = 0
   for index = 0,200,1 do
   for index2 = 0,500000,1 do
   r = r + 1
   r = r + 2
   r = r + 3
   r = r + 4
   r = r + 5
   r = r + 6
   r = r + 7
   r = r + 8
   r = r + 9
   r = r + 10
   r = r + 11
   r = r + 12
   r = r + 13
   r = r + 14
   r = r + 15
   r = r + 16
   r = r + 17
   r = r + 18
   r = r + 19
   r = r + 20
   r = r + 21
   r = r + 22
   r = r + 23
   r = r + 24
   r = r + 25
   end
   end
   return r
end

jit.setjitflags(sum_int, jit.JITBLACKLIST)
local int_sum_start = os.clock()
local int_sum_result = sum_int()
local int_sum_time = os.clock() - int_sum_start
assert_equal(32662565325, int_sum_result, "sum() interpreted returned the wrong value")
print(string.format("sum() of int constants interpreted took %.5f", int_sum_time))

assert_compile(sum, "sum")
jit.setjitflags(sum, jit.JITBLACKLIST)

for i = 1,5,1 do
local jit_sum_start = os.clock()
local jit_sum_result = sum()
local jit_sum_time = os.clock() - jit_sum_start
assert_equal(32662565325, jit_sum_result, "sum() compiled returned the wrong value")
print(string.format("sum() of int constants compiled took %.5f", jit_sum_time))
end
