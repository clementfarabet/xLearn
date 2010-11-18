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
--     luaFlow - an efficient flow-graph extension for Lua
--
-- history: 
--     September 11, 2010, 3:50PM - creation
----------------------------------------------------------------------

luaflow = {}

----------------------------------------------------------------------
-- luaflow.ArrayT: our basic array class
-- this is the main and only type handled by luaFlow
-- luaflow.Array() is a shortcut to build new arrays
----------------------------------------------------------------------
luaflow.ArrayT = {}
luaflow.ArrayT.mt = {}

function luaflow.ArrayT.new(...)
   -- init geometry
   local dims = {...}
   if type(dims[1]) == 'table' then
      dims = dims[1]
   end
   local array = {}
   array.type = 'luaflow.ArrayT'
   array.data = {}
   array.data_start = 1
   array.dims = dims
   array.size = 1
   for i = 1,#dims do
      array.size = array.size * dims[i]
   end
   array.data_end = array.data_start + array.size - 1
   -- set metatable for easy operators
   setmetatable(array, luaflow.ArrayT.mt)
   -- and set other operators
   for i,v in pairs(luaflow.ArrayT) do
      array[i] = v
   end
   return array
end

function luaflow.ArrayT.resize(array, ...)
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
      array.data = {}
      array.data_start = 1
   end
   array.size = new_size
   array.data_end = array.data_start + array.size - 1
   return array
end

function luaflow.ArrayT.copy(dest, src)
   for i = 1,src.size do
      dest.data[i] = src.data[i]
   end
   return dest
end

function luaflow.ArrayT.fill(dest, ...)
   local src = {...}
   if type(src[1]) == 'table' then
      src = src[1]
   end
   local src_size = #src
   for i = dest.data_start,dest.data_end do
      dest.data[i] = src[((i-1) % src_size) + 1]
   end
   return dest
end

function luaflow.ArrayT.range(dest, start, step, stop)
   start = start or 1
   step = step or 1
   stop = stop or (dest.size-start+1)*step
   local current = start
   for i = dest.data_start,dest.data_end do
      dest.data[i] = current
      current = current + step
      if current > stop then
         current = start
      end
   end
   return dest
end

function luaflow.ArrayT.index(array, idx)
   if type(idx)=='number' then
      if #array.dims == 1 then
         local data_idx = array.data_start + idx - 1
         if data_idx > array.data_end then
            error('<luaflow.ArrayT.newindex> index out of bound')
         end
         return array.data[data_idx]
      else
         if idx > array.dims[1] then
            error('<luaflow.ArrayT.newindex> index out of bound')
         end
         local new_dims = {}
         for i = 2,#array.dims do
            new_dims[i-1] = array.dims[i]
         end
         local new_array = luaflow.ArrayT.new(new_dims)
         new_array.data = array.data
         new_array.data_start = array.data_start + new_array.size * (idx-1)
         new_array.data_end = new_array.data_start + new_array.size - 1
         return new_array
      end
   else
      return rawget(array, idx)
   end
end

function luaflow.ArrayT.newindex(array, idx, val)
   if type(idx)=='number' then
      if #array.dims == 1 then
         local data_idx = array.data_start + idx - 1
         if data_idx > array.data_end then
            error('<luaflow.ArrayT.newindex> index out of bound')
         end
         array.data[data_idx] = val
      end
   else
      rawset(array, idx, val)
   end
end

function luaflow.ArrayT.mul_(array1, array2)
   local result = luaflow.ArrayT.new(array1.dims)
   for i = array1.data_start,array1.data_end do
      result.data[i] = array1.data[i] * array2.data[i]
   end
   return result
end

function luaflow.ArrayT.add_(array1, array2)
   local result = luaflow.ArrayT.new(array1.dims)
   for i = array1.data_start,array1.data_end do
      result.data[i] = array1.data[i] + array2.data[i]
   end
   return result
end

function luaflow.ArrayT.div_(array1, array2)
   local result = luaflow.ArrayT.new(array1.dims)
   for i = array1.data_start,array1.data_end do
      result.data[i] = array1.data[i] / array2.data[i]
   end
   return result
end

