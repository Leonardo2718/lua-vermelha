--[[----------------------------------------------------------------------------
 
  (c) Copyright IBM Corp. 2017,2017
 
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

--[[
  Tests for concat opcode.
  
  Other opcodes exercised by this function:
    - MOVE, RETURN
--]]

-- setup
local asserts = require "asserts"
local jit = require "lvjit"
local assert_equal = asserts.assert_equal
local assert_compile = asserts.assert_compile
jit.setjitflags(assert_equal, jit.JITBLACKLIST)
jit.setjitflags(assert_compile, jit.JITBLACKLIST)

local function concat2(a,b) return a .. b end
local function concat3(a,b,c) return a .. b .. c end
local function concat6(a,b,c,d,e,f) return a .. b .. c .. d .. e .. f end

assert_compile(concat2, "concat2")
jit.setjitflags(concat2, jit.BLACKLIST)
assert_equal("HelloWorld", concat2("Hello", "World"), "concat2('Hello', 'World')")
assert_equal("High5", concat2("High", 5), "concat2('High', 5)")
assert_equal("2.0Times", concat2(2.0, "Times"), "concat2(2.0, 'Times')")

assert_compile(concat3, "concat3")
jit.setjitflags(concat3, jit.BLACKLIST)
assert_equal("Hello World", concat3("Hello", " ", "World"), "concat2('Hello', ' ', 'World')")

assert_compile(concat6, "concat6")
jit.setjitflags(concat6, jit.BLACKLIST)
assert_equal("The 3rd planet from the sun", concat6("The", " ", 3, "rd", " ", "planet from the sun"), "concat6('The', ' ', 3, 'rd', ' ', 'planet from the sun')")

