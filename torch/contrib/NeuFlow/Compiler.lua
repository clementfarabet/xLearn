
----------------------------------------------------------------------
--- Class: Compiler
--
-- This class provides a set of methods to compile a neural network into 
-- bytecode for the dataflow computer.
--
local Compiler = torch.class('Compiler')

local message = {
   WARNING_IMPLEMENTED = '# WARNING: module not implemented > '
}

function Compiler:__init(args)
   -- args:
   self.opt_across_layers = args.optimize_across_layers or false
   self.logfile = args.logfile or nil
   self.core = args.core
   self.msg_level = args.msg_level or 'concise' -- can be 'none' or 'detailled'
   
   if (self.core == nil or self.logfile == nil) then
      error('# ERROR <Compiler> : please provide DataflowComputer + Log')
   end

   -- this param holds the number of ops required to compute the given net
   self.ops = 0
end

-- This is a list of layers in the NN package
local layers_table = {
   -- Local operators 
   ["nn.SpatialSubSampling"] = 
      function(net_compiler, module, inputs, mapping) 
         return net_compiler:SpatialSubSampling(module, inputs, mapping) 
      end,

   ["nn.SpatialMaxPooling"] = 
      function(net_compiler, module, inputs, mapping)
      end,

   ["nn.SpatialConvolution"] =
      function(net_compiler, module, inputs, mapping) 
         return net_compiler:SpatialConvolution(module, inputs, mapping) 
      end,

   ["nn.SpatialLinear"] = 
      function(net_compiler, module, inputs, mapping) 
         return net_compiler:SpatialLinear(module, inputs, mapping) 
      end,

   ["nn.SpatialConvolutionTable"] = 
      function(net_compiler, module, inputs, mapping) 
         return net_compiler:SpatialConvolutionTable(module, inputs, mapping)
      end,

   ["nn.LocalNorm"] = 
      function(net_compiler, module, inputs) 
         return net_compiler:LocalNorm(module, inputs)
      end,

   ["nn.SpatialPadding"] =
      function(net_compiler, module, inputs) 
         print(message.WARNING_IMPLEMENTED, module)
         return inputs
      end,

   -- Global operators (add hardware)
   ["nn.Max"] = 
      function(net_compiler, module, inputs) 
         print(message.WARNING_IMPLEMENTED, module)
         return inputs
      end,

   ["nn.Min"] = 
      function(net_compiler, module, inputs) 
         print(message.WARNING_IMPLEMENTED, module)
         return inputs
      end,

   ["nn.Sum"] = 
      function(net_compiler, module, inputs) 
         print(message.WARNING_IMPLEMENTED, module)
         return inputs
      end,

   ["nn.Mean"] = 
      function(net_compiler, module, inputs) 
         print(message.WARNING_IMPLEMENTED, module)
         return inputs
      end,

   ["nn.Euclidean"] =
      function(net_compiler, module, inputs) 
         print(message.WARNING_IMPLEMENTED, module)
         return inputs
      end,

   -- Non Linear mappings (add software)
   ["nn.AbsModule"] = 
      function(net_compiler, module, inputs) 
         return net_compiler:Mapping(module,inputs,'Abs')
      end,

   ["nn.Sqrt"] = 
      function(net_compiler, module, inputs) 
         print(message.WARNING_IMPLEMENTED, module)
         return inputs
      end,

   ["nn.SoftMax"] = 
      function(net_compiler, module, inputs) 
         print(message.WARNING_IMPLEMENTED, module)
         return inputs
      end,

   ["nn.Sigmoid"] = 
      function(net_compiler, module, inputs) 
         print(message.WARNING_IMPLEMENTED, module)
         return inputs
      end,

   ["nn.HardTanh"] =
      function(net_compiler, module, inputs) 
         print(message.WARNING_IMPLEMENTED, module)
         return inputs
      end,

   ["nn.StdSigm"] =
      function(net_compiler, module, inputs) 
         return net_compiler:Mapping(module,inputs,'StdSigm')
      end,

   ["nn.Tanh"] = 
      function(net_compiler, module, inputs) 
         return net_compiler:Mapping(module,inputs,'Tanh')
      end,

   ["nn.TanhAbs"] = 
      function(net_compiler, module, inputs) 
         return net_compiler:Mapping(module,inputs,'TanhAbs')
      end,

   -- component-wise operators 
   ["nn.CCSub"] =
      function(net_compiler, module, inputs) 
         return net_compiler:CCSub(module, inputs)
      end,

   ["nn.CCAdd"] =
      function(net_compiler, module, inputs)
         return net_compiler:CCAdd(module, inputs)
      end,

   -- Piece wise operations (add software)
   ["nn.Add"] =
      function(net_compiler, module, inputs) 
         print(message.WARNING_IMPLEMENTED, module)
         return inputs
      end,

   ["nn.Mult"] =
      function(net_compiler, module, inputs) 
         print(message.WARNING_IMPLEMENTED, module)
         return inputs
      end,

   ["nn.Linear"] =
      function(net_compiler, module, inputs) 
         print(message.WARNING_IMPLEMENTED, module)
         return inputs
      end,

   ["nn.Square"] = 
      function(net_compiler, module, inputs) 
         print(message.WARNING_IMPLEMENTED, module)
         return inputs
      end,

   -- Handled by high level software
   ["nn.Reshape"]  =
      function(net_compiler, module, inputs) 
         return net_compiler:Reshape(module, inputs) 
      end,

   ["nn.Threshold"] =
      function(net_compiler, module, inputs)
         return net_compiler:Threshold(module, inputs)
      end
}


