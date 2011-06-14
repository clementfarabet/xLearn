require 'math'
require 'libopencl'
------------------------------------------------------------
-- A class to handle opencl Tensors.
------------------------------------------------------------
local Tensor = torch.class('opencl.Tensor')

------------------------------------------------------------
-- Creates a Tensor object
------------------------------------------------------------
function Tensor:__init(ctx,dim1,dim2,dim3,dim4,dim5,dim6)
   if not ctx then 
      -- no params
      self.context = opencl.getContext{}
      self.usage = 'Tensor initiated with no dimensions, provide dimension for more info'
   elseif (torch.typename(ctx) == 'opencl.Context') then
      -- no table params {...}
      self.context = ctx
      self.usage = 'Tensor not initiated with a table params, use Tensor{} instead to construct Tensor'
      self.dim1 = dim1
      self.dim2 = dim2
      self.dim3 = dim3
      self.dim4 = dim4
      self.dim5 = dim5
      self.dim6 = dim6
   else
      local args = ctx
      toolBox.unpack_class(
         self,
         args,
         'opencl.Tensor', 
         'creates an opencl tensor',
         {arg='context', type='opencl.Context', help='opencl context', req=true},
         {arg='dim1', type='number', help='dimension 1'},
         {arg='dim2', type='number', help='dimension 2'},
         {arg='dim3', type='number', help='dimension 3'},
         {arg='dim4', type='number', help='dimension 4'},
         {arg='dim5', type='number', help='dimension 5'},
         {arg='dim6', type='number', help='dimension 6'}
      )
   end
   -- nDims
   self._nDims = 0
   if self.dim1 then self._nDims = self._nDims + 1 
      if self.dim2 then self._nDims = self._nDims + 1 
         if self.dim3 then self._nDims = self._nDims + 1 
            if self.dim4 then self._nDims = self._nDims + 1  
               if self.dim5 then self._nDims = self._nDims + 1  
                  if self.dim6 then self._nDims = self._nDims + 1 
   end end end end end end
   -- nElement
   self._nElement = 1
   self._size = torch.LongStorage(self._nDims)
   if self.dim1 then
      -- allocate on the gpu
      self.data = libopencl.Tensor(self.context._context,
                                   self.dim1,self.dim2,self.dim3,
                                   self.dim4,self.dim5,self.dim6)
      -- compute nb elements
      for i=1,self._nDims do
         self._nElement = self._nElement * self['dim'..i]
         self._size[i] = self['dim'..i]
      end
   else -- build empty tensor
      self._nElement = 0
   end
   self._mt = getmetatable(self.data)
   self.usage = self.usage ..'\n'.. tostring(self.data)
   self._lastkernel = {}
end

function Tensor:_cleardims()
   self._size = nil
   self._nDims = nil
   self._nElement = nil
   self.dim1 = nil
   self.dim2 = nil
   self.dim3 = nil
   self.dim4 = nil
   self.dim5 = nil
   self.dim6 = nil
end

-- check if 2 longtensors are the same
local function is_same_size(size1,size2)
   if size1:size() ~= size2:size() then
      return false
   end
   for i=1,size1:size() do
      if size1[i] ~= size2[i] then return false end
   end
   return true
end

function Tensor:resize(dim,dim2,dim3,dim4,dim5,dim6)
   if not dim then
      xerror('<opencl.Tensor:resize> you need to provide 1 to 6 dims')
   end
   if (torch.typename(dim) == 'torch.LongStorage') then
      -- first check if resize is needed
      if is_same_size(self._size,dim) then
         return
      end
      self:_cleardims()
      if dim:size()<1 or dim:size()>6 then
         xerror('<opencl.Tensor:resize> tensor size must have from 1 to 6 dims')
      end
      self._nDims = dim:size()
      self._size = torch.LongStorage(self._nDims):copy(dim)
      -- compute nb elements
      self._nElement = 1
      for i=1,self._nDims do
         self['dim'..i] = self._size[i]
         self._nElement = self._nElement * self['dim'..i]
      end
      if self.dim1 < 1 then
         xerror('<opencl.Tensor:resize> the first dimension is too small')
      end
      -- allocate on the gpu
      self.data = libopencl.Tensor(self.context._context,
                                   self.dim1,self.dim2,self.dim3,
                                   self.dim4,self.dim5,self.dim6)
   else
      -- nDims
      local nbdims = 0
      local tp ={}
      if dim then nbdims = nbdims + 1 
         tp.dim1 = dim
         if dim2 then nbdims = nbdims + 1 
            tp.dim2 = dim2
            if dim3 then nbdims = nbdims + 1 
               tp.dim3 = dim3
               if dim4 then nbdims = nbdims + 1  
                  tp.dim4 = dim4
                  if dim5 then nbdims = nbdims + 1  
                     tp.dim5 = dim5
                     if dim6 then nbdims = nbdims + 1 
                        tp.dim6 = dim6
      end end end end end end
      local lsize = torch.LongStorage(nbdims)
      -- compute nb elements
      for i=1,nbdims do
         lsize[i] = tp['dim'..i]
      end
      -- check if resize is needed
      if is_same_size(self._size,lsize) then
         return
      end
      self:_cleardims()
      self._nDims = nbdims
      self._size = torch.LongStorage(self._nDims)
      self._nElement = 1
      -- compute size elements
      for i=1,nbdims do
         self['dim'..i] = tp['dim'..i]
         self._nElement = self._nElement * self['dim'..i]
         self._size[i] = self['dim'..i]
      end
      if self.dim1 < 1 then
         xerror('<opencl.Tensor:resize> the first dimension is too small')
      end
      -- allocate on the gpu
      self.data = libopencl.Tensor(self.context._context,
                                   self.dim1,self.dim2,self.dim3,
                                   self.dim4,self.dim5,self.dim6)
   end
   self._mt = getmetatable(self.data)
   -- free old stuff
   collectgarbage()
