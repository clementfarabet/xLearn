----------------------------------------------------------------------
--
-- Copyright (c) 2010 Clement Farabet
-- 
-- Permission is hereby granted, free of charge, to any person obtaining
-- a copy of this software and associated documentation files (the
-- "Software"), to deal in the Software without restriction, including
-- without limitation the rights to use, copy, modify, merge, publish,
-- distribute, sublicense, and/or sell copies of the Software, and to
-- permit persons to whom the Software is furnished to do so, subject to
-- the following conditions:
-- 
-- The above copyright notice and this permission notice shall be
-- included in all copies or substantial portions of the Software.
-- 
-- THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
-- EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
-- MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
-- NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
-- LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
-- OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
-- WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
-- 
----------------------------------------------------------------------
-- description:
--     luaFlow.Array - an abstract cross-platform N-dim array 
--       + uses torch.Tensor as the underlying data struct
--       + difference with torch.Tensor:
--         - allocation: the data is never allocated until it is needed
--         - indexing: indexing is similar to C
--
-- history: 
--     September 11, 2010, 3:50PM - creation
----------------------------------------------------------------------


----------------------------------------------------------------------
-- luaFlow.ArrayT: our basic array class
-- this is the main and only type handled by luaFlow
-- luaFlow.Array() is a shortcut to build new arrays
----------------------------------------------------------------------
luaFlow.ArrayT = {}
luaFlow.ArrayT.mt = {}

function luaFlow.ArrayT.new(...)
   -- init geometry
   local dims = {...}
   if type(dims[1]) == 'table' then
      dims = dims[1]
   end
   local array = {}
   array.type = 'luaFlow.ArrayT'
   array.data = nil
   array.dims = dims
   array.size = 1
   for i = 1,#dims do
      array.size = array.size * dims[i]
   end
   array._idx_ = 0
   -- set metatable for easy operators
   setmetatable(array, luaFlow.ArrayT.mt)
   -- and set other operators
   for i,v in pairs(luaFlow.ArrayT) do
      array[i] = v
   end
   -- register array
   luaFlow.pool.register(array)
   return array
end

function luaFlow.ArrayT.resize(array, ...)
   -- init geometry
   local dims = {...}
   if type(dims[1]) == 'table' then
      dims = dims[1]
   end
   array.dims = dims
   local new_size = 1
   for i = 1,#dims do
      new_size = new_size * dims[i]
   end
   if new_size ~= array.size then
      array.data = nil
   else
      array:alloc()
   end
   array.size = new_size
   return array
end

function luaFlow.ArrayT.setdata(array, tensor)
   array.dims = {}
   for i = 1,tensor:dim() do
      array.dims[tensor:dim()-i+1] = tensor:size(i)
   end
   array.data = tensor
   array.datalin = array.data:storage()
   array.offset = array.data:storageOffset()-1
   array.size = tensor:nElement()
   return array
end