--
-- This function receives a network and
-- goes through all its layers
-- and calls the appropriate functions of the 
-- ByteCode, DataflowComputer classes to dump the bytecode
--
function Compiler:processNetwork(network, inputs)
   if (self.print_times == 'detailled') then
      self.core:startProcess()
      self.core:message('unrolling convnet...')
      self.core:resetTime()
      self.core:getTime()
      self.core:endProcess()
   end
   print('# processing network or type >', network)
   local doneAdvance = 0
   local outputs
   for i=1,#network.modules do
      if doneAdvance > 0 then
         doneAdvance = doneAdvance - 1
      else
         local module_0, module_1, module_2, module_3, module_name
         module_0 = network.modules[i+0].__typename
         module_name = module_0
         if i+1 <= #network.modules then
            module_1 = network.modules[i+1].__typename
         end
         if i+2 <= #network.modules then
            module_2 = network.modules[i+2].__typename
         end
         if i+3 <= #network.modules then
            module_3 = network.modules[i+3].__typename
         end
         io.write(toolBox.COLORS.cyan)
         io.write('# processing layer of type > '..module_0)
         mapping = nil
         if self.opt_across_layers then
            if module_0 == 'nn.Tanh' and module_1 == 'nn.AbsModule' then
               module_name = 'nn.TanhAbs'
               io.write(' merged with next layer > '..module_1..' >>> '..module_name)
               doneAdvance = 1
            elseif module_0 == 'nn.Mult' and module_1 == 'nn.Tanh' and module_2 == 'nn.Mult' then
               module_name = 'nn.StdSigm'
               io.write(' merged with next layers > '..module_1..' & '..module_2..
                        ' >>> '..module_name)
               doneAdvance = 2
            elseif module_0 == 'nn.SpatialConvolution' 
               and module_1 == 'nn.Tanh' and module_2 == 'nn.AbsModule' then
               mapping = 'TanhAbs'
               io.write(' merged with next layers > '..module_1..' & '..module_2..
                        ' >>> '..module_name)
               doneAdvance = 2
            elseif module_0 == 'nn.SpatialConvolution' and module_1 == 'nn.Tanh' then
               mapping = 'Tanh'
               io.write(' merged with next layers > '..module_1..' >>> '..module_name)
               doneAdvance = 1
            elseif module_0 == 'nn.SpatialSubSampling' and module_1 == 'nn.Tanh' then
               mapping = 'Tanh'
               io.write(' merged with next layers > '..module_1..' >>> '..module_name)
               doneAdvance = 1
            elseif module_0 == 'nn.SpatialSubSampling' and module_1 == 'nn.Mult'
               and module_2 == 'nn.Tanh' and module_3 == 'nn.Mult' then
               mapping = 'StdSigm'
               io.write(' merged with next layers > '..module_1..' & '..module_2..' & '..module_3
                        ..' >>> '..module_name)
               doneAdvance = 3
            elseif module_0 == 'nn.SpatialConvolutionTable' 
               and module_1 == 'nn.Tanh' and module_2 == 'nn.AbsModule' then
               mapping = 'TanhAbs'
               io.write(' merged with next layers > '..module_1..' & '..module_2
                        ..' >>> '..module_name)
               doneAdvance = 2
            elseif module_0 == 'nn.SpatialConvolutionTable' and module_1 == 'nn.Tanh' then
               mapping = 'Tanh'
               io.write(' merged with next layers > '..module_1..' >>> '..module_name)
               doneAdvance = 1
            elseif module_0 == 'nn.SpatialConvolutionTable' and module_1 == 'nn.Mult' 
               and module_2 == 'nn.Tanh' and module_3 == 'nn.Mult' then
               mapping = 'StdSigm'
               io.write(' merged with next layers > '..module_1..' & '..module_2..' & '..module_3
                        ..' >>> '..module_name)
               doneAdvance = 3
            end
         end
         --print('')
         print(toolBox.COLORS.none)
         outputs = layers_table[module_name](self, network.modules[i], inputs, mapping)
         inputs = outputs
         if (self.msg_level == 'detailled') then
            self.core:startProcess()
            self.core:getTime()
            self.core:resetTime()
            self.core:endProcess()
         end
      end
   end
   self:printStats()
   return inputs
