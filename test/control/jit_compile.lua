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

--[[
  Test that a function gets compiled after enough calls by the interpreter. Test
  passes if the function under test is not compiled before and after the first
  100 calls to it and the 101st call causes it to be JIT compiled.
]]--

local jit = require "lvjit"

local function foo() end

assert(not jit.iscompiled(foo), "foo compiled before any calls")

-- call `foo` enough times to force a compilation
local initcount = jit.initcallcounter()
for i = 1,initcount do
   foo()
   assert(not jit.iscompiled(foo), "foo compiled after "..tostring(i).." call(s) instead of "..tostring(initcount))
end

foo()
assert(jit.iscompiled(foo), "foo not compiled after "..tostring(initcount).." call(s)")

