--[[----------------------------------------------------------------------------
 
  (c) Copyright IBM Corp. 2017, 2017
 
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
  Tests for the TAILCALL opcode
  
  Two test functions are defined: One tail-calls an interpreted function,
  the other tail-calls a compiled function. Using these three test cases
  are defined for: interpreter-to-JIT tail-call, JIT-to-interpreter
  tail-call, and JIT-to-JIT tail-call. In the first case, the test passes
  if the caller returns the value returned by the tail-called function.
  In the other two cases, the test passes if the caller is successfully
  compiled and calling the compiled body returns the value returned by
  the tail-called funcion.
  
  Other opcodes exercised by this function:
    - GETUPVAL
    - RETRUN
--]]

-- setup
local asserts = require "asserts"
local jit = require "lvjit"
local assert_equal = asserts.assert_equal
local assert_compile = asserts.assert_compile
jit.setjitflags(assert_equal, jit.JITBLACKLIST)
jit.setjitflags(assert_compile, jit.JITBLACKLIST)

-- define test functions
local interpreted_tail_callee = function () return 3 end
jit.setjitflags(interpreted_tail_callee, jit.JITBLACKLIST)
local compiled_tail_callee = function () return 3 end
assert_compile(compiled_tail_callee, "compiled_tail_callee")

local tailcall_interpreted = function () return interpreted_tail_callee() end
local tailcall_compiled = function () return compiled_tail_callee() end

-- run tests

-- test interpreter-to-JIT dispatch
assert_equal(3, tailcall_compiled(), "tailcall_compiled()")

-- test JIT-to-interpreter dispatch
assert_compile(tailcall_interpreted, "tailcall_interpreted")
assert_equal(3, tailcall_interpreted(), "tailcall_interpreted()")

-- test JIT-to-JIT dispatch
assert_compile(tailcall_compiled, "tailcall_compiled")
assert_equal(3, tailcall_compiled(), "tailcall_compiled()")