end

-- This is a list of nodes from the xfLow libs
local nodes_table = {
   -- math operators
   ["math>x+y"] =
      function(net_compiler, node, inputs, mapping) 
         print(message.WARNING_IMPLEMENTED .. node.name)
         return inputs
      end,

   ["math>x*y"] =
      function(net_compiler, node, inputs, mapping) 
         print(message.WARNING_IMPLEMENTED .. node.name)
         return inputs
      end,

   ["std-normalization-2d>output"] =
      function(net_compiler, node, inputs)
         return net_compiler:LocalNorm(node.params, inputs)
      end,

   ["linear-filter-bank-2d>output"] = 
      function(net_compiler, node, inputs, mapping) 
         return net_compiler:SpatialConvolutionTable(node.params, inputs, mapping)
      end,

   ["math>mapping-x-"] = 
      function(net_compiler, node, inputs) 
         return net_compiler:Mapping(nil,inputs,node.params.mapping)
      end
}

--
-- This function receives a flow (compiled xFlow program),
-- goes through all its nodes,
-- and calls the appropriate functions of the 
-- ByteCode, DataflowComputer classes to dump the bytecode
--
function Compiler:processFlow(flow, inputs)
   if (self.print_times == 'detailled') then
      self.core:startProcess()
      self.core:message('unrolling convnet...')
      self.core:resetTime()
      self.core:getTime()
      self.core:endProcess()
   end
   print('# processing xFlow')

   -- find all mappings
   for i=1,#flow.machine do
      local m = flow.machine[i]
      if m.name == 'math>tanh-x-' then 
         m.name = 'math>mapping-x-' 
         m.params.mapping = 'Tanh'
      end
      if m.name == 'math>abs-x-' then 
         m.name = 'math>mapping-x-' 
         m.params.mapping = 'Abs'
      end
      if m.name == 'math>nn-sigmoid-x-' then 
         m.name = 'math>mapping-x-' 
         m.params.mapping = 'StdSigm'
      end
   end

   -- process each level
   local doneAdvance = 0
   local outputs
   for i=1,#flow.machine do
      if doneAdvance > 0 then
         doneAdvance = doneAdvance - 1
      else
         local module_0, module_1, module_2, module_3, module_name
         local params_0, params_1, params_2, params_3
         module_0 = flow.machine[i+0].name
         params_0 = flow.machine[i+0].params
         if i+1 <= #flow.machine then
            module_1 = flow.machine[i+1].name
            params_1 = flow.machine[i+1].params
         end
         if i+2 <= #flow.machine then
            module_2 = flow.machine[i+2].name
            params_2 = flow.machine[i+2].params
         end
         if i+3 <= #flow.machine then
            module_3 = flow.machine[i+3].name
            params_3 = flow.machine[i+3].params
         end
         io.write('# processing layer of type > '..module_0)
         mapping = nil
         if self.opt_across_layers then
            -- combine multiple mappings together:
            if module_0 == 'math>mapping-x-' and module_1 == 'math>mapping-x-' then
               params_0.mapping = params_0.mapping .. params_1.mapping
               io.write(' merged with next layer > '..module_1..' >>> '..module_0
                        ..' ['.. params_0.mapping ..']')
               doneAdvance = 1
            elseif module_0 == 'math>mapping-x-' 
               and module_1 == 'math>x*y' and params_1.mode == 'array-wise'
               and module_2 == 'math>mapping-x-' then
               params_0.mapping = params_0.mapping .. params_2.mapping
               io.write(' merged with next layers > '..module_1..' & '..module_2..
                        ' >>> '..module_0 ..' ['.. params_0.mapping ..']')
               doneAdvance = 2

            -- combine a filter bank plus mappings:
            elseif module_0 == 'linear-filter-bank-2d>output' 
               and module_1 == 'math>mapping-x-' and module_2 == 'math>mapping-x-' then
               mapping = params_1.mapping .. params_2.mapping
               io.write(' merged with next layers > '..module_1..' & '..module_2..
                        ' >>> '..module_0)
               doneAdvance = 2
            elseif module_0 == 'linear-filter-bank-2d>output' 
               and module_1 == 'math>mapping-x-' then
               mapping = params_1.mapping
               io.write(' merged with next layers > '..module_1..' >>> '..module_0)
               doneAdvance = 1

            -- combiner a filter bank plus bias layer
            elseif module_0 == 'linear-filter-bank-2d>output' 
               and module_1 == 'math>x+y' and params_1.mode == 'array-wise' then
               -- array-wise adder can be merged into the convolver's bias
               flow.machine[i].params.bias:add(params_1.y)
               io.write(' merged with next layers > '..module_1..' >>> '..module_0)
               doneAdvance = 1
            end
         end
         print('')
         outputs = nodes_table[module_0](self, flow.machine[i], inputs, mapping)
         inputs = outputs
         if (self.msg_level == 'detailled') then
            self.core:startProcess()
            self.core:getTime()
            self.core:resetTime()
            self.core:endProcess()
         end
      end
   end
   self:printStats()
   return inputs
