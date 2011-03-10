--- !/usr/bin/env lua
--- -*- coding: utf-8 -*-
require 'XLearn'
-- load jacobian tester
dofile 'test-jac.lua'

function test_SpatialLinear(fanin, fanout, sizex, sizey)
   -- init module
   module = nn.SpatialLinear(fanin, fanout)
   -- init input
   input = lab.rand(sizex,sizey,fanin)
   -- bprop
   print("========================== Testing SpatialLinear input ==========================")
   test_jac(module, input)
   print("========================== Testing weight ==========================")
   test_jac_param(module, input, module.weight, module.gradWeight)
   print("========================== Testing bias ==========================")
   test_jac_param(module, input, module.bias, module.gradBias)
end
 
-- run test !
test_SpatialLinear(30, 12, 16, 8)
