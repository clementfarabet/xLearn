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
-- Std Lib Nodes
----------------------------------------------------------------------
function luaFlow.Node.Flow(args)
   -- internal
   local name = '<luaFlow.Node.Flow>'

   -- parse args
   local inputs = {}
   inputs.x = args.x
   local outputs = {}
   outputs.copy_x_ = args.copy_x_ or luaFlow.Array()
   outputs.delay_x_ = args.delay_x_ or luaFlow.Array()
   local verbose = args.verbose or false

   -- validate args
   if not inputs.x then
      error(
      toolBox.usage(name, 'creates a flow control node', nil,
                    -- inputs
                    {arg='x', type='luaFlow.ArrayT', help='input data [N-dim]', req=true},
                    -- outputs
                    {arg='copy_x_', type='luaFlow.ArrayT', help='simple copy of x [N-dim]'},
                    {arg='delay_x_', type='luaFlow.ArrayT', help='delayed version of x [N-dim]'},
                    -- verbose
                    {arg='verbose', type='boolean', help='activate verbose computations [debug]'}))
   end

   -- node structure
   local node = {outputs=outputs, inputs=inputs,
                 copy_x_=outputs.copy_x_, delay_x_=outputs.delay_x_,
                 type=luaFlow.Node.type, name=name,
                 __tostring = function (n,tab) return tab .. '+' .. name end}

   -- node's content
   node.update = function()
                    for k,output in pairs(outputs) do
                       if output.CID and #output.CID ~= 0 then
                          output:resize(inputs.x.dims)
                          if k == 'copy_x_' then
                             output:copy(inputs.x)
                          elseif k == 'delay_x_' then
                             print(name .. 'WARNING: delay<x> not implemented yet')
                          end
                       end
                    end
                    if verbose then
                       print(name .. '.input:')
                       p(inputs)
                       print(name .. '.output:')
                       p(outputs)
                    end
                 end

   -- derivate:
   node.dupdate = function(self)
                     print(name .. ' WARNING: Derivative undefined')
                  end

   -- registration work
   luaFlow.pool.register(node)
   luaFlow.pool.attach(inputs.x,node)
   for k,output in pairs(outputs) do
      luaFlow.pool.attach(node,output)
   end

   -- return node
   return node
end


