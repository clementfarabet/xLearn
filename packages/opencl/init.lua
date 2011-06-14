--==============================================================================
-- File: sys
--
-- Description: A wrapper for using opencl
--              
--
-- Created: March 17, 2011, 5:37PM
--
-- Author: Benoit Corda -- corda@cs.nyu.edu
--==============================================================================

require 'XLearn'
require 'libopencl'

if not opencl then
   ------------------------------------------------------------
   -- Wrapper for opencl
   --
   -- To load: require 'opencl'
   --
   -- @release 2011 Benoit Corda
   ------------------------------------------------------------
   local print = print
   local xerror = xerror
   local toolBox = toolBox
   local libopencl = libopencl
   local torch = torch
   local string = string
   local math = math
   module('opencl')
   ------------------------------------------------------------
   -- Constants
   ------------------------------------------------------------
   BLOCK_W = 16
   BLOCK_H = 8

   ------------------------------------------------------------
   -- Include classes
   ------------------------------------------------------------
   torch.include('opencl', 'Kernel.lua')
   torch.include('opencl', 'Context.lua')
   torch.include('opencl', 'Tensor.lua')
   torch.include('opencl', 'CLSpatialConvolution.lua')

   ------------------------------------------------------------
   -- Create context
   ------------------------------------------------------------
   getContext = function(...)
                   local args, platformName, deviceName, idx, profiling = toolBox.unpack(
                      {...},
                      'opencl.getContext',
                      'create an opencl context of the given platform/device',
                      {arg='platformName', type='string', help='name of the platform'},
                      {arg='deviceName', type='string', help='name of the device'},
                      {arg='index', type='number', help='device index',default=0},
                      {arg='profiling', type='boolean', help='enable profiling'}
                   )
                   return Context{platformName=platformName,
                                  deviceName=deviceName,
                                  index=idx,
                                  profiling=profiling}
                end
   ------------------------------------------------------------
   -- set kernel's argument for kernel call
   ------------------------------------------------------------
   setarg = function(...)
               local args, kernel, index, type, val = toolBox.unpack(
                  {...},
                  'opencl.setarg',
                  "set kernel's arguments for kernel call",
                  {arg='kernel', type='opencl.Kernel', help='the kernel', req=true},
                  {arg='index', type='number', help='index of the arg (0-based)', req=true},
                  {arg='type', type='string', help='type of the arg', req=true},
                  {arg='val', type='arg_type', help='the actual value to pass', req=true}
               )
               -- some clean up
               type = string.gsub(type,'%s*const%s*','')
               type = string.gsub(type,'%s*volatile%s*','')
               type = string.gsub(type,'%s*__global%s*','')
               type = string.gsub(type,'%s*__local%s*','')
               type = string.gsub(type,'%s*__constant%s*','')
               libopencl.setarg(kernel._ker,index,type,val)
            end

   ------------------------------------------------------------
   -- Run an elementwise kernel on your device
   ------------------------------------------------------------
   test_elementwise_kernel = function()
     kernel_str = [[
           __kernel void fill(__global float* z, float val, unsigned int n)
           {
              unsigned lid = get_local_id(0);
              unsigned gsize = get_global_size(0);
              unsigned work_item_start = get_local_size(0)*get_group_id(0);
              unsigned i;
              
              ;
            
              for (i = work_item_start + lid; i < n; i += gsize)
              {
                 z[i]=val;
              }
              
              ;
           }
     ]]
     local ctx
     if toolBox.OS == 'macos' then
        ctx = getContext{platformName='Apple'}
     elseif toolBox.OS == 'linux' then
        ctx = getContext{platformName='NVIDIA'}
     end
     local tensor = Tensor(ctx,10,10,2)
     gs, ls = tensor:get_sizes()
     local fillval = math.random()*100
     libopencl.runElwiseWithVal(ctx._context, kernel_str, 'fill', gs, ls, tensor.data, fillval)
     print(tensor:get())
     print('<test_elementwise_kernel> TensorCL filled with',fillval)
  end
   ------------------------------------------------------------
   -- Computes a simple add vectors on your device
   ------------------------------------------------------------
   testme = function()
               kernel_str = [[__kernel void sum(
                        __global const float *a,
                        __global const float *b, 
                        __global float *c)
                     {
                        int gid = get_global_id(0);
                        c[gid] = a[gid] + b[gid];
                     }]]
               local ctx
               if toolBox.OS == 'macos' then
                  ctx = getContext{platformName='Apple'}
               elseif toolBox.OS == 'linux' then
                  ctx = getContext{platformName='NVIDIA'}
               end
               ker = Kernel{context=ctx, source=kernel_str, name='sum'}
               libopencl.test(ctx._context,ker._ker,2)
            end
end

return opencl
