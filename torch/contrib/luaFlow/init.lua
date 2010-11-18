----------------------------------
-- Import submodules
----------------------------------

if not luaFlowLoaded then
   local lf = 'luaFlow'
   -- submodules
   torch.include(lf, 'luaflow.lua')
   torch.include(lf, 'cudaflow.lua')
   torch.include(lf, 'neuflow.lua')
   luaFlowLoaded = true
end
