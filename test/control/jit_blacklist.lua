--[[----------------------------------------------------------------------------

  (c) Copyright IBM Corp. 2016, 2016

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
  Test that a black listed function does not get JIT compiled. The test
  passes if the function under test is not compiled after 101 calls to it.
]]--

local jit = require "lvjit"

local function foo() end

assert(not jit.iscompiled(foo), "foo compiled before any calls")
assert(jit.NOJITFLAGS == jit.checkjitflags(foo, -1), "default function JIT flags is not `NOJITFLAGS`")

-- black list the function
jit.setjitflags(foo, jit.JITBLACKLIST)
assert(jit.JITBLACKLIST == jit.checkjitflags(foo, -1), "function JIT flag JITBLACKLIST not set after call to `jit.setjitflags(foo, jit.JITBLACKLIST)`")

-- call `foo` enough times to force a compilation
local initcount = jit.initcallcounter()
for i = 1,initcount + 1 do
   foo()
   assert(not jit.iscompiled(foo), "foo compiled after "..tostring(i).." call(s) despite being black listed")
end
