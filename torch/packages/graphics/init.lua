

require('paths')

if package.preload['qt'] then 
   require("qt") 
end
if (qt and qt.qApp and not qt.qApp:runsWithoutGraphics()) then
   -- we are running qlua with graphics enabled
   paths.dofile("wrap-qt.lua")
else
   -- use lcairo
   paths.dofile("wrap-cairo.lua")
end

