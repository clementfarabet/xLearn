--- !/usr/bin/env lua
--- -*- coding: utf-8 -*-
require 'XLearn'
-- load jacobian tester
dofile('test-jac.lua')


function test_LocalNorm()
   -- params
   local inputSize = math.random(11,20)
   local kersize = math.random(5,10)
   local nbfeatures = math.random(5,10)
   -- init kernel
   kernel = image.gaussian{width=kersize}
   -- init module
   module = nn.LocalNorm(kernel,nbfeatures,0.1)
   -- init input
   input = lab.rand(inputSize,inputSize,nbfeatures)
   -- bprop
   print("========================== Testing LocalNorm input ==========================")
   test_jac(module, input)
   print("========================== Testing LocalNorm read/write ==========================")
   testwriting(module,input)
end
 
-- LocalNorm param
test_LocalNorm()