end

function Tensor:resizeAs(tensor)
   if is_same_size(self._size,tensor:size()) then
      return
   end
   self:_cleardims()
   self._nDims = tensor:size():size()
   self._size = torch.LongStorage(self._nDims):copy(tensor:size())
   -- compute nb elements
   self._nElement = 1
   for i=1,self._nDims do
      self['dim'..i] = self._size[i]
      self._nElement = self._nElement * self['dim'..i]
   end
   if self.dim1 < 1 then
      xerror('<opencl.Tensor:resize> the first dimension is too small')
   end
   -- allocate on the gpu
   self.data = libopencl.Tensor(self.context._context,
                                self.dim1,self.dim2,self.dim3,
                                self.dim4,self.dim5,self.dim6)
   self._mt = getmetatable(self.data)
   -- free old stuff
   collectgarbage()
end

------------------------------------------------------------
-- nDimension: same behaviour as torch.Tensor:nDimension
------------------------------------------------------------
function Tensor:nDimension()
   return self._nDims
end
function Tensor:dim()
   return self._nDims
end

------------------------------------------------------------
-- size: same behaviour as torch.Tensor:size
------------------------------------------------------------
function Tensor:size(idx)
   if type(idx) == 'number' then
      return self._size[idx]
   else
      return self._size      
   end
end

------------------------------------------------------------
-- nElement: same behaviour as torch.Tensor:nDimension
------------------------------------------------------------
function Tensor:nElement()
   return self._nElement
end

------------------------------------------------------------
-- return block/grid size for kernel execution
------------------------------------------------------------
function Tensor:get_sizes()
   local max_work_items = math.min(128, self.context:getMaxWorkGroupSize())
   local min_work_items = math.min(32, max_work_items)
   local max_groups = self.context:getMaxComputeUnits() * 4 * 8
   -- 4 to overfill the device
   -- 8 is an Nvidia constant that's how many
   -- groups fit onto one compute device
   local group_count, work_items_per_group
   if self._nElement < min_work_items then
      group_count = 1
      work_items_per_group = min_work_items
   elseif self._nElement < (max_groups * min_work_items) then
      group_count = math.floor((self._nElement + min_work_items - 1) 
                            / min_work_items)
      work_items_per_group = min_work_items
   elseif self._nElement < (max_groups * max_work_items) then
      group_count = max_groups
      local grp = math.floor((self._nElement + min_work_items - 1) 
                    / min_work_items)
      work_items_per_group = (grp + max_groups -1) / max_groups
      work_items_per_group = math.floor(work_items_per_group) * min_work_items
   else
      group_count = max_groups
      work_items_per_group = max_work_items
   end
   --print('TensorCL getsize: nElemt',n, 'gs',group_count, 'ls',work_items_per_group)
   -- return global_size and local_size for launching a kernel
   local gs = torch.LongTensor(3):fill(1)
   gs[1] = group_count*work_items_per_group
   local ls = torch.LongTensor(3):fill(1)
   ls[1] = work_items_per_group
   return gs,ls
end

