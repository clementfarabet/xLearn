----------------------------------
-- Import submodules
----------------------------------

if not xFlowLoaded then
   require 'XLearn'
   local xf = 'xFlow'
   -- submodules
   torch.include(xf, 'engine_0.2.lua')
   torch.include(xf, 'lib-math.lua')
   torch.include(xf, 'lib-neural.lua')
   xFlowLoaded = true
end

return xFlow
