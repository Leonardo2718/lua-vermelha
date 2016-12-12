local test_files = { "arithmetic.lua"
                   , "asserts.lua"
                   , "booleans.lua"
                   , "comparisons.lua"
                   , "goto.lua"
                   , "loads.lua"
                   , "loops.lua"
                   , "move.lua"
                   , "return.lua"
                   , "tables.lua"
                   , "upvaluetable.lua"
                   }

for _,file in ipairs(test_files) do
   pcall(dofile, file)
end