end

-- One function per type of layer:

function Compiler:SpatialConvolution(conv_module, inputs, mapping)
   local outputs = {}
   local new_layer = true

   if (self.msg_level ~= 'none') then
      self.core:startProcess()
      if mapping then
         self.core:message(string.format('SC+M'))
      else
         self.core:message(string.format('SC'))
      end
      self.core:endProcess()
   end

   -- timing info
   if (self.msg_level == 'timing') then
      self.core:startProcess()
      self.core:resetTime()
      self.core:endProcess()
   end
   
   local coefs
   if mapping then
      -- generate coefs for this non-linear mapping
      coefs = self:getCoefs(mapping)
   end

   -- lists of connections
   local input_list = {}
   local kernel_list = {}
   local output_list = {}

   -- store inputs
   for i = 1,conv_module.nInputPlane do
      table.insert(input_list, self.core.mem.buff[inputs[i]])
   end

   -- parse connections
   for o = 1,conv_module.nOutputPlane do
      -- allocate output
      local item = self.core.mem.buff[inputs[1]]
      local output_width = math.floor( (item.orig_w - conv_module.kW)/conv_module.dW + 1 )
      local output_height = (item.orig_h - conv_module.kH)/conv_module.dH + 1
      if output_height ~= math.floor(output_height) then
         error('# ERROR <Compiler> : inconsistent subsampling ratios in_h=' .. item.orig_h .. ', sub_h=' .. 
               conv_module.kH .. ', out_h=' .. output_height)
      end
      local id_output = self.core.mem:allocOnTheHeap(output_height, output_width, {}, new_layer)
      outputs[o] = id_output
      new_layer = false

      -- store output
      table.insert(output_list, self.core.mem.buff[outputs[o]])

      -- store kernels
      for i = 1,conv_module.nInputPlane do
         -- allocate kernel
         local kernel = conv_module.weight:select(3, i):select(3,o)
         local bias = conv_module.bias:narrow(1,o,1)
         local id_kernel = self.core.mem:allocKernel(conv_module.kH, conv_module.kW, kernel, bias)

         -- collect connections
         table.insert(kernel_list, self.core.mem.raw_data[id_kernel])

         -- for info, update the number of ops
         self.ops = self.ops + output_width*output_height*conv_module.kW*conv_module.kH*2
      end
   end

   -- compute whole convol bank
   self.core:convolBank(input_list, kernel_list, output_list, coefs)

   -- timing info
   if (self.msg_level == 'timing') then
      self.core:startProcess()
      self.core:getTime()
      self.core:endProcess()
   end

   return outputs
end