------------------------------------------------------------
-- add: same behaviour as torch.Tensor:add
------------------------------------------------------------
function Tensor:add(thing, tens)
   local gs, ls = self:get_sizes()
   if tens == nil then
      if type(thing) == 'number' then
         -- memoise ADD VALUE
         if not self._addval then
            self._addval = self.context:get_elwise_kernel{
               types = {'__global float*','float','unsigned int'},
               argsnames = {'z', 'val', 'n'},
               operation = 'z[i] = z[i] + val',
               name = 'add_val'
            }
         end
         local args = {self.data, thing, self._nElement}
         self._lastkernel.duration = self._addval(gs, ls, args)
      elseif torch.typename(thing) == 'opencl.Tensor' then
         -- memoise ADD TENSOR
         if not self._addtensor then
            if self._nElement ~= thing._nElement then
               xerror('<opencl.Tensor:add> number of elements must match.')
            end
            self._addtensor = self.context:get_elwise_kernel{
               types = {'__global float*','__global float*','unsigned int'},
               argsnames = {'z', 'y', 'n'},
               operation = 'z[i] = z[i] + y[i]',
               name = 'add_tensor'
            }
         end
         local args = {self.data, thing.data, self._nElement}
         self._lastkernel.duration = self._addtensor(gs, ls, args)
      else
         xerror('<opencl.Tensor:add> only handle number or opencl.Tensor as first arg')
      end
   elseif torch.typename(tens) == 'opencl.Tensor' then
      if type(thing) == 'number' then
         -- memoise ADD VALUE+TENSOR
         if not self._addvaltens then
            self._addvaltens = self.context:get_elwise_kernel{
               types = {'__global float*','__global float*','float','unsigned int'},
               argsnames = {'z', 'y', 'val', 'n'},
               operation = 'z[i] = z[i] + y[i] + val',
               name = 'add_valtens'
            }
         end
         local args = {self.data, tens.data, thing, self._nElement}
         self._lastkernel.duration = self._addvaltens(gs, ls, args)
      else
         xerror('<opencl.Tensor:add> if you provide a opencl.Tensor, first arg must be number')
      end
   else
      xerror('<opencl.Tensor:add> only handle opencl.Tensor as second arg')
   end
   self._lastkernel.name = 'add'
   return self
end

------------------------------------------------------------
-- mul like with torch.Tensor
------------------------------------------------------------
function Tensor:mul(val)
   -- memoise
   if not self._mul then
      self._mul = self.context:get_elwise_kernel{
         types = {'__global float*','float','unsigned int'},
         argsnames = {'z', 'val', 'n'},
         operation = 'z[i]*=val',
         name = 'mul'
      }
   end
   local gs, ls = self:get_sizes()
   local args = {self.data, val, self._nElement}
   self._lastkernel.duration = self._mul(gs, ls, args)
   self._lastkernel.name = 'mul'
   return self
end

------------------------------------------------------------
-- cmul like with torch.Tensor
------------------------------------------------------------
function Tensor:cmul(tens)
   if self._nElement ~= tens._nElement then
      xerror('<opencl.Tensor:cmul> number of elements must match.')
   end
   -- memoise
   if not self._cmul then
      self._cmul = self.context:get_elwise_kernel{
         types = {'__global float*','__global float*','unsigned int'},
         argsnames = {'z', 'y', 'n'},
         operation = 'z[i]*=y[i]',
         name = 'cmul'
      }
   end
   local gs, ls = self:get_sizes()
   local args = {self.data, tens.data, self._nElement}
   self._lastkernel.duration = self._cmul(gs, ls, args)
   self._lastkernel.name = 'cmul'
   return self
end

------------------------------------------------------------
-- addcmul like with torch.Tensor
------------------------------------------------------------
function Tensor:addcmul(val, tens1, tens2)
   if self._nElement ~= tens1._nElement or 
      self._nElement ~= tens2._nElement then
      xerror('<opencl.Tensor:addcmul> number of elements must match.')
   end
   -- memoise
   if not self._addcmul then
      self._addcmul = self.context:get_elwise_kernel{
         types = {'__global float*','__global float*','__global float*','float','unsigned int'},
         argsnames = {'x','z', 'y', 'val', 'n'},
         operation = 'x[i] += val * z[i]*y[i]',
         name = 'addcmul'
      }
   end
   local gs, ls = self:get_sizes()
   local args = {self.data, tens1.data, tens2.data, val, self._nElement}
   self._lastkernel.duration = self._addcmul(gs, ls, args)
   self._lastkernel.name = 'addcmul'
   return self
end

------------------------------------------------------------
-- div like with torch.Tensor
------------------------------------------------------------
function Tensor:div(val)
   -- memoise
   if not self._div then
      self._div = self.context:get_elwise_kernel{
         types = {'__global float*','float','unsigned int'},
         argsnames = {'z', 'val', 'n'},
         operation = 'z[i]/=val',
         name = 'div'
      }
   end
   local gs, ls = self:get_sizes()
   local args = {self.data, val, self._nElement}
   self._lastkernel.duration = self._div(gs, ls, args)
   self._lastkernel.name = 'div'
   return self