----------------------------------------------------------------------
-- Math Nodes
----------------------------------------------------------------------
function luaFlow.Node.Math(args)
   -- internal
   local name = '<luaFlow.Node.Math>'

   -- parse args
   local inputs = {}
   inputs.x = args.x
   inputs.y = args.y or luaFlow.Array()
   inputs.k = args.k
   local outputs = {}
   outputs.minus_x_ = args.minus_x_ or luaFlow.Array()
   outputs.inv_x_ = args.inv_x_ or luaFlow.Array()
   outputs.abs_x_ = args.abs_x_ or luaFlow.Array()
   outputs.add_xy_ = args.add_xy_ or luaFlow.Array()
   outputs.sub_xy_ = args.mul_xy_ or luaFlow.Array()
   outputs.mul_xy_ = args.mul_xy_ or luaFlow.Array()
   outputs.div_xy_ = args.add_xy_ or luaFlow.Array()
   local verbose = args.verbose or false

   -- validate args
   if not inputs.x then
      error(
      toolBox.usage(name, 'creates a math node', nil,
                    -- inputs
                    {arg='x', type='luaFlow.ArrayT', help='input data [N-dim]', req=true},
                    {arg='y', type='luaFlow.ArrayT', help='input data [N-dim]'},
                    {arg='k', type='number', help='a constant used by certain outputs'},
                    -- outputs
                    {arg='add_xy_', type='luaFlow.ArrayT', help='point-wise addition x+y [N-dim]'},
                    {arg='sub_xy_', type='luaFlow.ArrayT', help='point-wise subtraction x-y [N-dim]'},
                    {arg='mul_xy_', type='luaFlow.ArrayT', help='point-wise multiplication x*y [N-dim]'},
                    {arg='div_xy_', type='luaFlow.ArrayT', help='point-wise division x/y [N-dim]'},
                    -- verbose
                    {arg='verbose', type='boolean', help='activate verbose computations [debug]'}))
   end

   -- node structure
   local node = {outputs=outputs, inputs=inputs, 
                 x=inputs.x, y=inputs.y, k=inputs.k,
                 minus_x_=outputs.minus_x_, inv_x_=outputs.inv_x_,
                 add_xy_=outputs.add_xy_, mul_xy_=outputs.mul_xy_,
                 sub_xy_=outputs.sub_xy_, div_xy_=outputs.div_xy_,
                 abs_x_=outputs.abs_x_,
                 type=luaFlow.Node.type, name=name,
                 __tostring = function (n,tab) return tab .. '+' .. name end}

   -- hashtable for quick lookup:
   local compute = { inv_x_ = function(output) 
                                 output:fill(1)
                                 output:div(inputs.x)
                              end,
                     minux_x_ = function(output)
                                   output:copy(inputs.x)
                                   output:mul(-1)
                                end,
                     add_xy_ = function(output)
                                 output:copy(inputs.x)
                                 output:add(inputs.y)
                              end,
                     mul_xy_ = function(output)
                                 output:copy(inputs.x)
                                 output:mul(inputs.y)
                              end,
                     div_xy_ = function(output)
                                 output:copy(inputs.x)
                                 output:div(inputs.y)
                              end,
                     sub_xy_ = function(output)
                                 output:copy(inputs.x)
                                 output:sub(inputs.y)
                              end,
                     abs_x_ = function(output) 
                                 output:copy(inputs.x)
                                 output:abs()
                              end
                  }

   -- node's content
   node.update = function()
                    for k,output in pairs(outputs) do
                       if output.CID and #output.CID ~= 0 then
                          output:resize(inputs.x.dims)
                          compute[k](output)
                       end
                    end
                    if verbose then
                       print(name .. '.input:')
                       p(inputs)
                       print(name .. '.output:')
                       p(outputs)
                    end
                 end

   -- derivate:
   node.dupdate = function(self)
                     print(name .. ' WARNING: Derivative undefined')
                  end

   -- registration work
   luaFlow.pool.register(node)
   for k,input in pairs(inputs) do
      luaFlow.pool.attach(input,node)
   end
   for k,output in pairs(outputs) do
      luaFlow.pool.attach(node,output)
   end

   -- return node
   return node
end


function luaFlow.Node.Apply(args)
   -- internal
   local name = '<luaFlow.Node.Apply>'

   -- parse args
   local inputs = {}
   inputs[1] = args.input
   local outputs = {}
   outputs[1] = args.output or luaFlow.Array()
   local mapping = args.mapping
   local verbose = args.verbose or false

   -- validate args
   if not inputs[1] or not mapping then
      error(
      toolBox.usage(name, 'creates a mapping node', nil,
                    {arg='input', type='luaFlow.ArrayT', help='input data [N-dim]', req=true},
                    {arg='mapping', type='function', 
                     help='a mapping function, e.g. function (x) return f(x) end', req=true},
                    {arg='output', type='luaFlow.ArrayT', help='output data [N-dim]'},
                    {arg='verbose', type='boolean', help='activate verbose computations [debug]'}))
   end

   -- node structure
   local node = {outputs=outputs, inputs=inputs, 
                 output=outputs[1], input=inputs[1],
                 type=luaFlow.Node.type, name=name,
                 __tostring = function (n,tab) return tab .. '+' .. name end}

   -- node's content
   node.update = function()
                    outputs[1]:resize(inputs[1].dims)
                    outputs[1]:copy(inputs[1])
                    outputs[1]:apply(args.mapping)
                    if verbose then
                       print(name .. '.input:')
                       print(inputs[1])
                       print(name .. '.output:')
                       print(outputs[1])
                    end
                 end

   -- derivate:
   node.dupdate = function(self)
                     print(name .. ' WARNING: Derivative undefined, just copying gradients')
                     self.dinput[1]:copy(self.doutput[1])
                  end

   -- registration work
   luaFlow.pool.register(node)
   luaFlow.pool.attach(inputs[1],node)
   luaFlow.pool.attach(node,outputs[1])

   -- return node
   return node
end


