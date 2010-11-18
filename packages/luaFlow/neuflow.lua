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
--     neuFlow - a handler for our custom dataflow computer
--
-- history: 
--     September 12, 2010, 6:45PM
----------------------------------------------------------------------

neuflow = {}

----------------------------------------------------------------------
-- neuflow.ArrayT: an extension of luaflow.ArrayT for neuFlow
-- it provides the same functions, same API, but everything done
-- here runs on neuFlow rather than on the local machine
----------------------------------------------------------------------
neuflow.ArrayT = {}
neuflow.ArrayT.mt = {}

function neuflow.ArrayT.new(...)
   -- init geometry
   local dims = {...}
   if type(dims[1]) == 'table' then
      dims = dims[1]
   end
   local array = {}
   array.type = 'neuflow.ArrayT'
   array.data = {}
   array.data_start = 1
   array.dims = dims
   array.size = 1
   for i = 1,#dims do
      array.size = array.size * dims[i]
   end
   array.data_end = array.data_start + array.size - 1
   -- set metatable for easy operators
   setmetatable(array, neuflow.ArrayT.mt)
   -- and set other operators
   for i,v in pairs(neuflow.ArrayT) do
      array[i] = v
   end
   return array
end


----------------------------------------------------------------------
-- neuflow.Node: the Node class, to create computing nodes
-- these nodes can be assembled into flow-graphs
----------------------------------------------------------------------
neuflow.Node = {}

function neuflow.Node.Convolution(args)
   -- parse args
   local input = args.input
   local output = args.output or luaflow.Array()
   local weight = args.weight
   local bias = args.bias
   local stride = args.stride or {1,1}
   local table = args.table
   local verbose = args.verbose or false

   -- create update function
   local node = {output=output, input=input, weight=weight, bias=bias, type='neuflow.Node'}
   node.update = function()
                    print('<neuflow.Node.Convolution> warning: not implemented')
                 end

   -- return node
   return node
end

function neuflow.Node.Apply(args)
   -- parse args
   local input = args.input
   local output = args.output or luaflow.Array()
   local mapping = args.mapping
   local verbose = args.verbose or false

   -- create update function
   local node = {output=output, input=input, type='luaflow.Node'}
   node.update = function()
                    print('<neuflow.Node.Node> warning: not implemented')
                 end

   -- return node
   return node
end


----------------------------------------------------------------------
-- global initialize function
-- this function takes care of the device/communication
----------------------------------------------------------------------
function neuflow.initialize(args)
   print('<neuFlow> context initialized')
end


----------------------------------------------------------------------
-- global execute function
-- this function takes executes code on the device
----------------------------------------------------------------------
function neuflow.execute(args)
   print('<neuFlow> loading and executing code')
end


----------------------------------------------------------------------
-- transfer functions: to/from neuflow
-- this function automatically infers the transfer direction,
-- and allocates the destination array
----------------------------------------------------------------------
function neuflow.transfer(array)
   -- array type ?
   if array.type == 'luaflow.ArrayT' then
      -- transfer luaFlow -> neuFlow
      local array_nf = neuflow.Array(array.dims)
      return array_nf
   elseif array.type == 'neuflow.ArrayT' then
      -- transfer neuFlow -> luaFlow
      local array_lf = luaflow.Array(array.dims)
      return array_lf
   end
end


----------------------------------------------------------------------
-- return neuflow
----------------------------------------------------------------------
neuflow.Array = neuflow.ArrayT.new
return neuflow