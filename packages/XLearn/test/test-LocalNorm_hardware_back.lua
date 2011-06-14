--- !/usr/bin/env lua
--- -*- coding: utf-8 -*-
require 'XLearn'
-- load jacobian tester
dofile('test-jac_fixed.lua')


function test_LocalNorm()
   -- params
   local inputSize = 10-- math.random(11,20)
   local kersize = 3--math.random(5,10)
   local nbfeatures = 3--math.random(5,10)
   -- init kernel
   kernel = image.gaussian{width=kersize}
   -- init module
   --module = nn.LocalNorm_hardware_back(kernel,nbfeatures,2/256,true)
   module = nn.LocalNorm_hardware(kernel,nbfeatures,2/256,true)
   -- init input
   input = lab.rand(inputSize,inputSize,nbfeatures)
   
  -- local lena = torch.Tensor(image.lena():size(1),image.lena():size(2),1):copy(image.lena():select(3,2))
   --image.scale(lena, input, 'bilinear')
   --image.scale(image.lena(), input, 'bilinear')
   
   -- bprop
   print("========================== Testing LocalNorm input ==========================")
   test_jac(module, input)
   print("========================== Testing LocalNorm read/write ==========================")
   testwriting(module,input)
end
 
-- LocalNorm param
test_LocalNorm()