function Compiler:SpatialConvolutionTable(conv_module, inputs, mapping)
   local outputs = {}
   local new_layer = true
   local new_output = true
   local current_op = 1

   if (self.msg_level ~= 'none') then
      self.core:startProcess()
      if mapping then
         self.core:message(string.format('SCT+M'))
      else
         self.core:message(string.format('SCT'))
      end
      self.core:endProcess()
   end
   
   -- timing info
   if (self.msg_level == 'timing') then
      self.core:startProcess()
      self.core:resetTime()
      self.core:endProcess()
   end

   local coefs
   if mapping then
      -- generate coefs for this non-linear mapping
      coefs = self:getCoefs(mapping)
   end

   -- parse connex table, and identidy output reuse / one2one connex
   -- if outputs are used more than once, then they'll be reused
   local output_reuse = false
   local one_to_one = false
   local diff = (conv_module.connTable:select(2,1)-conv_module.connTable:select(2,2)):abs():max()
   if diff == 0 then
      one_to_one = true
   else
      for i = 1,conv_module.connTable:size(1) do
         local current = conv_module.connTable[i][2]
         for j = 1,conv_module.connTable:size(1) do
            if j ~= i and current == conv_module.connTable[j][2] then
               output_reuse = true
               break
            end
         end
         if output_reuse then break end
      end
   end

   -- depending on output/input reuse and one2one connex:
   if one_to_one then
      local input_list = {}
      local kernel_list = {}
      local output_list = {}

      for o = 1,conv_module.nOutputPlane do
         -- allocate output
         local item = self.core.mem.buff[inputs[1]]
         local output_width = math.floor( (item.orig_w - conv_module.kW)/conv_module.dW + 1 )
         local output_height = (item.orig_h - conv_module.kH)/conv_module.dH + 1
         if output_height ~= math.floor(output_height) then
            error('# ERROR <Compiler> : inconsistent subsampling ratios in_h=' .. item.orig_h .. ', sub_h=' .. 
                  conv_module.kH .. ', out_h=' .. output_height)
         end
         local id_output = self.core.mem:allocOnTheHeap(output_height, output_width, {}, new_layer)
         outputs[o] = id_output

         -- allocate kernel + bias
         local kernel = conv_module.weight:select(3, current_op)
         local bias = conv_module.bias:narrow(1,o,1)
         local id_kernel = self.core.mem:allocKernel(conv_module.kH, conv_module.kW, 
                                                     kernel, bias)

         -- collect connections
         table.insert(input_list, self.core.mem.buff[inputs[o]])
         table.insert(output_list, self.core.mem.buff[outputs[o]])
         table.insert(kernel_list, self.core.mem.raw_data[id_kernel])

         -- for info, update the number of ops
         self.ops = self.ops + output_width*output_height*conv_module.kW*conv_module.kH*2

         -- next connex
         current_op = current_op + 1
         new_layer = false
      end

      -- compute output
      self.core:convolBank(input_list, kernel_list, output_list, coefs)

   elseif output_reuse then
      for o = 1,conv_module.nOutputPlane do
         -- allocate output
         local item = self.core.mem.buff[inputs[1]]
         local output_width = math.floor( (item.orig_w - conv_module.kW)/conv_module.dW + 1 )
         local output_height = (item.orig_h - conv_module.kH)/conv_module.dH + 1
         if output_height ~= math.floor(output_height) then
            error('# ERROR <Compiler> : inconsistent subsampling ratios in_h=' .. item.orig_h .. ', sub_h=' .. 
                  conv_module.kH .. ', out_h=' .. output_height)
         end
         local id_output = self.core.mem:allocOnTheHeap(output_height, output_width, {}, new_layer)
         outputs[o] = id_output
         new_layer = false
         new_output = true

         local input_list = {}
         local kernel_list = {}
         local output_list = {self.core.mem.buff[id_output]}

         -- find all inputs
         for i = 1,conv_module.connTable:size(1) do
            if (o == conv_module.connTable[i][2]) then
               -- get input from table
               local input_p = conv_module.connTable[i][1]

               -- allocate kernel + bias
               local kernel = conv_module.weight:select(3, current_op)
               local bias = conv_module.bias:narrow(1,o,1)
               local id_kernel = self.core.mem:allocKernel(conv_module.kH, conv_module.kW, 
                                                           kernel, bias)

               -- collect connections
               table.insert(input_list, self.core.mem.buff[inputs[input_p]])
               table.insert(kernel_list, self.core.mem.raw_data[id_kernel])

               -- for info, update the number of ops
               self.ops = self.ops + output_width*output_height*conv_module.kW*conv_module.kH*2

               -- next connex
               new_output = false
               current_op = current_op + 1
            end
         end

         -- compute output
         self.core:convolBank(input_list, kernel_list, output_list, coefs)
      end

   else -- input reuse
      for i = 1,conv_module.nInputPlane do
         -- lists of outputs/kernels
         local input_list = {self.core.mem.buff[inputs[i]]}
         local kernel_list = {}
         local output_list = {}

         -- find all outputs
         for o = 1,conv_module.connTable:size(1) do
            if (i == conv_module.connTable[o][1]) then
               -- allocate output
               local item = self.core.mem.buff[inputs[1]]
               local output_width = math.floor( (item.orig_w - conv_module.kW)/conv_module.dW + 1 )
               local output_height = (item.orig_h - conv_module.kH)/conv_module.dH + 1
               if output_height ~= math.floor(output_height) then
                  error('# ERROR <Compiler> : inconsistent subsampling ratios in_h=' .. item.orig_h .. ', sub_h=' .. 
                        conv_module.kH .. ', out_h=' .. output_height)
               end
               local id_output = self.core.mem:allocOnTheHeap(output_height, output_width, {}, 
                                                              new_layer)
               outputs[o] = id_output

               -- allocate kernel + bias
               local kernel = conv_module.weight:select(3, current_op)
               local bias = conv_module.bias:narrow(1,o,1)
               local id_kernel = self.core.mem:allocKernel(conv_module.kH, conv_module.kW, 
                                                           kernel, bias)

               -- collect connections
               table.insert(output_list, self.core.mem.buff[id_output])
               table.insert(kernel_list, self.core.mem.raw_data[id_kernel])

               -- for info, update the number of ops
               self.ops = self.ops + output_width*output_height*conv_module.kW*conv_module.kH*2

               -- next connex
               current_op = current_op + 1
               new_layer = false
            end
         end

         -- compute all outputs for given input
         self.core:convolBank(input_list, kernel_list, output_list, coefs)         
      end
   end

   -- timing info
   if (self.msg_level == 'timing') then
      self.core:startProcess()
      self.core:getTime()
      self.core:endProcess()
   end

   return outputs