function luaflow.ArrayT.sub_(array1, array2)
   local result = luaflow.ArrayT.new(array1.dims)
   for i = array1.data_start,array1.data_end do
      result.data[i] = array1.data[i] - array2.data[i]
   end
   return result
end

function luaflow.ArrayT.mul(array1, array2)
   if type(array2) == 'table' then
      -- component wise operation
      for i = array1.data_start,array1.data_end do
         array1.data[i] = array1.data[i] * array2.data[i]
      end
   else
      -- scalar operation
      for i = array1.data_start,array1.data_end do
         array1.data[i] = array1.data[i] * array2
      end      
   end
   return array1
end

function luaflow.ArrayT.add(array1, array2)
   if type(array2) == 'table' then
      -- component wise operation
      for i = array1.data_start,array1.data_end do
         array1.data[i] = array1.data[i] + array2.data[i]
      end
   else
      -- scalar operation
      for i = array1.data_start,array1.data_end do
         array1.data[i] = array1.data[i] + array2
      end      
   end
   return array1
end

function luaflow.ArrayT.div(array1, array2)
   if type(array2) == 'table' then
      -- component wise operation
      for i = array1.data_start,array1.data_end do
         array1.data[i] = array1.data[i] / array2.data[i]
      end
   else
      -- scalar operation
      for i = array1.data_start,array1.data_end do
         array1.data[i] = array1.data[i] / array2
      end      
   end
   return array1
end

function luaflow.ArrayT.sub(array1, array2)
   if type(array2) == 'table' then
      -- component wise operation
      for i = array1.data_start,array1.data_end do
         array1.data[i] = array1.data[i] - array2.data[i]
      end
   else
      -- scalar operation
      for i = array1.data_start,array1.data_end do
         array1.data[i] = array1.data[i] - array2
      end      
   end
   return array1
end

function luaflow.ArrayT.apply(array, mapping)
   -- component wise operation
   for i = array.data_start,array.data_end do
      array.data[i] = mapping(array.data[i])
   end
   return array
end

function luaflow.ArrayT.cos(array)
   -- component wise operation
   for i = array.data_start,array.data_end do
      array.data[i] = math.cos(array.data[i])
   end
   return array
end

function luaflow.ArrayT.sin(array)
   -- component wise operation
   for i = array.data_start,array.data_end do
      array.data[i] = math.sin(array.data[i])
   end
   return array
end

function luaflow.ArrayT.tan(array)
   -- component wise operation
   for i = array.data_start,array.data_end do
      array.data[i] = math.tan(array.data[i])
   end
   return array
end

function luaflow.ArrayT.tanh(array)
   -- component wise operation
   for i = array.data_start,array.data_end do
      array.data[i] = math.tanh(array.data[i])
   end
   return array
end

function luaflow.ArrayT.conv2D(dest, src, kernel, mode)
   dest:resize(src.dims[1]-kernel.dims[1]+1, src.dims[2]-kernel.dims[2]+1)
   if not (mode and (mode == 'acc' or mode == true)) then
      dest:fill(0)
   end
   local ker_p = kernel.data_start
   for m = 0,kernel.dims[1]-1 do
      for n = 0,kernel.dims[2]-1 do
         local dest_p = dest.data_start
         local src_p = src.data_start + m*src.dims[2] + n
         local weight = kernel.data[ker_p]
         for i = 0,dest.dims[1]-1 do
            for j = 0,dest.dims[2]-1 do
               dest.data[dest_p+j] = dest.data[dest_p+j] + src.data[src_p+j]*weight
            end
            dest_p = dest_p + dest.dims[2]
            src_p = src_p + dest.dims[2]
         end
         ker_p = ker_p + 1
      end
   end
end

function luaflow.ArrayT.tostring(array,str,tab)
   if #array.dims > 1 then
      if str then 
         str = str .. '['
      else
         str = '['
      end
      if not tab then
         tab = ' '
      else
         tab = tab .. ' '
      end
      for i = 1,array.dims[1] do
         str = luaflow.ArrayT.tostring(array[i],str,tab)
         if i ~= array.dims[1] then str = str  .. '\n' .. tab end
      end
      str = str .. ']'
   else
      if not str then
         str = '['
      else
         str = str .. '['
      end
      for i = array.data_start,array.data_end do
         str = str .. (array.data[i] or '-')
         if i ~= array.data_end then
            str = str .. '  '
         end
      end
      str = str .. ']'
   end
   return str