----------------------------------------------------------------------
-- Neural Network Nodes
----------------------------------------------------------------------
function luaFlow.Node.LinearFilterBank(args)
   -- internal
   local name = '<luaFlow.Node.LinearFilterBank>'

   -- parse args
   local inputs = {}
   inputs[1] = args.input
   local outputs = {}
   outputs[1] = args.output or luaFlow.Array()
   local weight = args.weight
   local bias = args.bias
   local stride = args.stride or {1,1}
   local table = args.table
   local verbose = args.verbose or false

   -- validate args
   if not inputs[1] or not weight or not table then
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
   local node = {outputs=outputs, inputs=inputs, 
                 weight=weight, bias=bias, 
                 output=outputs[1], input=inputs[1], 
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
                    if #inputs[1].dims ~= 3 then
                       error(name .. ' input must be a 3-dimensional Array')
                    end
                    conv:forward(inputs[1].data)
                    outputs[1]:setdata(conv.output)
                    if verbose then
                       print(name .. '.input:')
                       print(inputs[1])
                       print(name .. '.output:')
                       print(outputs[1])
                    end
                 end

   -- derivate:
   node.dupdate = function(self)
                     conv:backward(self.inputs[1].data, self.doutputs[1].data)
                     self.dinputs[1]:setdata(conv.gradInput)
                  end

   -- registration work
   luaFlow.pool.register(node)
   luaFlow.pool.attach(inputs[1],node)
   luaFlow.pool.attach(node,outputs[1])

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
   local outputs = {}
   outputs[1] = args.output or luaFlow.Array(320,240,3)
   local inputs = {}
   inputs[1] = luaFlow.Array()
   local type = args.type or 'lena'
   local verbose = args.verbose or false
   local grab = nn.ImageSource(type)

   -- structure
   local node = {outputs=outputs, inputs=inputs, 
                 output=outputs[1], input=inputs[1], 
                 name=name, type=luaFlow.Node.type, source=type,
                 __tostring = function (n,tab) return tab .. '+' .. name end}

   -- content
   node.update = function()
                    if #outputs[1].dims ~= 3 then
                       error(name .. ' input must be a 3-dimensional Array')
                    end
                    grab:forward()
                    outputs[1]:alloc()
                    image.scale(grab.output, outputs[1].data, 'bilinear')
                    if verbose then
                       print(name .. '.output:')
                       print(outputs[1])
                    end
                 end

   -- derivate:
   node.dupdate = function(self)
                     -- nothing to do
                  end

   -- registration work
   luaFlow.pool.register(node)
   luaFlow.pool.attach(inputs[1],node)
   luaFlow.pool.attach(node,outputs[1])

   -- return node
   return node
end

function luaFlow.Node.ImageTransform(args)
   -- internal
   local name = '<luaFlow.Node.ImageTransform>'

   -- parse args
   local inputs = {}
   inputs[1] = args.input
   local outputs = {}
   outputs[1] = args.output or luaFlow.Array()
   local type = args.type
   local verbose = args.verbose or false
   local trans = nn.ImageTransform(type)

   -- validate args
   if not type or not inputs[1] then
      error(toolBox.usage('nn.ImageTransform', nil, nil,
                          {arg='input', type='luaFlow.ArrayT', help='input data [3-dim]', req=true},
                          {arg='output', type='luaFlow.ArrayT', help='output data [3-dim]'},
                          {arg='type', type='string', help='transform = yuv2rgb | rgb2yuv | rgb2y', 
                           req=true}))
   end

   -- structure
   local node = {outputs=outputs, inputs=inputs, 
                 transfo=type,
                 output=outputs[1], input=inputs[1], 
                 name=name, type=luaFlow.Node.type,
                 __tostring = function (n,tab) return tab .. '+' .. name end}

   -- content
   node.update = function()
                    trans:forward(inputs[1].data)
                    outputs[1]:setdata(trans.output)
                    if verbose then
                       print(name .. '.output:')
                       print(outputs[1])
                    end
                 end

   -- derivate:
   node.dupdate = function(self)
                     trans:backward(inputs[1].data, self.doutputs[1].data)
                     self.dinputs[1]:setdata(trans.gradInput)
                  end

   -- registration work
   luaFlow.pool.register(node)
   luaFlow.pool.attach(inputs[1],node)
   luaFlow.pool.attach(node,outputs[1])

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
   local inputs = {}
   inputs[1] = args.input
   local outputs = {}
   outputs[1] = luaFlow.Array()
   local verbose = args.verbose or false
   local legend = name .. ' :: ' .. (args.legend or 'nil')

   -- structure
   local node = {outputs=outputs, inputs=inputs, 
                 source=type,
                 output=outputs[1], input=inputs[1], 
                 name=name, type=luaFlow.Node.type,
                 __tostring = function (n,tab) return tab .. '+' .. name end}

   -- content
   node.update = function()
                    if #inputs[1].dims ~= 2 and #inputs[1].dims ~= 3 then
                       error(name .. ' input must be a 2|3-dimensional Array')
                    end
                    win = image.displayList{images=inputs[1].data, zoom=zoom, 
                                            min=min, max=max, window=win, legend=legend, 
                                            gui=false}
                    if verbose then
                       print(name .. ' displaying data [' .. legend .. ']')
                    end
                 end

   -- derivate:
   node.dupdate = function(self)
                     -- fixed zero gradient
                     self.dinputs[1]:resize(inputs[1].dims).data:zero()
                  end

   -- registration work
   luaFlow.pool.register(node)
   luaFlow.pool.attach(inputs[1],node)
   luaFlow.pool.attach(node,outputs[1])

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
   if not nodes[1].name then nodes = nodes[1] end
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
   node.inputs = {}
   node.outputs = {}
   for i,nod in ipairs(node) do
      for k,output in pairs(nod.outputs) do
         if not output.CID then -- this is a leaf == an output
            table.insert(node.outputs, output)
         end
      end
      for k,input in pairs(nod.inputs) do
         if not input.PID then -- this is a root == an input
            table.insert(node.inputs, input)
         end
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
   for i,input in ipairs(node.inputs) do
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
   for i,input in pairs(node.inputs) do
      luaFlow.pool.register(input)
      luaFlow.pool.attach(input,node)
   end
   for i,output in pairs(node.outputs) do
      luaFlow.pool.register(output)
      luaFlow.pool.attach(node,output)
   end

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
   local inputs = args.inputs or {}
   local outputs = args.outputs or {}
   local dnode = args.node
   local verbose = args.verbose or false

   -- structure
   local node = {outputs=outputs, inputs=inputs, 
                 derivative = 1,
                 name=name, type=luaFlow.Node.type,
                 __tostring = function (n,tab) return tab .. '+' .. name end}

   -- replicate I/Os from underlying node
   for key in pairs(dnode.outputs) do
      inputs[key] = inputs[key] or luaFlow.Array()
   end
   for key in pairs(dnode.inputs) do
      outputs[key] = outputs[key] or luaFlow.Array()
   end

   -- set I/Os for underlying node
   dnode.doutputs = inputs
   dnode.dinputs = outputs

   -- content
   node.update = function()
                    dnode:dupdate()
                 end

   -- registration work
   luaFlow.pool.register(node)
   for i,input in pairs(inputs) do
      luaFlow.pool.attach(input,node)
   end
   for i,output in pairs(outputs) do
      luaFlow.pool.attach(node,output)
   end

   -- return node
   return node
end


----------------------------------------------------------------------
-- define new Node from string description
----------------------------------------------------------------------
function luaFlow.Node.define(str)
   -- internal
   local name = '<luaFlow.Node.New>'
   local code = loadstring(str)

   -- create function to create node
   local creator = function(args)
                      -- define protected environment
                      local env = {luaFlow=luaFlow,table=table}

                      -- get user I/Os
                      if args.inputs then
                         for k,arginput in pairs(args.inputs) do
                            env[k] = arginput
                         end
                      end
                      if args.outputs then
                         for k,argoutput in pairs(args.outputs) do
                            env[k] = argoutput
                         end
                      end

                      -- exec/implement code in protected env
                      setfenv(code,env)
                      code()

                      -- gather nodes
                      local inodes = {}
                      local knodes = {}
                      for k,v in pairs(env) do 
                         if type(v) == 'table' and v.type and v.type == luaFlow.Node.type then
                            table.insert(inodes,v) 
                            knodes[k] = v
                         end
                      end
                      local node = luaFlow.Node.gather(inodes)

                      -- make I/Os and submodes available
                      node.inputs = {}
                      node.outputs = {}
                      for _,var in pairs(env.input_vars) do
                         node.inputs[var] = env[var]
                      end
                      for _,var in pairs(env.output_vars) do
                         node.outputs[var] = env[var]
                      end
                      node.nodes = knodes

                      -- return node
                      return node
                   end

   -- return creator function
   return creator
end
luaFlow.Node.def = luaFlow.Node.define


----------------------------------------------------------------------
-- Serialization: using xFlow !
-- we handle version 0.2 and 0.3 here, not automatically though:
-- the arg.version has to be provided
----------------------------------------------------------------------
function luaFlow.Node.load(args)
   local version = tostring(args.version or luaFlow.ver) -- default version
   return luaFlow.Node.loadver[version](args)
end

luaFlow.Node.loadver = {
   ['0.3'] = function(args)
                -- internal
                local name = '<luaFlow.Node.xFlow>'
                
                -- parse args
                local inputs = args.inputs or {}
                local outputs = args.outputs or {}
                local file = args.file
                local verbose = args.verbose or false

                -- validate args
                if not file then
                   error(
                      toolBox.usage(
                         name, nil, nil,
                         {arg='inputs', type='table', help='table of inputs'},
                         {arg='outputs', type='table', help='table of outputs'},
                         {arg='file', type='string', help='path to an xFlow file',
                          req=true}
                   ))
                end

                -- check file existence
                if not paths.filep(file) then
                   error(name .. ' file ' .. file .. ' does not exist')
                end

                -- load xFlow
                local xflow2lua_m4 = paths.concat(paths.install_share,'xFlow','m4','lua.m4')
                local translated = toolBox.exec('xflow-translate ' .. file .. ' ' .. xflow2lua_m4)
                if string.find(translated, 'not found') or not paths.filep(xflow2lua_m4) then
                   error(name .. ' xflow-translate not found, please install xFlow tools')
                end

                -- verbose print
                if verbose then
                   print(toolBox.COLORS.blue)
                   print('\n\n+++ implementing node +++ ')
                   print(translated)                   
                   print('--- implementing node --- \n\n')
                   print(toolBox.COLORS.none)
                end

                -- define new node
                local NewNode = luaFlow.Node.define(translated)
                
                -- instantiate new node
                local node = NewNode{inputs = inputs, outputs = outputs, verbose = verbose}
                node.name = name
                
                -- return node, and its def
                return node, NewNode
             end,
   
   ['0.2'] = function(args)
                -- internal
                local name = '<luaFlow.Node.xFlow>'
                
                -- parse args
                local inputs = {}
                inputs[1] = args.input
                local outputs = {}
                outputs[1] = args.output or luaFlow.Array()
                local file = args.file
                local verbose = args.verbose or false
                
                -- validate args
                if not file or not inputs[1] then
                   error(
                      toolBox.usage(
                         name, nil, nil,
                         {arg='input', type='luaFlow.ArrayT', help='input data [3-dim]', req=true},
                         {arg='output', type='luaFlow.ArrayT', help='output data [3-dim]'},
                         {arg='file', type='string', help='path to an xFlow file',
                          req=true}
                   ))
                end
                
                -- load xFlow
                local xf = require 'xFlow'
                local xnode = xf.engine.implement(file)
                
                -- structure
                local node = {outputs=outputs, inputs=inputs,
                              input=inputs[1], output=outputs[1],
                              name=name, type=luaFlow.Node.type,
                              __tostring = function (n,tab) return tab .. '+ ' 
                              .. name .. '\n' .. xnode:__tostring(tab..'  ') end}
                
                -- content
                node.update = function()
                                 xnode.input[1]:resizeAs(inputs[1].data):copy(inputs[1].data)
                                 xnode:update()
                    outputs[1]:setdata(xnode.output[1])
                 end
                
                -- registration work
                luaFlow.pool.register(node)
                luaFlow.pool.attach(inputs[1],node)
                luaFlow.pool.attach(node,outputs[1])
                
                -- return node
                return node
             end
}

function luaFlow.Node.save(node, xfl_file)
   local xf = require 'xFlow'
   local node = xf.engine.serialize(node, xfl_file)
end
