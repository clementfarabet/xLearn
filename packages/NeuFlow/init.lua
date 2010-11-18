----------------------------------
-- Import all sorts of packages
----------------------------------

-- lua libs
require 'XLearn'
require 'bit'

if not NeuFlowLoaded then

   torch.include('NeuFlow', 'defines.lua')
   torch.include('NeuFlow', 'tools.lua')
   torch.include('NeuFlow', 'Log.lua')
   torch.include('NeuFlow', 'Memory.lua')
   torch.include('NeuFlow', 'Compiler.lua')
   torch.include('NeuFlow', 'Interface.lua')
   torch.include('NeuFlow', 'Core.lua')
   torch.include('NeuFlow', 'CoreUser.lua')
   torch.include('NeuFlow', 'Linker.lua')
   torch.include('NeuFlow', 'Serial.lua')
   torch.include('NeuFlow', 'NeuFlow.lua')

   -- create a path in home dir to store things
   -- like coefficients for example
   home_path = paths.home
   coefpath = home_path..'/.neuflow/coefs'
   os.execute('mkdir -p ' .. coefpath)
   os.execute('chmod a+rw ' .. coefpath)

   -- migrate all the coefficients
   os.execute('cp ' ..  paths.concat(paths.install_lua_path, 'NeuFlow/coef*') .. ' ' .. coefpath)
   os.execute('chmod a+rw ' .. coefpath .. '/*')

   NeuFlowLoaded = true
end