end

function Compiler:SpatialSubSampling(sub_module, inputs, mapping)
   local outputs = {}
   local new_layer = true

   if (self.msg_level ~= 'none') then
      self.core:startProcess()
      if mapping then
         self.core:message(string.format('SS+M'))
      else
         self.core:message(string.format('SS')) 
      end
      self.core:endProcess()
   end

   -- timing info
   if (self.msg_level == 'timing') then
      self.core:startProcess()
      self.core:resetTime()
      self.core:endProcess()
   end

   if mapping then
      -- generate coefs for this non-linear mapping
      coefs = self:getCoefs(mapping)
   end

   -- NEW
   do
      local input_list = {}
      local kernel_list = {}
      local output_list = {}

      for o = 1,sub_module.nInputPlane do
         -- allocate output
         local item = self.core.mem.buff[inputs[1]]
         local output_width = math.floor( (self.core.mem.buff[inputs[o]].orig_w-sub_module.kW)/sub_module.dW + 1)
         local output_height = (self.core.mem.buff[inputs[o]].orig_h-sub_module.kH)/sub_module.dH + 1
         if output_height ~= math.floor(output_height) then
            error('# ERROR <Compiler> : inconsistent subsampling ratios in_h=' .. item.orig_h .. ', sub_h=' .. 
               conv_module.kH .. ', out_h=' .. output_height)
         end
         local id_output = self.core.mem:allocOnTheHeap(output_height, output_width, {}, new_layer)
         outputs[o] = id_output

         -- allocate kernel + bias
         local kernel = torch.Tensor(sub_module.kW, sub_module.kH):fill(sub_module.weight[o])
         local bias = sub_module.bias:narrow(1,o,1)
         local id_kernel = self.core.mem:allocKernel(sub_module.kH, sub_module.kW, 
                                                     kernel, bias)

         -- collect connections
         table.insert(input_list, self.core.mem.buff[inputs[o]])
         table.insert(output_list, self.core.mem.buff[outputs[o]])
         table.insert(kernel_list, self.core.mem.raw_data[id_kernel])

         -- for info, update the number of ops
         self.ops = self.ops + output_width*output_height*sub_module.kW*sub_module.kH*2

         -- next connex
         new_layer = false
      end

      -- compute output
      self.core:convolBank(input_list, kernel_list, output_list, coefs)
   end

   -- timing info
   if (self.msg_level == 'timing') then
      self.core:startProcess()
      self.core:getTime()
      self.core:endProcess()
   end

   return outputs
end

function Compiler:LocalNorm(sub_module, inputs)      
   -- verbose
   if (self.msg_level ~= 'none') then
      self.core:startProcess()
      self.core:message(string.format('NZ')) 
      self.core:endProcess()
   end

   -- timing info
   if (self.msg_level == 'timing') then
      self.core:startProcess()
      self.core:resetTime()
      self.core:endProcess()
   end

   -- alloc one kernel for the whole layer
   local kernel = sub_module.kernel
   local kernel_w = kernel:size(1)
   local kernel_h = kernel:size(2)
   local id_kernel_mean = self.core.mem:allocRawData(kernel_h, kernel_w, kernel)
   local id_kernel_std = self.core.mem:allocRawData(kernel_h, kernel_w, kernel)

   -- alloc one intermediate map (to hold zero-mean feature map)
   local zerom_w = self.core.mem.buff[inputs[1]].orig_w
   local zerom_h = self.core.mem.buff[inputs[1]].orig_h
   local zeros = {}
   local new_layer = true
   for i = 1,sub_module.nfeatures do
      zeros[i] = self.core.mem:allocOnTheHeap(zerom_h, zerom_w, {}, new_layer)
      new_layer = false
   end

   -- alloc all output maps
   local outputs = {}
   local new_layer = true
   local output_w = self.core.mem.buff[inputs[1]].orig_w
   local output_h = self.core.mem.buff[inputs[1]].orig_h
   for i = 1,sub_module.nfeatures do
      outputs[i] = self.core.mem:allocOnTheHeap(output_h, output_w, {}, new_layer)
   end

   -- collect inputs/outputs/kernels
   local input_maps = {}
   local zero_maps = {}
   local output_maps = {}
   local mean_kernels = {}
   local std_kernels = {}
   for i = 1,sub_module.nfeatures do
      table.insert(input_maps, self.core.mem.buff[inputs[i]])
      table.insert(zero_maps, self.core.mem.buff[zeros[i]])
      table.insert(output_maps, self.core.mem.buff[outputs[i]])
      table.insert(mean_kernels, self.core.mem.raw_data[id_kernel_mean])
      table.insert(std_kernels, self.core.mem.raw_data[id_kernel_std])
   end

   -- local norm mean
   self.core:localNormalizeMeanBank(input_maps, mean_kernels, zero_maps)
   --self.core:localNormalizeMean(input_maps[1], mean_kernels[1], zero_maps[1])
   
   -- do only zero mean
   --self.core:localNormalizeMean(input_maps[1], mean_kernels[1], output_maps[1])
   
   -- local norm std
   self.core:localNormalizeStdBank(zero_maps, std_kernels, output_maps, 1/256)
   --self.core:localNormalizeStd(zero_maps[1], std_kernels[1], output_maps[1], 1/256)

   -- for info, update the number of ops
   self.ops = self.ops + (output_w*output_h*kernel_w*kernel_h*2
                          + zerom_w*zerom_h*(kernel_w*kernel_h*2 + 16)) * sub_module.nfeatures

   -- timing info
   if (self.msg_level == 'timing') then
      self.core:startProcess()
      self.core:getTime()
      self.core:endProcess()
   end

   -- return output maps
   return outputs
