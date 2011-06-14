--- !/usr/bin/env lua

require 'XLearn'
dofile 'test-jac.lua'

require 'lunit'

module("TestLocalNorm", lunit.testcase, package.seeall)

-- params
local inputSize = math.random(11,20)
local kersize = math.random(5,10)
local nbfeatures = math.random(5,10)

function test_Gaussian2D()
   -- init kernel
   kernel = image.gaussian{width=kersize}
   -- init module
   module = nn.LocalNorm(kernel,nbfeatures,0.1)
   -- init input
   input = lab.rand(inputSize,inputSize,nbfeatures)
   -- bprop
   local error = test_jac(module, input)
   assert_almost_equal(0, error, 5, "2D Gaussian -- L2 distance")
end

function test_Gaussian1D()
   -- init kernel
   kernelh = image.gaussian1D(11):resize(11,1)
   kernelv = kernelh:t()
   -- init module
   module = nn.LocalNorm{kernels={kernelv,kernelh},nInputPlane=nbfeatures,threshold=0.1}
   -- init input
   input = lab.rand(inputSize,inputSize,nbfeatures)
   -- bprop
   local error = test_jac(module, input)
   assert_almost_equal(0, error, 5, "1D Gaussians -- L2 distance")
end

function test_io()
   module = nn.LocalNorm(kernel,nbfeatures,0.1)
   local error_f, error_b = testwriting(module,input)
   assert_equal(0, error_f, "IO -- L2 distance (forward)")
   assert_equal(0, error_b, "IO -- L2 distance (backward)")
end

-- LocalNorm param
lunit.main()
