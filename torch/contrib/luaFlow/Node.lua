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
--     luaFlow.Node - the basic compute element of a flow-graph
--
-- history: 
--     September 11, 2010, 3:50PM - creation
----------------------------------------------------------------------


----------------------------------------------------------------------
-- luaFlow.Node: the Node class, to create computing nodes
-- these nodes can be assembled into flow-graphs
----------------------------------------------------------------------
luaFlow.Node = {}
luaFlow.Node.type = 'luaFlow.Node'


----------------------------------------------------------------------
-- Neural Network Nodes
----------------------------------------------------------------------
function luaFlow.Node.LinearFilterBank2D(args)
   -- internal
   local name = '<luaFlow.Node.LinearFilterBank2D>'

   -- parse args
   local input = args.input
   local output = args.output or luaFlow.Array()
   local weight = args.weight
   local bias = args.bias
   local stride = args.stride or {1,1}
   local table = args.table
   local verbose = args.verbose or false

   -- validate args
   if not input or not weight or not table then
      error(
      toolBox.usage(name, 
                    'creates a linear filter bank node', nil,
                    {arg='input', type='luaFlow.ArrayT', help='input data [3-dim]', req=true},
                    {arg='weight', type='luaFlow.ArrayT OR table', 
                     help='weight vector [3-dim]', req=true},
                    {arg='table', type='table', help='connection table [2-dim]', req=true},
                    {arg='output', type='luaFlow.ArrayT', help='output data [3-dim]'},
                    {arg='bias', type='luaFlow.ArrayT OR table', help='bias vector [1-dim]'},
                    {arg='stride', type='table', help='strides [1-dim]'},
                    {arg='verbose', type='boolean', help='activate verbose computations [debug]'}))
   end

   -- node structure
   local node = {output=output, input=input, weight=weight, bias=bias, 
                 type=luaFlow.Node.type, name=name,
                 __tostring = function (n,tab) return tab .. '+' .. name end}

   -- initiliaze params
   local flattable = luaFlow.Array(2,#table)
   for p,pair in ipairs(table) do
      flattable[1][p] = pair[1]
      flattable[2][p] = pair[2]
   end
   local conv = nn.SpatialConvolutionTable(flattable.data, weight.dims[3], weight.dims[2])
   conv.weight:copy(weight.data)
   if bias then
      conv.bias:copy(bias.data)
   else
      conv.bias:fill(0)
   end

   -- node's content:
   node.update = function()
                    if #input.dims ~= 3 then
                       error(name .. ' input must be a 3-dimensional Array')
                    end
                    conv:forward(input.data)
                    output:setdata(conv.output)
                    if verbose then
                       print(name .. '.input:')
                       print(input)
                       print(name .. '.output:')
                       print(output)
                    end
                 end

   -- derivate:
   node.dupdate = function(self)
                     conv:backward(self.input.data, self.doutput.data)
                     self.dinput:setdata(conv.gradInput)
                  end

   -- registration work
   luaFlow.pool.register(node)
   luaFlow.pool.attach(input,node)
   luaFlow.pool.attach(node,output)

   -- return node
   return node
end


----------------------------------------------------------------------
-- Math Nodes
----------------------------------------------------------------------
function luaFlow.Node.Apply(args)
   -- internal
   local name = '<luaFlow.Node.Apply>'

   -- parse args
   local input = args.input
   local output = args.output or luaFlow.Array()
   local mapping = args.mapping
   local verbose = args.verbose or false

   -- validate args
   if not input or not mapping then
      error(
      toolBox.usage(name, 'creates a mapping node', nil,
                    {arg='input', type='luaFlow.ArrayT', help='input data [N-dim]', req=true},
                    {arg='mapping', type='function', 
                     help='a mapping function, e.g. function (x) return f(x) end', req=true},
                    {arg='output', type='luaFlow.ArrayT', help='output data [N-dim]'},
                    {arg='verbose', type='boolean', help='activate verbose computations [debug]'}))
   end

   -- node structure
   local node = {output=output, input=input, type=luaFlow.Node.type, name=name,
                 __tostring = function (n,tab) return tab .. '+' .. name end}

   -- node's content
   node.update = function()
                    output:resize(input.dims)
                    output:copy(input)
                    output:apply(args.mapping)
                    if verbose then
                       print(name .. '.input:')
                       print(input)
                       print(name .. '.output:')
                       print(output)
                    end
                 end

   -- derivate:
   node.dupdate = function(self)
                     print(name .. ' WARNING: Derivative undefined, just copying gradients')
                     self.dinput:copy(self.doutput)
                  end

   -- registration work
   luaFlow.pool.register(node)
   luaFlow.pool.attach(input,node)
   luaFlow.pool.attach(node,output)

   -- return node
   return node
end


----------------------------------------------------------------------
-- Useful Nodes
----------------------------------------------------------------------
function luaFlow.Node.ImageSource(args)
   -- internal
   local name = '<luaFlow.Node.ImageSource>'

   -- parse args
   local output = args.output or luaFlow.Array(320,240,3)
   local input = luaFlow.Array()
   local type = args.type or 'lena'
   local verbose = args.verbose or false
   local grab = nn.ImageSource(type)

   -- structure
   local node = {output=output, input=input, name=name, type=luaFlow.Node.type, source=type,
                 __tostring = function (n,tab) return tab .. '+' .. name end}

   -- content
   node.update = function()
                    if #output.dims ~= 3 then
                       error(name .. ' input must be a 3-dimensional Array')
                    end
                    grab:forward()
                    output:alloc()
                    image.scale(grab.output, output.data, 'bilinear')
                    if verbose then
                       print(name .. '.output:')
                       print(output)
                    end
                 end

   -- derivate:
   node.dupdate = function(self)
                     -- nothing to do
                  end

   -- registration work
   luaFlow.pool.register(node)
   luaFlow.pool.attach(input,node)
   luaFlow.pool.attach(node,output)

   -- return node
   return node
end

function luaFlow.Node.ImageTransform(args)
   -- internal
   local name = '<luaFlow.Node.ImageTransform>'

   -- parse args
   local input = args.input
   local output = args.output or luaFlow.Array()
   local type = args.type
   local verbose = args.verbose or false
   local trans = nn.ImageTransform(type)

   -- validate args
   if not type or not input then
      error(toolBox.usage('nn.ImageTransform', nil, nil,
                          {arg='input', type='luaFlow.ArrayT', help='input data [3-dim]', req=true},
                          {arg='output', type='luaFlow.ArrayT', help='output data [3-dim]'},
                          {arg='type', type='string', help='transform = yuv2rgb | rgb2yuv | rgb2y', 
                           req=true}))
   end

   -- structure
   local node = {output=output, input=input, name=name, type=luaFlow.Node.type, transfo=type,
                 __tostring = function (n,tab) return tab .. '+' .. name end}

   -- content
   node.update = function()
                    trans:forward(input.data)
                    output:setdata(trans.output)
                    if verbose then
                       print(name .. '.output:')
                       print(output)
                    end
                 end

   -- derivate:
   node.dupdate = function(self)
                     trans:backward(input.data, self.doutput.data)
                     self.dinput:setdata(trans.gradInput)
                  end

   -- registration work
   luaFlow.pool.register(node)
   luaFlow.pool.attach(input,node)
   luaFlow.pool.attach(node,output)

   -- return node
   return node
end

function luaFlow.Node.Display(args)
   -- internal
   local name = '<luaFlow.Node.Display>'

   -- parse args
   local zoom = args.zoom or 1
   local min = args.min
   local max = args.max
   local win = args.win
   local input = args.input
   local output = luaFlow.Array()
   local verbose = args.verbose or false
   local disp = Displayer()

   -- structure
   local node = {output=output, input=input, name=name, type=luaFlow.Node.type, source=type,
                 __tostring = function (n,tab) return tab .. '+' .. name end}

   -- content
   node.update = function()
                    if #input.dims ~= 2 and #input.dims ~= 3 then
                       error(name .. ' input must be a 2|3-dimensional Array')
                    end
                    win = image.displayList{images=input.data, zoom=zoom, 
                                            min=min, max=max, window=win}
                 end

   -- derivate:
   node.dupdate = function(self)
                     -- fixed zero gradient
                     self.dinput:resize(input.dims).data:zero()
                  end

   -- registration work
   luaFlow.pool.register(node)
   luaFlow.pool.attach(input,node)
   luaFlow.pool.attach(node,output)

   -- return node
   return node
end


----------------------------------------------------------------------
-- Meta Nodes: to assemble basic nodes, and create complex graphs
----------------------------------------------------------------------
function luaFlow.Node.gather(...)
   -- internal
   local name = '<luaFlow.Node.gather>'

   -- get args
   local nodes = {...}
   if not (nodes[1].type and nodes[1].type == 'luaFlow.Node') then
      nodes = nodes[1]
   end

   -- create node
   local node = {name=name, type=luaFlow.Node.type}

   -- gather nodes:
   for i = 1,#nodes do
      node[i] = nodes[i]
   end

   -- identify and gather outputs/inputs
   node.input = {}
   node.output = {}
   for i,nod in ipairs(node) do
      if nod.output and not nod.output.CID then -- this is a leaf == an output
         table.insert(node.output, nod.output)
      end
      if nod.input and not nod.input.PID then -- this is a root == an input
         table.insert(node.input, nod.input)
      end
   end

   -- figure out the list of updates, starting from the outputs
   -- we mark each data node with an X=NNN, where NNN is an integer giving the
   -- priority of the update
   local count = 1
   local pool = luaFlow.pool.nodes
   function parseDown (node)
      if node.type == 'luaFlow.ArrayT' then
         node.X = count; count = count + 1
         if node.CID then
            for i,id in ipairs(node.CID) do
               parseDown(pool[id])
            end
         end
      else
         for i,id in ipairs(node.PID) do
            if not pool[id].X then
               return
            end
            node.X = count; count = count + 1
         end
         if node.CID then
            for i,id in ipairs(node.CID) do
               if not pool[id].X then
                  parseDown(pool[id])
               end
            end
         end
      end
   end

   -- start from the inputs:
   for i,input in ipairs(node.input) do
      if not input.X then
         parseDown(input)
      end
   end

   -- sort nodes, based on their priority:
   table.sort(node, function (node1, node2) if node1.X < node2.X then return node2 end end)

   -- add update function
   node.update = function()
                    for i,n in ipairs(node) do
                       n:update()
                    end
                 end

   -- derivate:
   node.dupdate = function(self)
                     for i = #node,1,-1 do
                        node[i]:dupdate()
                     end
                  end

   -- add print function
   node.__tostring = function(n,tab)
                        tab = tab or ''
                        local str = name .. '\n'
                        for i,n in ipairs(node) do
                           str = str .. tab .. n:__tostring(tab..'  ') .. '\n'
                        end
                        return str
                     end
   setmetatable(node, {__tostring=node.__tostring})

   -- registration work
   luaFlow.pool.register(node)
   luaFlow.pool.register(node.input)
   luaFlow.pool.register(node.output)
   luaFlow.pool.attach(node.input,node)
   luaFlow.pool.attach(node,node.output)

   -- return new node
   return node
end


----------------------------------------------------------------------
-- Derivative nodes, to be used by learning algorithms
----------------------------------------------------------------------
function luaFlow.Node.getDerivative(args)
   -- internal
   local name = '<luaFlow.Node.Derivative[' .. args.node.name .. ']>'

   -- parse args
   local zoom = args.zoom or 1
   local input = args.input
   local output = args.output or luaFlow.Array()
   local dnode = args.node
   local verbose = args.verbose or false

   -- structure
   local node = {output=output, input=input, 
                 name=name, type=luaFlow.Node.type, derivative = 1,
                 __tostring = function (n,tab) return tab .. '+' .. name end}

   -- set I/Os for underlying node
   dnode.doutput = input
   dnode.dinput = output

   -- content
   node.update = function()
                    dnode:dupdate()
                 end

   -- registration work
   luaFlow.pool.register(node)
   luaFlow.pool.attach(input,node)
   luaFlow.pool.attach(node,output)

   -- return node
   return node
end


----------------------------------------------------------------------
-- Serialization: using xFlow !
----------------------------------------------------------------------
function luaFlow.Node.load(args)
   -- internal
   local name = '<luaFlow.Node.Loaded>'

   -- parse args
   local input = args.input
   local output = args.output or luaFlow.Array()
   local file = args.file
   local verbose = args.verbose or false

   -- validate args
   if not file or not input then
      error(toolBox.usage(name, nil, nil,
                          {arg='input', type='luaFlow.ArrayT', help='input data [3-dim]', req=true},
                          {arg='output', type='luaFlow.ArrayT', help='output data [3-dim]'},
                          {arg='file', type='string', help='path to an xFlow file',
                           req=true}))
   end

   -- load xFlow
   local xf = require 'xFlow'
   local xnode = xf.engine.implement(file)

   -- structure
   local node = {output=output, input=input, name=name, type=luaFlow.Node.type,
                 __tostring = function (n,tab) return tab .. '+ ' 
                              .. name .. '\n' .. xnode:__tostring(tab..'  ') end}

   -- content
   node.update = function()
                    xnode.input[1]:resizeAs(input.data):copy(input.data)
                    xnode:update()
                    output:setdata(xnode.output[1])
                 end

   -- registration work
   luaFlow.pool.register(node)
   luaFlow.pool.attach(input,node)
   luaFlow.pool.attach(node,output)

   -- return node
   return node
end

function luaFlow.Node.save(node, xfl_file)
   local xf = require 'xFlow'
   local node = xf.engine.serialize(node, xfl_file)
end
