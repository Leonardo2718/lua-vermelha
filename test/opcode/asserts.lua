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
  Module defining asserts that can be used
  in test cases
--]]

local jit = require "lvjit"

local asserts = {}

-- asserts that two values are equal
asserts.assert_equal = function (exp, got, msg)
   assert(exp == got, msg .. " [ " .. tostring(exp) .. " ~= " .. tostring(got) .. " ]")
end

-- compiles a function and asserts that compilation succeeded
asserts.assert_compile = function (f, info)
  assert(jit.compile(f), "failed to compile function "..tostring(f).." ["..info.."]")
end

return asserts
