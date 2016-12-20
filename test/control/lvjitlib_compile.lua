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
  Test the `lvjit.compile()` function. The test passes if `foo`
  *is not compiled before* a call to `lvjit.compile(foo)` and
  *is compiled after* a call to `lvjit.compile(foo)`.
]]--

local jit = require "lvjit"

local function foo() end

assert(not jit.iscompiled(foo), "foo compiled before call to `jit.compile(foo)`")
assert(jit.compile(foo), "foo failed to compile")
assert(jit.iscompiled(foo), "foo not compiled after call to `jit.compile(foo)`")

