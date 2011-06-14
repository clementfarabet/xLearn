------------------------------------------------------------
-- A class to handle opencl Contexts.
------------------------------------------------------------
local Context = torch.class('opencl.Context')
Context.memoise_tag = 'memoised_ker_'
------------------------------------------------------------
-- Creates a Context object
------------------------------------------------------------
function Context:__init(...)
   toolBox.unpack_class(
      self,
      {...},
      'opencl.Context', 
      'creates an opencl context',
      {arg='platformName', type='string', help='name of the platform'},
      {arg='deviceName', type='string', help='name of the device'},
      {arg='index', type='number', help='device index',default=0},
      {arg='profiling', type='boolean', help='enable profiling'}
   )
   
   if self.platformName == nil and  self.deviceName == nil and self.index == nil then
      xerror('you must provide at least: platformName | deviceName | index',
             'opencl.Context',self.usage)
   end

   self._context = libopencl.Context(self.platformName,self.deviceName,
                                     self.index,self.profiling)
   self._mt = getmetatable(self._context)
end

function Context:getKernelFromString(...)
   local args, source, kername = toolBox.unpack(
      {...},
      'opencl. Context:getKernelFromString', 
      'creates a kernel from source',
      {arg='source', type='string', help='source code of the kernel', req=true},
      {arg='kerFunctionName', type='string', help='name of the kernel', req=true}
   )
   return self._mt.buildKernelFromSource(self._context,source,kername)
end

function Context:getMaxComputeUnits()
   return self._mt.getMaxComputeUnits(self._context)
end

function Context:getMaxWorkGroupSize()
   return self._mt.getMaxWorkGroupSize(self._context)
end

function Context:getMaxWorkItemSize()
   return self._mt.getMaxWorkItemSize(self._context)
end

function Context:get_elwise_kernel(...)
   local args = toolBox.unpack(
      {...},
      'Context:get_elwise_program',
      'JIT compile a kernel and return it',
      {arg='types', type='table of string', help='type of arguments of the kernel call', req=true},
      {arg='argsnames', type='table of string', help='names of arguments of the kernel call', req=true},
      {arg='operation', type='string', help='one line operation to apply', req=true},
      {arg='name', type='string', help='kernel function name', default='elwise_kernel'},
      {arg='preamble', type='string', help='code before running kernel'},
      {arg='loop_prep', type='string', help='code in the kernel before the loop'},
      {arg='after_loop', type='string', help='code in the kernel after the loop'}
   )
   -- memoisation
   if args.name ~= nil and self[Context.memoise_tag..args.name] ~= nil then
      return self[Context.memoise_tag..args.name] 
   end
   if #args.types ~= #args.argsnames then
      xerror('You must provide types for every argsnames element')
   end
   local tmpl = 
      [[ $preamble
         __kernel void $name($arguments)
         {
            unsigned lid = get_local_id(0);
            unsigned gsize = get_global_size(0);
            unsigned work_item_start = get_local_size(0)*get_group_id(0);
            unsigned i;
            
            $loop_prep;
            
            for (i = work_item_start + lid; i < n; i += gsize)
            {
               $operation;
            }
            
            $after_loop;
         }]] 

   -- build values for templating
   local arguments = ''
   for i=1,#args.argsnames do
      arguments = arguments .. args.types[i] ..' '
      arguments = arguments .. args.argsnames[i]
      if i<#args.argsnames then 
         arguments = arguments .. ', '
      end
   end
   local lookup = {preamble = args.preamble,
                   name = args.name,
                   arguments = arguments,
                   loop_prep = args.loop_prep,
                   operation = args.operation,
                   after_loop = args.after_loop}
                   
   -- Todo memoise this  to the self table and warn if overwrite
   local ker = opencl.Kernel{context = self,
                             name = args.name,
                             template = tmpl,
                             lookup = lookup,
                             types = args.types,
                             args = args.argsnames}
   self[Context.memoise_tag..args.name] = ker:makeClosure()
   self["source_"..args.name] = ker.source
   return self[Context.memoise_tag..args.name] 
end