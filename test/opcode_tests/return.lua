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
  Test for RETURN opcode

  Each function defined exercises RETURN in
  a slightly different way. `empty()` has an empty
  body, meaning it will only have the default
  RETURN that is inserted by `luac`. `justReturn()`
  has an explicit return statement. It will
  therefore have onr RETURN opcodes for the
  explicit statement, and one that is implicitly
  added by `luac`. A test passes if the expected
  value is returned by the function call (`nil` if
  no return value is specified).
--]]

local asserts = require "asserts"
local assert_equal = asserts.assert_equal

local function empty() end
local function justReturn() return end

assert_equal(nil, empty(), "empty()")
assert_equal(nil, justReturn(), "justReturn()")
