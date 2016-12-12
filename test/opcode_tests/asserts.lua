--[[
  Module defining asserts that can be used
  in test cases
--]]

local asserts = {}

asserts.assert_equal = function (exp, got, msg)
   assert(exp == got, msg .. " [ " .. tostring(exp) .. " ~= " .. tostring(got) .. " ]")
end

return asserts
