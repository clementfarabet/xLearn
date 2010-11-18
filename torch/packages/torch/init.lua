
-- We are using paths.require to appease mkl
--require "paths"
--paths.require "libtorch"
require "libtorch"

--- package stuff
function torch.packageLuaPath(name)
   if not name then
      local ret = string.match(torch.packageLuaPath('torch'), '(.*)/')
       if not ret then --windows?
           ret = string.match(torch.packageLuaPath('torch'), '(.*)\\')
       end
       return ret 
   end
   for path in string.gmatch(package.path, "(.-);") do
      path = string.gsub(path, "%?", name)
      local f = io.open(path)
      if f then
         f:close()
         local ret = string.match(path, "(.*)/")
         if not ret then --windows?
             ret = string.match(path, "(.*)\\")
         end
         return ret
      end
   end
   local path = string.match(package.path, "([^;]-)$")
   path = string.gsub(path, "%?", name)
   local f = io.open(path)
   if f then
      f:close()
      local ret = string.match(path, "(.*)/")
      if not ret then --windows?
         ret = string.match(path, "(.*)\\")
      end
      return ret
   end
end

function torch.include(package, file)
   dofile(torch.packageLuaPath(package) .. '/' .. file) 
end

function torch.class(tname, parenttname)

   local function constructor(...)
      local self = {}
      torch.setmetatable(self, tname)
      if self.__init then
         self:__init(...)
      end
      return self
   end
   
   local function factory()
      local self = {}
      torch.setmetatable(self, tname)
      return self
   end

   local mt = torch.newmetatable(tname, parenttname, constructor, nil, factory)
   local mpt
   if parenttname then
      mpt = torch.getmetatable(parenttname)
   end
   return mt, mpt
end

torch.include('torch', 'Tensor.lua')
torch.include('torch', 'File.lua')

if not package.loaded.help then
   require('help')
end

return torch