function luaFlow.ArrayT.alloc(array)
   if not array.data then
      local dims = torch.LongStorage(#array.dims)
      for i = 1,#array.dims do dims[#array.dims-i+1] = array.dims[i] end
      array:setdata(torch.Tensor(dims))
   end
   return array
end

function luaFlow.ArrayT.copy(array, src)
   array:alloc()
   if type(src) == 'table' then
      array.data:copy(src.data)
   else
      array.data:copy(src)
   end
   return array
end

function luaFlow.ArrayT.fill(array, ...)
   local src = {...}
   if type(src[1]) == 'table' then
      src = src[1]
   end
   local src_size = #src
   array:alloc()
   for i = 1,array.size do
      array.datalin[array.offset+i] = src[((i-1) % src_size) + 1]
   end
   return array
end

function luaFlow.ArrayT.range(array, start, step, stop)
   start = start or 1
   step = step or 1
   stop = stop or (array.size-start+1)*step
   local current = start
   array:alloc()
   for i = 1,array.size do
      array.datalin[array.offset+i] = current
      current = current + step
      if current > stop then
         current = start
      end
   end
   return array
end

function luaFlow.ArrayT.index(array, idx)
   if type(idx) == 'number' then
      array:alloc()
      if #array.dims == 1 then
         if idx > array.size then
            error('<luaFlow.ArrayT.newindex> index out of bound')
         end
         return array.data[idx]
      else
         if idx > array.dims[1] then
            error('<luaFlow.ArrayT.newindex> index out of bound')
         end
         local new_dims = {}
         for i = 2,#array.dims do
            new_dims[i-1] = array.dims[i]
         end
         local new_array = luaFlow.Array(new_dims)
         new_array:setdata(array.data:select(#array.dims, idx))
         return new_array
      end
   elseif type(idx) == 'string' and (idx:find(':') or idx == '') then
      if idx == '' or idx == ':' then 
         local new_array = luaFlow.Array(array.dims)
         new_array:setdata(array.data)
         new_array._idx_ = array._idx_ + 1
         return new_array
      else
         local _,_,start,stop = string.find(idx,'([%w]+):([%w]+)')
         print(start,stop)
         if not stop then
            local _,_,idx = string.find(idx,'([%w]+):')
            local k = 1
            local new_dims = {}
            for i = 1,#array.dims do
               if array._idx_+1 ~= i then
                  new_dims[k] = array.dims[i]
                  k = k + 1
               end
            end
            local new_array = luaFlow.Array(new_dims)
            new_array:setdata(array.data:select(#array.dims-array._idx_, idx))
            new_array._idx_ = array._idx_
            return new_array
         else
            local new_dims = {}
            new_dims[1] = stop-start+1
            for i = 2,#array.dims do
               new_dims[i] = array.dims[i]
            end
            local new_array = luaFlow.Array(new_dims)
            new_array:setdata(array.data:narrow(#array.dims-array._idx_, start, stop-start+1))
            new_array._idx_ = array._idx_ + 1
            return new_array
         end
      end
   else
      return rawget(array, idx)
   end
end

function luaFlow.ArrayT.newindex(array, idx, val)
   if type(idx)=='number' then
      array:alloc()
      if #array.dims == 1 then
         if idx > array.size then
            error('<luaFlow.ArrayT.newindex> index out of bound')
         end
         array.data[idx] = val
      end
   else
      rawset(array, idx, val)
   end
end

function luaFlow.ArrayT.tostring(array,str,tab)
   local str = '<luaFlow.ArrayT [dim = '
   for i,d in ipairs(array.dims) do
      str = str .. d
      if i ~= #array.dims then str = str .. 'x' end
   end
   str = str .. ']>\n'
   if not array.data then
      str = str .. '[not allocated]'
      return str
   else
      return array:genstr(str)
   end
end

function luaFlow.ArrayT.genstr(array,str,tab)
   if #array.dims > 1 then
      str = str .. '['
      if not tab then
         tab = ' '
      else
         tab = tab .. ' '
      end
      for i = 1,array.dims[1] do
         str = array[i]:genstr(str,tab)
         if i ~= array.dims[1] then str = str  .. '\n' .. tab end
      end
      str = str .. ']'
   else
      str = str .. '['
      for i = 1,array.size do
         str = str .. array.data[i]
         if i ~= array.size then
            str = str .. '  '
         end
      end
      str = str .. ']'
   end
   return str
end


----------------------------------------------------------------------
-- common math functions: +*/-, and trigonometry
----------------------------------------------------------------------
function luaFlow.ArrayT.mul_(array1, array2)
   local result = luaFlow.Array(array1.dims)
   result:copy(array1)
   result:mul(array2)
   return result
end

function luaFlow.ArrayT.add_(array1, array2)
   local result = luaFlow.Array(array1.dims)
   result:copy(array1)
   result:add(array2)
   return result
end

function luaFlow.ArrayT.div_(array1, array2)
   local result = luaFlow.Array(array1.dims)
   result:copy(array1)
   result:div(array2)
   return result
end

function luaFlow.ArrayT.sub_(array1, array2)
   local result = luaFlow.Array(array1.dims)
   result:copy(array1)
   result:sub(array2)
   return result
end

function luaFlow.ArrayT.mul(array1, array2)
   if type(array2) == 'table' then
      -- component wise operation
      array1.data:cmul(array2.data)
   else
      -- scalar operation
      array1.data:mul(array2)
   end
   return array1
end

function luaFlow.ArrayT.add(array1, array2)
   if type(array2) == 'table' then
      -- component wise operation
      array1.data:add(array2.data)
   else
      -- scalar operation
      array1.data:add(array2)
   end
   return array1
end

function luaFlow.ArrayT.div(array1, array2)
   if type(array2) == 'table' then
      -- component wise operation
      array1.data:cdiv(array2.data)
   else
      -- scalar operation
      array1.data:div(array2)
   end
   return array1
end

function luaFlow.ArrayT.sub(array1, array2)
   if type(array2) == 'table' then
      -- component wise operation
      array1.data:add(-array2.data)
   else
      -- scalar operation
      array1.data:add(-array2)
   end
   return array1
end

function luaFlow.ArrayT.cos(array)
   -- component wise operation
   array.data:cos()
   return array
end

function luaFlow.ArrayT.sin(array)
   -- component wise operation
   array.data:sin()
   return array
end

function luaFlow.ArrayT.tan(array)
   -- component wise operation
   array.data:tan()
   return array
end

function luaFlow.ArrayT.cosh(array)
   -- component wise operation
   array.data:tanh()
   return array
end

function luaFlow.ArrayT.sinh(array)
   -- component wise operation
   array.data:tanh()
   return array
end

function luaFlow.ArrayT.tanh(array)
   -- component wise operation
   array.data:tanh()
   return array
end

function luaFlow.ArrayT.acos(array)
   -- component wise operation
   array.data:tanh()
   return array
end

function luaFlow.ArrayT.asin(array)
   -- component wise operation
   array.data:tanh()
   return array
end

function luaFlow.ArrayT.atan(array)
   -- component wise operation
   array.data:tanh()
   return array
end


----------------------------------------------------------------------
-- classical component-wise operators
----------------------------------------------------------------------
function luaFlow.ArrayT.apply(array, mapping)
   -- component wise operation
   array.data:apply(mapping)
   return array
end

function luaFlow.ArrayT.map(array1, array2, mapping)
   -- component wise operation
   array1.data:apply(array2, mapping)
   return array1
end
function luaFlow.ArrayT.floor(array)
   -- component wise operation
   array.data:floor()
   return array
end

function luaFlow.ArrayT.ceil(array)
   -- component wise operation
   array.data:ceil()
   return array
end

function luaFlow.ArrayT.round(array)
   -- component wise operation
   array.data:add(0.5):floor()
   return array
end

function luaFlow.ArrayT.abs(array)
   -- component wise operation
   array.data:abs()
   return array
end

function luaFlow.ArrayT.square(array)
   -- component wise operation
   array.data:cmul(array.data)
   return array
end

function luaFlow.ArrayT.sqrt(array)
   -- component wise operation
   array.data:sqrt()
   return array
end

function luaFlow.ArrayT.log(array)
   -- component wise operation
   array.data:log()
   return array
end

function luaFlow.ArrayT.exp(array)
   -- component wise operation
   array.data:exp()
   return array
end


----------------------------------------------------------------------
-- classical reduce functions (array -> scalar)
----------------------------------------------------------------------
function luaFlow.ArrayT.min(array)
   return array.data:min()
end

function luaFlow.ArrayT.max(array)
   return array.data:max()
end

function luaFlow.ArrayT.mean(array)
   return array.data:mean()
end

function luaFlow.ArrayT.std(array)
   return array.data:std()
end

function luaFlow.ArrayT.var(array)
   return array.data:mean()
end


----------------------------------------------------------------------
-- create aliases / shortcuts
----------------------------------------------------------------------
luaFlow.ArrayT.mt.__add = luaFlow.ArrayT.add_
luaFlow.ArrayT.mt.__mul = luaFlow.ArrayT.mul_
luaFlow.ArrayT.mt.__div = luaFlow.ArrayT.div_
luaFlow.ArrayT.mt.__sub = luaFlow.ArrayT.sub_
luaFlow.ArrayT.mt.__tostring = luaFlow.ArrayT.tostring
luaFlow.ArrayT.mt.__index = luaFlow.ArrayT.index
luaFlow.ArrayT.mt.__newindex = luaFlow.ArrayT.newindex

luaFlow.Array = luaFlow.ArrayT.new