end

function Compiler:SpatialLinear(linear_module, inputs)
   local outputs = {}
   local new_layer = true

   if (self.msg_level ~= 'none') then
      self.core:startProcess()
      self.core:message(string.format('SL'))
      self.core:endProcess()
   end

   for o = 1,linear_module.fanout do
      -- allocate output
      local item = self.core.mem.buff[inputs[1]]
      local output_width = item.orig_w
      local output_height = item.orig_h
      local id_output = self.core.mem:allocOnTheHeap(output_height, output_width, {}, new_layer)
      outputs[o] = id_output
      new_layer = false

      for i = 1,linear_module.fanin do
         -- allocate kernel
         local kernel = torch.Tensor(1, 1):fill(linear_module.weight[i][o])
         self.logfile:write(string.format("ker #(%d,%d):\n", i,o))
         self.logfile:write(tostring(kernel))
         local id_kernel = self.core.mem:allocRawData(1, 1, kernel)

         -- for info, update the number of ops
         self.ops = self.ops + output_width*output_height*3
         
         -- generate code for convolution
         if (i == 1) then
            self.core:convolve(self.core.mem.buff[inputs[i]], 
                               self.core.mem.raw_data[id_kernel], 
                               self.core.mem.buff[id_output])
         else
            self.core:convolveAndAcc(self.core.mem.buff[inputs[i]], 
                                     self.core.mem.raw_data[id_kernel], 
                                     self.core.mem.buff[outputs[o]], 
                                     self.core.mem.buff[outputs[o]])
            -- nb of ops
            self.ops = self.ops + output_width*output_height
         end

         -- optional time
         if (self.msg_level == 'detailled') then
            self.core:startProcess()
            self.core:messagebody('.')
            self.core:endProcess()
         end
      end
   end
   return outputs
end

function Compiler:getCoefs(mapping)
   local type = mapping
      
   -- generate coefs for this non-linear mapping
   if type == 'Tanh' then
      coefs=math.approx{mapping=math.tanh, min=-5, max=5, odd=true,
			nbSegments=grid.mapper_segs, Q=num.frac_,
			verbose=true, epsilon = 11.7/256, error_type = 0,
			name = type}
      
   elseif type == 'Abs' then
      coefs=math.approx{mapping=math.abs, min=num.min, max=num.max, even=true,
			nbSegments=grid.mapper_segs, Q=num.frac_,
			verbose=true, error_type = 0,
			name = type}
   elseif type == 'TanhAbs' then
      function tanhabs (x) return math.abs(math.tanh(x)) end 
      coefs=math.approx{mapping=tanhabs, min=-5, max=5, even=true,
			nbSegments=grid.mapper_segs, Q=num.frac_,
			verbose=true, epsilon = 11.7/256, error_type = 0,
			name = type}
   elseif type == 'StdSigm' then
      function stdsigm (x) return 1.71593428 * math.tanh(0.66666666*x) end 
      coefs=math.approx{mapping=stdsigm, min=num.min, max=num.max, odd=true,
			nbSegments=grid.mapper_segs, Q=num.frac_,
			verbose=true,epsilon = 4/256, error_type = 1,
			name = 'StdSigm_abs_err_all_range'}--type}
   elseif type == 'StdSigmAbs' then
      function stdsigm (x) return 1.71593428 * math.tanh(0.66666666*x) end 
      coefs=math.approx{mapping=stdsigm, min=-5.5, max=5.5, even=true,
			nbSegments=grid.mapper_segs, Q=num.frac_,
			verbose=true, epsilon = 32.21/256, error_type = 0,
			name = type}
   elseif type == 'Sqrt' then
      coefs=math.approx{mapping=math.sqrt, min=0, max=num.max,
			nbSegments=grid.mapper_segs, Q=num.frac_,
			verbose=true, epsilon = 19.7/256, error_type = 0,
			name = type}
   else
      error('# ERROR <Compiler> : unknown mapping')
   end
   
   return coefs