end

------------------------------------------------------------
-- cdiv like with torch.Tensor
------------------------------------------------------------
function Tensor:cdiv(tens)
   if self._nElement ~= tens._nElement then
      xerror('<opencl.Tensor:cdiv> number of elements must match.')
   end
   -- memoise
   if not self._cdiv then
      self._cdiv = self.context:get_elwise_kernel{
         types = {'__global float*','__global float*','unsigned int'},
         argsnames = {'z', 'y', 'n'},
         operation = 'z[i]/=y[i]',
         name = 'cdiv'
      }
   end
   local gs, ls = self:get_sizes()
   local args = {self.data, tens.data, self._nElement}
   self._lastkernel.duration = self._cdiv(gs, ls, args)
   self._lastkernel.name = 'cdiv'
   return self
end

------------------------------------------------------------
-- addcdiv like with torch.Tensor
------------------------------------------------------------
function Tensor:addcdiv(val, tens1, tens2)
   if self._nElement ~= tens1._nElement or 
      self._nElement ~= tens2._nElement then
      xerror('<opencl.Tensor:addcdiv> number of elements must match.')
   end
   -- memoise
   if not self._addcdiv then
      self._addcdiv = self.context:get_elwise_kernel{
         types = {'__global float*','__global float*','__global float*','float','unsigned int'},
         argsnames = {'x','z', 'y', 'val', 'n'},
         operation = 'x[i] += val * z[i]/y[i]',
         name = 'addcdiv'
      }
   end
   local gs, ls = self:get_sizes()
   local args = {self.data, tens1.data, tens2.data, val, self._nElement}
   self._lastkernel.duration = self._addcdiv(gs, ls, args)
   self._lastkernel.name = 'addcdiv'
   return self
end

------------------------------------------------------------
-- fill
------------------------------------------------------------
function Tensor:fill(val)
   -- memoise
   if not self._fill then
      self._fill = self.context:get_elwise_kernel{
         types = {'__global float*','float','unsigned int'},
         argsnames = {'z', 'val', 'n'},
         operation = 'z[i]=val',
         name = 'fill'
      }
   end
   local gs, ls = self:get_sizes()
   local args = {self.data, val, self._nElement}
   self._lastkernel.duration = self._fill(gs, ls, args)
   self._lastkernel.name = 'fill'
   return self
end

------------------------------------------------------------
-- fill the tensor with values starting at start with increment step 
------------------------------------------------------------
function Tensor:arange(start,step)
   -- memoise
   if not self._arange then
      self._arange = self.context:get_elwise_kernel{
         types = {'__global float*','float','float','unsigned int'},
         argsnames = {'z', 'start', 'step', 'n'},
         operation = 'z[i]=start+i*step',
         name = 'arange'
      }
   end
   local gs, ls = self:get_sizes()
   local args = {self.data, start, step, self._nElement}
   self._lastkernel.duration = self._arange(gs, ls, args)
   self._lastkernel.name = 'arange'
   return self
end


------------------------------------------------------------
-- apply f(x) elementwise
------------------------------------------------------------
function Tensor:elwiseApply(cmd)
   if cmd == nil then
      xerror("Command should be like 'z[i] = f(z[i])' where f is the function of your choice")
   end
   local kernel = self.context:get_elwise_kernel{
      types = {'__global float*','unsigned int'},
      argsnames = {'z', 'n'},
      operation = cmd,--e.g. 'z[i]=tanh(z[i])',
      name = 'apply'
   }
   local gs, ls = self:get_sizes()
   local args = {self.data, self._nElement}
   kernel(gs, ls, args)
   return self
end

------------------------------------------------------------
-- fill with 0s
------------------------------------------------------------
function Tensor:zero()
   return self:fill(0)
end
------------------------------------------------------------
-- return cpu Tensor
------------------------------------------------------------
function Tensor:get()
  return self._mt.getLuaTensor(self.data)
end

------------------------------------------------------------
-- from torch Tensor
------------------------------------------------------------
function Tensor:copy(torchTensor)
  self._mt.copy(self.data,torchTensor)
  return self
end

function Tensor:__tostring__()
   if self.data then
      return self:get():__tostring__() .. "\n** opencl.Tensor view on CPU **\n"
   else
      return "\n** opencl.Tensor is empty **\n"
   end
end


------------------------------------------------------------
-- overloaded ops
------------------------------------------------------------
function Tensor:__unm__()
   print('minus myslef')
end