end

luaflow.ArrayT.mt.__add = luaflow.ArrayT.add_
luaflow.ArrayT.mt.__mul = luaflow.ArrayT.mul_
luaflow.ArrayT.mt.__div = luaflow.ArrayT.div_
luaflow.ArrayT.mt.__sub = luaflow.ArrayT.sub_
luaflow.ArrayT.mt.__tostring = luaflow.ArrayT.tostring
luaflow.ArrayT.mt.__index = luaflow.ArrayT.index
luaflow.ArrayT.mt.__newindex = luaflow.ArrayT.newindex


----------------------------------------------------------------------
-- luaflow.Node: the Node class, to create computing nodes
-- these nodes can be assembled into flow-graphs
----------------------------------------------------------------------
luaflow.Node = {}

function luaflow.Node.Convolution(args)
   -- parse args
   local input = args.input
   local output = args.output or luaflow.Array()
   local weight = args.weight
   local bias = args.bias
   local stride = args.stride or {1,1}
   local table = args.table
   local verbose = args.verbose or false

   -- create update function
   local node = {output=output, input=input, weight=weight, bias=bias, type='luaflow.Node'}
   node.update = function()
                    if #input.dims ~= 3 then
                       error('<luaflow.Node.Convolution> input must be a 3-dimensional Array')
                    end
                    if not table then
                       table = {}
                       for i = 1,input.dims[1] do
                          table[i] = {i}
                       end
                    end
                    output:resize(#table,
                                  input.dims[2]-weight.dims[2]+1, 
                                  input.dims[3]-weight.dims[3]+1)
                    if bias then
                       for i = 1,#table do
                          output[i]:fill(bias[i])
                       end
                    else
                       output:fill(0)
                    end
                    local w = 1
                    for o = 1,#table do
                       for idx_i = 1,#table[o] do
                          local i = table[o][idx_i]
                          output[o]:conv2D(input[i],weight[w],true)
                          w = w + 1
                       end
                    end
                    if verbose then
                       print('<luaflow.Node.Convolution>.input:')
                       print(input)
                       print('<luaflow.Node.Convolution>.output:')
                       print(output)
                    end
                 end

   -- return node
   return node
end

function luaflow.Node.Apply(args)
   -- parse args
   local input = args.input
   local output = args.output or luaflow.Array()
   local mapping = args.mapping
   local verbose = args.verbose or false

   -- create update function
   local node = {output=output, input=input, type='luaflow.Node'}
   node.update = function()
                    output:resize(input.dims)
                    output:copy(input)
                    output:apply(args.mapping)
                    if verbose then
                       print('<luaflow.Node.Apply>.input:')
                       print(input)
                       print('<luaflow.Node.Apply>.output:')
                       print(output)
                    end
                 end

   -- return node
   return node
end

function luaflow.Node.Camera(args)
   -- parse args
   local output = args.output or luaflow.Array()
   local verbose = args.verbose or false

   local node = {output=output, input=input, type='luaflow.Node'}
   node.update = function()
                    print('<luaflow.Node.Camera> warning: not implemented')
                    output:fill(0.01)
                    if verbose then
                       print('<luaflow.Node.Camera>.output:')
                       print(output)
                    end
                 end

   -- return node
   return node
end

function luaflow.Node.gather(...)
   -- get args
   local nodes = {...}
   if not (nodes[1].type and nodes[1].type == 'luaflow.Node') then
      nodes = nodes[1]
   end
   -- connect nodes
   local node = {}
   for i = 1,#nodes do
      node[i] = nodes[i]
   end
   node.input = nodes[1].input
   node.output = nodes[#nodes].output
   -- add update function
   node.update = function()
                    for i = 1,#node do
                       node[i]:update()
                    end
                 end
   -- return new node
   return node
end


----------------------------------------------------------------------
-- create aliases
----------------------------------------------------------------------
luaflow.Array = luaflow.ArrayT.new
return luaflow