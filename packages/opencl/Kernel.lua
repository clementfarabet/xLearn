------------------------------------------------------------
-- A class to handle opencl Kernels.
------------------------------------------------------------
local Kernel = torch.class('opencl.Kernel')

require 'cosmo'
------------------------------------------------------------
-- Creates a Kernel object
------------------------------------------------------------
function Kernel:__init(...)
   toolBox.unpack_class(
      self,
      {...},
      'opencl.Kernel', 
      'creates an opencl kernel',
      {arg='context', type='opengl.Context', help='context for the kernel', req=true},
      {arg='name', type='string', help='function name of the kernel', req=true},
      {arg='source', type='string', help='kernel source code'},
      {arg='template', type='string', help='template for source code'},  
      {arg='lookup', type='string', help='lookup table for template'},      
      {arg='types', type='table', help='type of all the arguments'},
      {arg='args', type='table', help='name of all the arguments'}
   )
   -- build the source
   if self.template ~= nil then
      if self.lookup == nil then
         xerror('If you provide a template you must provide a lookup for the kernel')
      end
      self.source = cosmo.fill(self.template,self.lookup)
   end
   self._ker = self.context:getKernelFromString(self.source,self.name)
end

function Kernel:makeClosure()
   local clos = function(global_size, local_size, args)
                   -- set kernel args
                   for i=1,#self.args do 
                      local idx = i-1
                      local type = self.types[i]
                      type = string.gsub(type,'%s*const%s*','')
                      type = string.gsub(type,'%s*volatile%s*','')
                      type = string.gsub(type,'%s*__global%s*','')
                      type = string.gsub(type,'%s*__local%s*','')
                      type = string.gsub(type,'%s*__constant%s*','')
                      libopencl.setarg(self._ker,idx,type,args[i])
                   end
                   -- push the kernel into the queue
                   return libopencl.enqueue(self.context._context,
                                     self._ker, global_size, local_size)
                end
   return clos
end