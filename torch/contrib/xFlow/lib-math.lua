--------------------------------------------------------------------------------
-- xFlow.libs.math: a class that provides all the math operators.
-- 
-- what?  xFlow programs are abstract representations of computational graphs.
--        xLib is a library of black-box operators that are essential to build
--        any actual program.
--
-- version: 0.2
--------------------------------------------------------------------------------


----------------------------------------------------------------------
--- xFlow.libs.math
-- The standard xFlow math library. Defines all the basic operators.
-- The code is based on closures, each call to a specific node returns
-- a function with an internal state that is ready to insert into
-- a ComputableFlow object.
--
if not xFlow.libs then xFlow.libs = {} end
xFlow.libs.math = {}

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
function xFlow.libs.math.load(stack)
   -- define all compulsory slots:
   stack['version'] = 0.2
   if not stack['compute-node'] then 
      stack['compute-node'] = {}
      stack['data-node'] = {}
      stack['input-node'] = {}
      stack['output-node'] = {}
   end

   -- data nodes: constants
   stack['data-node']['pi'] = {'scalar', {'type', 'float'}, {'value', '3.1415926535897931'}}
   stack['data-node']['phi'] = {'scalar', {'type', 'float'}, {'value', '1.6180339887498948'}}

   -- node: math
   stack['compute-node']['math'] = {}
   stack['compute-node']['math']['-x'] = 
      function (inputs,output)
         assert(inputs['x'], ERROR_MISSING_VAR .. 'x')
         assert(output, ERROR_MISSING_VAR .. 'output')
         local in1 = inputs['x']
         local out1 = output
         local name = 'math>-x'
         local update = function (args)
                           if args and args.verbose then print('updating: '..name) end
                           out1:resizeAs(in1):copy(in1):mul(-1)
                        end
         return {update=update,name=name,output=output,params={}}
      end
   stack['compute-node']['math']['1/x'] = 
      function (inputs,output)
         assert(inputs['x'], ERROR_MISSING_VAR .. 'x')
         assert(output, ERROR_MISSING_VAR .. 'output')
         local in1 = inputs['x']
         local out1 = output
         local name = 'math>1/x'
         local update = function (args)
                           if args and args.verbose then print('updating: '..name) end
                           out1:resizeAs(in1):fill(1):cdiv(in1)
                        end
         return {update=update,name=name,output=output,params={}}
      end
   stack['compute-node']['math']['x+y'] = 
      function (inputs,output)
         assert(inputs['x'], ERROR_MISSING_VAR .. 'x')
         assert(inputs['y'], ERROR_MISSING_VAR .. 'y')
         assert(output, ERROR_MISSING_VAR .. 'output')
         local x = inputs['x']
         local y = inputs['y']
         local out1 = output
         local name = 'math>x+y'
         local params = {x=x,y=y,mode='array-wise'}
         local update = function (args)
                           if args and args.verbose then print('updating: '..name) end
                           -- resize
                           out1:resizeAs(x):copy(x)
                           -- find mode based on input sizes
                           if y:size(1) == x:size(x:size():size()) and y:size():size() == 1 then
                              -- outer replicated mode: elt-wise operation for each subtensor
                              for i = 1,y:size(1) do
                                 out1:select(out1:size():size(),i):add(y[i])
                              end
                              params.mode = 'array-wise'
                           else
                              -- default mode: elt-wise operation
                              out1:resizeAs(x):copy(x):add(y)
                              params.mode = 'elt-wise'
                           end
                        end
         return {update=update,name=name,output=output,params=params}
      end
   stack['compute-node']['math']['x*y'] = 
      function (inputs,output)
         assert(inputs['x'], ERROR_MISSING_VAR .. 'x')
         assert(inputs['y'], ERROR_MISSING_VAR .. 'y')
         assert(output, ERROR_MISSING_VAR .. 'output')
         local x = inputs['x']
         local y = inputs['y']
         local out1 = output
         local name = 'math>x*y'
         local params = {x=x,y=y,mode='array-wise'}
         local update = function (args)
                           if args and args.verbose then print('updating: '..name) end
                           -- resize
                           out1:resizeAs(x):copy(x)
                           -- find mode based on input sizes
                           if y:size(1) == x:size(x:size():size()) and y:size():size() == 1 then
                              -- outer replicated mode: elt-wise operation for each subtensor
                              for i = 1,y:size(1) do
                                 out1:select(out1:size():size(),i):mul(y[i])
                              end
                              params.mode = 'array-wise'
                           else
                              -- default mode: elt-wise operation
                              out1:resizeAs(x):copy(x):cmul(y)
                              params.mode = 'elt-wise'
                           end
                        end
         return {update=update,name=name,output=output,params=params}
      end
   stack['compute-node']['math']['x/y'] = 
      function (inputs,output)
         assert(inputs['x'], ERROR_MISSING_VAR .. 'x')
         assert(inputs['y'], ERROR_MISSING_VAR .. 'y')
         assert(output, ERROR_MISSING_VAR .. 'output')
         local x = inputs['x']
         local y = inputs['y']
         local out1 = output
         local name = 'math>x/y'
         local params = {x=x,y=y,mode='array-wise'}
         local update = function (args)
                           if args and args.verbose then print('updating: '..name) end
                           -- resize
                           out1:resizeAs(x):copy(x)
                           -- find mode based on input sizes
                           if y:size(1) == x:size(x:size():size()) and y:size():size() == 1 then
                              -- outer replicated mode: elt-wise operation for each subtensor
                              for i = 1,y:size(1) do
                                 out1:select(out1:size():size(),i):div(y[i])
                              end
                              params.mode = 'array-wise'
                           else
                              -- default mode: elt-wise operation
                              out1:resizeAs(x):copy(x):cdiv(y)
                              params.mode = 'elt-wise'
                           end
                        end
         return {update=update,name=name,output=output,params=params}
      end
   stack['compute-node']['math']['x-y'] = 
      function (inputs,output)
         assert(inputs['x'], ERROR_MISSING_VAR .. 'x')
         assert(inputs['y'], ERROR_MISSING_VAR .. 'y')
         assert(output, ERROR_MISSING_VAR .. 'output')
         local x = inputs['x']
         local y = inputs['y']
         local out1 = output
         local name = 'math>x-y'
         local params = {x=x,y=y,mode='array-wise'}
         local update = function (args)
                           if args and args.verbose then print('updating: '..name) end
                           -- resize
                           out1:resizeAs(x):copy(x)
                           -- find mode based on input sizes
                           if y:size(1) == x:size(x:size():size()) and y:size():size() == 1 then
                              -- outer replicated mode: elt-wise operation for each subtensor
                              for i = 1,y:size(1) do
                                 out1:select(out1:size():size(),i):add(-y[i])
                              end
                              params.mode = 'array-wise'
                           else
                              -- default mode: elt-wise operation
                              out1:resizeAs(x):copy(x):add(-1,y)
                              params.mode = 'elt-wise'
                           end
                        end
         return {update=update,name=name,output=output,params=params}
      end
   stack['compute-node']['math']['x*y+k'] = 
      function (inputs,output)
         assert(inputs['x'], ERROR_MISSING_VAR .. 'x')
         assert(inputs['y'], ERROR_MISSING_VAR .. 'y')
         assert(inputs['k'], ERROR_MISSING_VAR .. 'k')
         assert(output, ERROR_MISSING_VAR .. 'output')
         local x = inputs['x']
         local y = inputs['y']
         local k = inputs['k']
         local out1 = output
         local name = 'math>x*y+k'
         local params = {x=x,y=y,mode='array-wise'}
         local update = function (args)
                           if args and args.verbose then print('updating: '..name) end
                           if args and args.verbose then print('updating: '..name) end
                           -- resize
                           out1:resizeAs(x):copy(x)
                           -- find mode based on input sizes
                           if y:size(1) == x:size(x:size():size()) and y:size():size() == 1 then
                              -- outer replicated mode: elt-wise operation for each subtensor
                              for i = 1,y:size(1) do
                                 out1:select(out1:size():size(),i):mul(y[i]):add(k[i])
                              end
                              params.mode = 'array-wise'
                           else
                              -- default mode: elt-wise operation
                              out1:resizeAs(x):copy(x):cmul(y):add(k)
                              params.mode = 'elt-wise'
                           end
                        end
         return {update=update,name=name,output=output,params=params}
      end
   stack['compute-node']['math']['log-x-'] = 
      function (inputs,output)
         assert(inputs['x'], ERROR_MISSING_VAR .. 'x')
         assert(output, ERROR_MISSING_VAR .. 'output')
         local in1 = inputs['x']
         local out1 = output
         local name = 'math>log-x-'
         local update = function (args)
                           if args and args.verbose then print('updating: '..name) end
                           out1:resizeAs(in1):copy(in1):log()
                        end
         return {update=update,name=name,output=output,params={}}
      end
   stack['compute-node']['math']['exp-x-'] = 
      function (inputs,output)
         assert(inputs['x'], ERROR_MISSING_VAR .. 'x')
         assert(output, ERROR_MISSING_VAR .. 'output')
         local in1 = inputs['x']
         local out1 = output
         local name = 'math>exp-x-'
         local update = function (args)
                           if args and args.verbose then print('updating: '..name) end
                           out1:resizeAs(in1):copy(in1):exp()
                        end
         return {update=update,name=name,output=output,params={}}
      end
   stack['compute-node']['math']['cos-x-'] = 
      function (inputs,output)
         assert(inputs['x'], ERROR_MISSING_VAR .. 'x')
         assert(output, ERROR_MISSING_VAR .. 'output')
         local in1 = inputs['x']
         local out1 = output
         local name = 'math>cos-x-'
         local update = function (args)
                           if args and args.verbose then print('updating: '..name) end
                           out1:resizeAs(in1):copy(in1):cos()
                        end
         return {update=update,name=name,output=output,params={}}
      end
   stack['compute-node']['math']['sin-x-'] = 
      function (inputs,output)
         assert(inputs['x'], ERROR_MISSING_VAR .. 'x')
         assert(output, ERROR_MISSING_VAR .. 'output')
         local in1 = inputs['x']
         local out1 = output
         local name = 'math>sin-x-'
         local update = function (args)
                           if args and args.verbose then print('updating: '..name) end
                           out1:resizeAs(in1):copy(in1):sin()
                        end
         return {update=update,name=name,output=output,params={}}
      end
   stack['compute-node']['math']['tan-x-'] = 
      function (inputs,output)
         assert(inputs['x'], ERROR_MISSING_VAR .. 'x')
         assert(output, ERROR_MISSING_VAR .. 'output')
         local in1 = inputs['x']
         local out1 = output
         local name = 'math>tan-x-'
         local update = function (args)
                           if args and args.verbose then print('updating: '..name) end
                           out1:resizeAs(in1):copy(in1):tan()
                        end
         return {update=update,name=name,output=output,params={}}
      end
   stack['compute-node']['math']['acos-x-'] = 
      function (inputs,output)
         assert(inputs['x'], ERROR_MISSING_VAR .. 'x')
         assert(output, ERROR_MISSING_VAR .. 'output')
         local in1 = inputs['x']
         local out1 = output
         local name = 'math>acos-x-'
         local update = function (args)
                           if args and args.verbose then print('updating: '..name) end
                           out1:resizeAs(in1):copy(in1):acos()
                        end
         return {update=update,name=name,output=output,params={}}
      end
   stack['compute-node']['math']['asin-x-'] = 
      function (inputs,output)
         assert(inputs['x'], ERROR_MISSING_VAR .. 'x')
         assert(output, ERROR_MISSING_VAR .. 'output')
         local in1 = inputs['x']
         local out1 = output
         local name = 'math>asin-x-'
         local update = function (args)
                           if args and args.verbose then print('updating: '..name) end
                           out1:resizeAs(in1):copy(in1):asin()
                        end
         return {update=update,name=name,output=output,params={}}
      end
   stack['compute-node']['math']['atan-x-'] = 
      function (inputs,output)
         assert(inputs['x'], ERROR_MISSING_VAR .. 'x')
         assert(output, ERROR_MISSING_VAR .. 'output')
         local in1 = inputs['x']
         local out1 = output
         local name = 'math>atan-x-'
         local update = function (args)
                           if args and args.verbose then print('updating: '..name) end
                           out1:resizeAs(in1):copy(in1):atan()
                        end
         return {update=update,name=name,output=output,params={}}
      end
   stack['compute-node']['math']['cosh-x-'] = 
      function (inputs,output)
         assert(inputs['x'], ERROR_MISSING_VAR .. 'x')
         assert(output, ERROR_MISSING_VAR .. 'output')
         local in1 = inputs['x']
         local out1 = output
         local name = 'math>cosh-x-'
         local update = function (args)
                           if args and args.verbose then print('updating: '..name) end
                           out1:resizeAs(in1):copy(in1):cosh()
                        end
         return {update=update,name=name,output=output,params={}}
      end
   stack['compute-node']['math']['sinh-x-'] = 
      function (inputs,output)
         assert(inputs['x'], ERROR_MISSING_VAR .. 'x')
         assert(output, ERROR_MISSING_VAR .. 'output')
         local in1 = inputs['x']
         local out1 = output
         local name = 'math>sinh-x-'
         local update = function (args)
                           if args and args.verbose then print('updating: '..name) end
                           out1:resizeAs(in1):copy(in1):sinh()
                        end
         return {update=update,name=name,output=output,params={}}
      end
   stack['compute-node']['math']['tanh-x-'] = 
      function (inputs,output)
         assert(inputs['x'], ERROR_MISSING_VAR .. 'x')
         assert(output, ERROR_MISSING_VAR .. 'output')
         local in1 = inputs['x']
         local out1 = output
         local name = 'math>tanh-x-'
         local update = function (args)
                           if args and args.verbose then print('updating: '..name) end
                           out1:resizeAs(in1):copy(in1):tanh()
                        end
         return {update=update,name=name,output=output,params={}}
      end
   stack['compute-node']['math']['sigmoid-x-'] = 
      function (inputs,output)
         assert(inputs['x'], ERROR_MISSING_VAR .. 'x')
         assert(output, ERROR_MISSING_VAR .. 'output')
         local in1 = inputs['x']
         local out1 = output
         local sigm = nn.Sigmoid()
         sigm.output = out1
         local name = 'math>sigmoid-x-'
         local update = function (args)
                           if args and args.verbose then print('updating: '..name) end
                           sigm:forward(in1)
                        end
         return {update=update,name=name,output=output,params={}}
      end
   stack['compute-node']['math']['nn-sigmoid-x-'] = 
      function (inputs,output)
         assert(inputs['x'], ERROR_MISSING_VAR .. 'x')
         assert(output, ERROR_MISSING_VAR .. 'output')
         local in1 = inputs['x']
         local out1 = output
         local name = 'math>nn-sigmoid-x-'
         local update = function (args)
                           if args and args.verbose then print('updating: '..name) end
                           out1:resizeAs(in1):copy(in1):mul(0.66666666):tanh():mul(1.71593428)
                        end
         return {update=update,name=name,output=output,params={}}
      end
   stack['compute-node']['math']['hard-tanh-x-'] = 
      function (inputs,output)
         assert(inputs['x'], ERROR_MISSING_VAR .. 'x')
         assert(output, ERROR_MISSING_VAR .. 'output')
         local in1 = inputs['x']
         local out1 = output
         local sigm = nn.HardTanh()
         sigm.output = out1
         local name = 'math>hard-tanh-x-'
         local update = function (args)
                           if args and args.verbose then print('updating: '..name) end
                           sigm:forward(in1)
                        end
         return {update=update,name=name,output=output,params={}}
      end
   stack['compute-node']['math']['x^2'] = 
      function (inputs,output)
         assert(inputs['x'], ERROR_MISSING_VAR .. 'x')
         assert(output, ERROR_MISSING_VAR .. 'output')
         local in1 = inputs['x']
         local out1 = output
         local name = 'math>x^2'
         local update = function (args)
                           if args and args.verbose then print('updating: '..name) end
                           out1:resizeAs(in1):copy(in1):pow(2)
                        end
         return {update=update,name=name,output=output,params={}}
      end
   stack['compute-node']['math']['x^3'] = 
      function (inputs,output)
         assert(inputs['x'], ERROR_MISSING_VAR .. 'x')
         assert(output, ERROR_MISSING_VAR .. 'output')
         local in1 = inputs['x']
         local out1 = output
         local name = 'math>x^3'
         local update = function (args)
                           if args and args.verbose then print('updating: '..name) end
                           out1:resizeAs(in1):copy(in1):pow(3)
                        end
         return {update=update,name=name,output=output,params={}}
      end
   stack['compute-node']['math']['x^k'] = 
      function (inputs,output)
         assert(inputs['x'], ERROR_MISSING_VAR .. 'x')
         assert(inputs['k'], ERROR_MISSING_VAR .. 'k')
         assert(output, ERROR_MISSING_VAR .. 'output')
         local in1 = inputs['x']
         local in2 = inputs['k']
         local out1 = output
         local name = 'math>x^k'
         local update = function (args)
                           if args and args.verbose then print('updating: '..name) end
                           out1:resizeAs(in1):copy(in1):pow(in2[1])
                        end
         return {update=update,name=name,output=output,params={}}
      end
   stack['compute-node']['math']['abs-x-'] = 
      function (inputs,output)
         assert(inputs['x'], ERROR_MISSING_VAR .. 'x')
         assert(output, ERROR_MISSING_VAR .. 'output')
         local in1 = inputs['x']
         local out1 = output
         local name = 'math>abs-x-'
         local update = function (args)
                           if args and args.verbose then print('updating: '..name) end
                           out1:resizeAs(in1):copy(in1):abs()
                        end
         return {update=update,name=name,output=output,params={}}
      end
   stack['compute-node']['math']['floor-x-'] = 
      function (inputs,output)
         assert(inputs['x'], ERROR_MISSING_VAR .. 'x')
         assert(output, ERROR_MISSING_VAR .. 'output')
         local in1 = inputs['x']
         local out1 = output
         local name = 'math>floor-x-'
         local update = function (args)
                           if args and args.verbose then print('updating: '..name) end
                           out1:resizeAs(in1):copy(in1):floor()
                        end
         return {update=update,name=name,output=output,params={}}
      end
   stack['compute-node']['math']['ceil-x-'] = 
      function (inputs,output)
         assert(inputs['x'], ERROR_MISSING_VAR .. 'x')
         assert(output, ERROR_MISSING_VAR .. 'output')
         local in1 = inputs['x']
         local out1 = output
         local name = 'math>ceil-x-'
         local update = function (args)
                           if args and args.verbose then print('updating: '..name) end
                           out1:resizeAs(in1):copy(in1):ceil()
                        end
         return {update=update,name=name,output=output,params={}}
      end
   stack['compute-node']['math']['sqrt-x-'] = 
      function (inputs,output)
         assert(inputs['x'], ERROR_MISSING_VAR .. 'x')
         assert(output, ERROR_MISSING_VAR .. 'output')
         local in1 = inputs['x']
         local out1 = output
         local name = 'math>sqrt-x-'
         local update = function (args)
                           if args and args.verbose then print('updating: '..name) end
                           out1:resizeAs(in1):copy(in1):sqrt()
                        end
         return {update=update,name=name,output=output,params={}}
      end

   -- node: generate
   stack['compute-node']['generate'] = {}
   stack['compute-node']['generate']['zeros'] = 
      function (inputs,output)
         assert(inputs['size'], ERROR_MISSING_VAR .. 'size')
         assert(output, ERROR_MISSING_VAR .. 'output')
         local in1 = inputs['size']
         local out1 = output
         local name = 'generate>zeros'
         local update = function (args)
                           if args and args.verbose then print('updating: '..name) end
                           -- resize tensor
                           local dim = torch.LongStorage(in1:size(1))
                           for i=1,dim:size() do
                              dim[i] = in1[1]
                           end
                           out1:resize(dim):zero()
                        end
         return {update=update,name=name,output=output,params={}}
      end
   stack['compute-node']['generate']['ones'] = 
      function (inputs,output)
         assert(inputs['size'], ERROR_MISSING_VAR .. 'size')
         assert(output, ERROR_MISSING_VAR .. 'output')
         local in1 = inputs['size']
         local out1 = output
         local name = 'generate>ones'
         local update = function (args)
                           if args and args.verbose then print('updating: '..name) end
                           -- resize tensor
                           local dim = torch.LongStorage(in1:size(1))
                           for i=1,dim:size() do
                              dim[i] = in1[1]
                           end
                           out1:resize(dim):fill(1)
                        end
         return {update=update,name=name,output=output,params={}}
      end
   stack['compute-node']['generate']['fill'] = 
      function (inputs,output)
         assert(inputs['size'], ERROR_MISSING_VAR .. 'size')
         assert(inputs['a'], ERROR_MISSING_VAR .. 'a')
         assert(output, ERROR_MISSING_VAR .. 'output')
         local in1 = inputs['size']
         local in2 = inputs['a']
         local out1 = output
         local name = 'generate>fill-a-'
         local update = function (args)
                           if args and args.verbose then print('updating: '..name) end
                           -- resize tensor
                           local dim = torch.LongStorage(in1:size(1))
                           for i=1,dim:size() do
                              dim[i] = in1[1]
                           end
                           out1:resize(dim):fill(in2[1])
                        end
         return {update=update,name=name,output=output,params={}}
      end
   stack['compute-node']['generate']['range-a-to-b-'] = 
      function (inputs,output)
         assert(inputs['a'], ERROR_MISSING_VAR .. 'a')
         assert(inputs['b'], ERROR_MISSING_VAR .. 'b')
         assert(output, ERROR_MISSING_VAR .. 'output')
         local in1 = inputs['size']
         local in2 = inputs['a']
         local in3 = inputs['b']
         local a,b = -1,-1
         local out1 = output
         local name = 'generate>range-a-to-b-'
         local update = function (args)
                           if args and args.verbose then print('updating: '..name) end
                           if in2[1] ~= a or in3[1] ~= b then
                              -- get range
                              a = in2[1]
                              b = in3[2]
                              -- resize
                              if in1 then
                                 -- get dims
                                 local dim = torch.LongStorage(in1:size(1))
                                 for i=1,dim:size() do
                                    dim[i] = in1[1]
                                 end
                                 -- resize tensor
                                 out1:resize(dim)
                              else
                                 -- resize to a 1D array
                                 out1:resize(b-a+1)
                              end
                              -- issue warning if ill-sized tensor
                              if (b-a+1) ~= out1:storage():size() then
                                 error(ERROR_BAD_PARAMS..'range-a-to-b- > inconsistent sizes')
                              end
                              -- fill tensor
                              for i=a,b do
                                 out1:storage()[1] = i
                              end
                           end
                        end
         return {update=update,name=name,output=output,params={}}
      end

   -- node: reduce
   stack['compute-node']['reduce'] = {}
   stack['compute-node']['reduce']['p-norm-x-'] = 
      function (inputs,output)
         assert(inputs['p'], ERROR_MISSING_VAR .. 'p')
         assert(inputs['x'], ERROR_MISSING_VAR .. 'x')
         assert(output, ERROR_MISSING_VAR .. 'output')
         local in1 = inputs['x']
         local p = inputs['p']
         local out1 = output
         local name = 'reduce>p-norm-x-'
         local update = function (args)
                           if args and args.verbose then print('updating: '..name) end
                           out1:resize(1)
                           out1[1] = in1:norm(p[1])
                        end
         return {update=update,name=name,output=output,params={}}
      end
   stack['compute-node']['reduce']['p-dist-x-y-'] = 
      function (inputs,output)
         assert(inputs['p'], ERROR_MISSING_VAR .. 'p')
         assert(inputs['x'], ERROR_MISSING_VAR .. 'x')
         assert(inputs['y'], ERROR_MISSING_VAR .. 'y')
         assert(output, ERROR_MISSING_VAR .. 'output')
         local in1 = inputs['x']
         local in2 = inputs['y']
         local p = inputs['p']
         local out1 = output
         local name = 'reduce>p-dist-x-y-'
         local update = function (args)
                           if args and args.verbose then print('updating: '..name) end
                           out1:resize(1)
                           out1[1] = in1:dist(in2,p[1])
                        end
         return {update=update,name=name,output=output,params={}}
      end
   stack['compute-node']['reduce']['var-x-'] = 
      function (inputs,output)
         assert(inputs['x'], ERROR_MISSING_VAR .. 'x')
         assert(output, ERROR_MISSING_VAR .. 'output')
         local in1 = inputs['x']
         local out1 = output
         local name = 'reduce>var-x-'
         local update = function (args)
                           if args and args.verbose then print('updating: '..name) end
                           out1:resize(1)
                           out1[1] = in1:var()
                        end
         return {update=update,name=name,output=output,params={}}
      end
   stack['compute-node']['reduce']['std-x-'] = 
      function (inputs,output)
         assert(inputs['x'], ERROR_MISSING_VAR .. 'x')
         assert(output, ERROR_MISSING_VAR .. 'output')
         local in1 = inputs['x']
         local out1 = output
         local name = 'reduce>std-x-'
         local update = function (args)
                           if args and args.verbose then print('updating: '..name) end
                           out1:resize(1)
                           out1[1] = in1:std()
                        end
         return {update=update,name=name,output=output,params={}}
      end
   stack['compute-node']['reduce']['max-x-'] = 
      function (inputs,output)
         assert(inputs['x'], ERROR_MISSING_VAR .. 'x')
         assert(output, ERROR_MISSING_VAR .. 'output')
         local in1 = inputs['x']
         local out1 = output
         local name = 'reduce>min-x-'
         local update = function (args)
                           if args and args.verbose then print('updating: '..name) end
                           out1:resize(1)
                           out1[1] = in1:min()
                        end
         return {update=update,name=name,output=output,params={}}
      end
   stack['compute-node']['reduce']['max-x-'] = 
      function (inputs,output)
         assert(inputs['x'], ERROR_MISSING_VAR .. 'x')
         assert(output, ERROR_MISSING_VAR .. 'output')
         local in1 = inputs['x']
         local out1 = output
         local name = 'reduce>max-x-'
         local update = function (args)
                           if args and args.verbose then print('updating: '..name) end
                           out1:resize(1)
                           out1[1] = in1:max()
                        end
         return {update=update,name=name,output=output,params={}}
      end
   stack['compute-node']['reduce']['mean-x-'] = 
      function (inputs,output)
         assert(inputs['x'], ERROR_MISSING_VAR .. 'x')
         assert(output, ERROR_MISSING_VAR .. 'output')
         local in1 = inputs['x']
         local out1 = output
         local name = 'reduce>mean-x-'
         local update = function (args)
                           if args and args.verbose then print('updating: '..name) end
                           out1:resize(1)
                           out1[1] = in1:mean()
                        end
         return {update=update,name=name,output=output,params={}}
      end
   stack['compute-node']['reduce']['sum-x-'] = 
      function (inputs,output)
         assert(inputs['x'], ERROR_MISSING_VAR .. 'x')
         assert(output, ERROR_MISSING_VAR .. 'output')
         local in1 = inputs['x']
         local out1 = output
         local name = 'reduce>sum-x-'
         local update = function (args)
                           if args and args.verbose then print('updating: '..name) end
                           out1:resize(1)
                           out1[1] = in1:sum()
                        end
         return {update=update,name=name,output=output,params={}}
      end

   -- set loaded flag
   xFlow.libs.math.loaded = true
end
