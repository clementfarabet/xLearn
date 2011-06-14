require 'XLearn'
require 'opencl'
require 'lunit'

module("CLSpatialConvolution", lunit.testcase, package.seeall)
local profiling = false
context = opencl.getContext{profiling=profiling}

function compareToTorch(inW, inH, inZ, outZ, kW, kH)
   local conv =  nn.SpatialConvolution(inZ, outZ, kW, kH ,1,1)
   local clconv = nn.CLSpatialConvolution{context = context, 
                                          nInputPlane = inZ, 
                                          nOutputPlane = outZ, 
                                          kW = kW, 
                                          kH = kH,
                                          dW = 1,
                                          dH = 1}
   local input = lab.randn(inW, inH, inZ)
   local clin = opencl.Tensor(clconv.context,inW, inH, inZ)
   clin:copy(input)
   conv.bias:fill(0)
   clconv.weight:copy(conv.weight)
   local myt = os.realtime()
   local out = conv:forward(input)
   if profiling then print(' *** CPU TIME \t',os.realtime()-myt) end
   local clout = clconv:forward(clin)
--    print('out cpu',out)
--    print('out gpu',clout)
   --print('in', input)
   --print('w', conv.weight)
   local diff = torch.Tensor():resizeAs(out):copy(clout:get())
   local distance = out:dist(diff,2)
   got = torch.Tensor():resizeAs(out):copy(clout:get())
   local max_diff = diff:add(-1,out):abs():max()
   assert_almost_equal(0, max_diff, 5, "L1 norm of the diff")
   --io.write(string.format(" - max 1l1 diff %8.6e\n", max_diff))
   --io.write(string.format(" - distance %8.6e\n",distance))
end

function make_compareToTorch_cases()
   insizes = {15,25}
   kersizes = {4,5}
   inzs = {1,3}
   outzs = {1,8}
   for _,inz in ipairs(inzs) do
      for _,outz in ipairs(outzs) do
         for _,inh in ipairs(insizes) do
            for _,inw in ipairs(insizes) do
               for _,kw in ipairs(kersizes) do
                  for _,kh in ipairs(kersizes) do
                     _M['test_inw'.. inw
                        .. '_inh' .. inh
                        .. '_inz' .. inz
                        .. '_outz' .. outz
                        .. '_kw' .. kw
                        .. '_kh' .. kh
                     ] = 
                        function ()
                           compareToTorch(inw,inh,inz,outz, kw, kh)
                        end
                  end
               end
            end
         end
      end
   end
end


local my=false
if my then
   _M['mytest'] = function ()
                     --compareToTorch(10,10,2,1,5,5)
                     compareToTorch(500,300,3,16,9,9)
                  end
else
   make_compareToTorch_cases()
end
-- run all tests
lunit.main()
--TODO assert alost equal tensor