--------------------------------------------------------------------------------
-- xFlow.libs.neural: a class that provides all the neural operators.
-- 
-- what?  xFlow programs are abstract representations of computational graphs.
--        xLib is a library of black-box operators that are essential to build
--        any actual program.
--
-- version: 0.2
--------------------------------------------------------------------------------


----------------------------------------------------------------------
--- xFlow.libs.neural
-- The standard xFlow neural library. Defines all the basic operators.
-- The code is based on closures, each call to a specific node returns
-- a function with an internal state that is ready to insert into
-- a ComputableFlow object.
--
if not xFlow.libs then xFlow.libs = {} end
xFlow.libs.neural = {}

-- messages
local ERROR_MISSING_VAR = '# ERROR: nil variable > '
local ERROR_BAD_PARAMS = '# ERROR: bad parameters > '


----------------------------------------------------------------------
--- Initializes the object
-- An XFLowLib object can then be used to retrieve standard compute
-- nodes as defined in the standard xLib.
--
-- Example:
-- > xlib = {}
-- > xFlow.libs.neural(xlib)
-- > inputNodes = {x=torch.Tensor(10,10), k=torch.Tensor(1)}
-- > outputNode = torch.Tensor()
-- > computeNode = xlib['compute-node']['math']['x^k'](inputNodes, outputNode)
-- # at this stage, computeNode is a function that can be used
-- # to update the outputNode, based on the current state of
-- # its inputNodes, e.g.
-- > inputNodes.x:copy(someTensor)
-- > k[1] = 4
-- > computeNode()
-- # result: outputNode is now a tensor of the size of inputNode.x,
-- # and equals inputNode.x ^ 4 
-- 
-- @param stack   a stack to hold symbols (can be an empty list)
--
function xFlow.libs.neural.load(stack)
   -- define all compulsory slots:
   stack['version'] = 0.2
   if not stack['compute-node'] then 
      stack['compute-node'] = {}
      stack['data-node'] = {}
      stack['input-node'] = {}
      stack['output-node'] = {}
   end

   -- node: linear-combination
   stack['compute-node']['linear-combination'] = {}
   stack['compute-node']['linear-combination']['output'] = 
      function (inputs,output)
         assert(inputs['input'], ERROR_MISSING_VAR .. 'input')
         assert(inputs['weight'], ERROR_MISSING_VAR .. 'weight')
         assert(output, ERROR_MISSING_VAR .. 'output')
         local input = inputs['input']
         local weight = inputs['weight']
         local fanin = weight:size(1)
         local fanout = weight:size(2)
         local bias = inputs['bias'] or torch.Tensor(fanout):zero()
         local linear = nn.SpatialLinear(fanin, fanout)
         linear.weight:copy(weight)
         linear.bias:copy(bias)
         local name = 'linear-combination>output'
         local update = function (args)
                           if args and args.verbose then print('updating: '..name) end
                           local out = linear:forward(input)
                           output:resizeAs(out):copy(out)
                        end
         return {update=update,name=name,output=output,params=linear}
      end

   -- node: linear-filter-bank-2d
   stack['compute-node']['linear-filter-bank-2d'] = {}
   stack['compute-node']['linear-filter-bank-2d']['output'] = 
      function (inputs,output)
         assert(inputs['input'], ERROR_MISSING_VAR .. 'input')
         assert(inputs['weight'], ERROR_MISSING_VAR .. 'weight')
         assert(output, ERROR_MISSING_VAR .. 'output')
         local input = inputs['input']
         local weight = inputs['weight']
         local fanin = weight:size(3) -- default fanin if no table
         local connex = inputs['connex']:add(1):t() or nn.SpatialConvolutionTable:OneToOneTable(fanin)
         fanin = connex:narrow(2,1,1):max()
         local fanout = connex:narrow(2,2,1):max()
         local kerw = weight:size(1)
         local kerh = weight:size(1)
         local stride = inputs['stride'] or torch.Tensor(2):fill(1)
         local stridey = stride[1]
         local stridex = stride[2]
         local bias = inputs['bias'] or torch.Tensor(fanout):zero()
         local linear_filter_2d = nn.SpatialConvolutionTable(connex, kerw, kerh, stridex, stridey)
         linear_filter_2d.weight:copy(weight)
         linear_filter_2d.bias:copy(bias)
         local name = 'linear-filter-bank-2d>output'
         local update = function (args)
                           if args and args.verbose then print('updating: '..name) end
                           local out = linear_filter_2d:forward(input)
                           output:resizeAs(out):copy(out)
                        end
         return {update=update,name=name,output=output,params=linear_filter_2d}
      end

   -- node: std-normalization-2d
   stack['compute-node']['std-normalization-2d'] = {}
   stack['compute-node']['std-normalization-2d']['output'] = 
      function (inputs,output)
         assert(inputs['input'], ERROR_MISSING_VAR .. 'input')
         assert(inputs['mean-kernel'], ERROR_MISSING_VAR .. 'mean-kernel')
         assert(inputs['std-kernel'], ERROR_MISSING_VAR .. 'std-kernel')
         assert(output, ERROR_MISSING_VAR .. 'output')
         local input = inputs['input']
         local mean_kernel = inputs['mean-kernel']
         local std_kernel = inputs['std-kernel']
         local std_threshold = inputs['std-threshold'] or torch.Tensor(1):fill(0)
         local std_min = inputs['std-min'] or torch.Tensor(1):fill(0)
         local std_kernel_w = std_kernel:size(1)
         local std_kernel_h = std_kernel:size(1)
         local same_size = inputs['same-size'] or {1}
         local normer_2d = nn.LocalNorm(std_kernel:select(3,1), std_kernel:size(3))
         local name = 'std-normalization-2d>output'
         local update = function (args)
                           if args and args.verbose then print('updating: '..name) end
                           local out = normer_2d:forward(input)
                           if same_size[1] == 0 then
                              local out_valid = out:narrow(1, math.ceil(std_kernel_w/2),
                                                           out:size(1)-std_kernel_w+1)
                              out_valid = out_valid:narrow(2, math.ceil(std_kernel_h/2),
                                                           out:size(2)-std_kernel_h+1)
                              output:resizeAs(out_valid):copy(out_valid)
                           else
                              output:resizeAs(out):copy(out)
                           end
                        end
         return {update=update,name=name,output=output,params=normer_2d}
      end

   -- node: average-pooling-2d
   stack['compute-node']['average-pooling-2d'] = {}
   stack['compute-node']['average-pooling-2d']['output'] = 
      function (inputs,output)
         assert(inputs['input'], ERROR_MISSING_VAR .. 'input')
         assert(inputs['weight'], ERROR_MISSING_VAR .. 'weight')
         assert(output, ERROR_MISSING_VAR .. 'output')
         local input = inputs['input']
         local weight = inputs['weight']
         local fanin = weight:size(1)
         local stride = inputs['stride'] or torch.Tensor(2):fill(1)
         local stridex = stride[1]
         local stridey = stride[2]
         local bias = inputs['bias'] or torch.Tensor(fanin):zero()
         local average_pooling_2d = nn.SpatialSubSampling(fanin, stridex, stridey, stridex, stridey)
         average_pooling_2d.weight:copy(weight)
         average_pooling_2d.bias:copy(bias)
         local name = 'average-pooling-2d>output'
         local update = function (args)
                           if args and args.verbose then print('updating: '..name) end
                           local out = average_pooling_2d:forward(input)
                           output:resizeAs(out):copy(out)
                        end
         return {update=update,name=name,output=output,params=average_pooling_2d}
      end

   -- set loaded flag
   xFlow.libs.neural.loaded = true
end
