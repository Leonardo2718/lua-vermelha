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
  Test the functions for manipulating JIT flags. The test passes if:
    1) initially, a function has no JIT flags set
    2) a function has a specific flag set after a call to `jit.setjitflags` sets it
    3) a function has no flags set after a call to `jit.clearjitflags` clears the
       flag previously set
]]--

local jit = require "lvjit"

local function foo() end

assert(jit.NOJITFLAGS == jit.checkjitflags(foo, -1), "default function JIT flags is not `NOJITFLAGS`")
jit.setjitflags(foo, jit.JITBLACKLIST)
assert(jit.JITBLACKLIST == jit.checkjitflags(foo, -1), "function JIT flag JITBLACKLIST not set after call to `jit.setjitflags(foo, jit.JITBLACKLIST)`")
jit.clearjitflags(foo, jit.JITBLACKLIST)
assert(jit.NOJITFLAGS == jit.checkjitflags(foo, -1), "function JIT flag JITBLACKLIST set after call to `jit.clearjitflags(foo, jit.JITBLACKLIST)`")