end

function Compiler:Mapping(module, inputs, type)
   local outputs = {}

   if (self.msg_level ~= 'none') then
      self.core:startProcess()
      self.core:message(string.format('doing Tanh [%0d maps]', #inputs))
      self.core:endProcess()
   end
   
   -- generate coefs for this non-linear mapping
   coefs = self:getCoefs(type)

   -- generate code
   for i = 1,#inputs do
      local id_output = self.core.mem:allocOnTheHeap(self.core.mem.buff[inputs[i]].orig_h,
                                                     self.core.mem.buff[inputs[i]].orig_w , {}, false)
      self.core:mapping(self.core.mem.buff[inputs[i]], self.core.mem.buff[id_output], coefs)

      -- optional time
      if (self.msg_level == 'detailled') then
         self.core:startProcess()
         self.core:messagebody('.')
         self.core:endProcess()
      end
      outputs[i] = id_output

      -- for info (16 is approx here, it's hard to say what a mapping takes)
      self.ops = self.ops + self.core.mem.buff[inputs[i]].orig_h*self.core.mem.buff[inputs[i]].orig_w*16
   end
   return outputs
end

function Compiler:CCSub(module, inputs)
   local outputs = {}

   if (self.msg_level ~= 'none') then
      self.core:startProcess()
      self.core:message(string.format('doing CCSub [%0d maps]', #inputs))
      self.core:endProcess()
   end
   
   -- 2 inputs required
   if #inputs ~= 2 then
      error('<Compiler:CCSub> 2 inputs required')
   end

   -- alloc output
   outputs[1] = self.core.mem:allocOnTheHeap(self.core.mem.buff[inputs[1]].orig_h,
                                             self.core.mem.buff[inputs[1]].orig_w , {}, false)

   -- generate code
   self.core:subtract(self.core.mem.buff[inputs[1]],
                      self.core.mem.buff[inputs[2]],
                      self.core.mem.buff[outputs[1]])

   -- optional time
   if (self.msg_level == 'detailled') then
      self.core:startProcess()
      self.core:messagebody('.')
      self.core:endProcess()
   end

   -- for info
   self.ops = self.ops + self.core.mem.buff[inputs[1]].orig_h*self.core.mem.buff[inputs[1]].orig_w
   return outputs
end

function Compiler:CCAdd(module, inputs)
   local outputs = {}

   if (self.msg_level ~= 'none') then
      self.core:startProcess()
      self.core:message(string.format('doing CCSub [%0d maps]', #inputs))
      self.core:endProcess()
   end
   
   -- 2 inputs required
   if #inputs ~= 2 then
      error('<Compiler:CCSub> 2 inputs required')
   end

   -- alloc output
   outputs[1] = self.core.mem:allocOnTheHeap(self.core.mem.buff[inputs[1]].orig_h,
                                             self.core.mem.buff[inputs[1]].orig_w , {}, false)

   -- generate code
   self.core:add(self.core.mem.buff[inputs[1]],
                      self.core.mem.buff[inputs[2]],
                      self.core.mem.buff[outputs[1]])

   -- optional time
   if (self.msg_level == 'detailled') then
      self.core:startProcess()
      self.core:messagebody('.')
      self.core:endProcess()
   end

   -- for info
   self.ops = self.ops + self.core.mem.buff[inputs[1]].orig_h*self.core.mem.buff[inputs[1]].orig_w
   return outputs
end


function Compiler:Reshape(reshape_module, inputs)
   -- just do nothing here??? because the output is going to be 
   -- the next input and that is defined in self.core.mem.
   -- or maybe we need to extract output and pass it to the next layer???????
   local outputs = {}
   outputs[1] = self.core.mem:allocOnTheHeap(reshape_module.output:size(1), 
                                             reshape_module.output:size(2), 
                                             reshape_module.output, true)
   return outputs
end

function Compiler:Threshold(threshold_module, inputs)
   -- just do nothing here??? because the output is going to 
   -- be the next input and that is defined in self.core.mem.
   -- or maybe we need to extract output and pass it to the next layer???????
   local outputs = {}
   outputs[1] = self.core.mem:allocOnTheHeap(threshold_module.output:size(1),
                                             threshold_module.output:size(2),
                                             threshold_module.output, true)
   return outputs
end

function Compiler:printStats()
   str = string.format('network computed requires %f MOPs', self.ops/1000000.)
   print('# '..str)
   return str
end
