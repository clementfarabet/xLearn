--- !/usr/bin/env lua
--- -*- coding: utf-8 -*-
require 'XLearn'
-- load jacobian tester
dofile('test-jac.lua')

function test_LcEncoder(inX, inY, winX, winY, woutX, woutY, 
                           overcompleteX, overcompleteY)
   -- init module
   module = nn.LcEncoder(winX, winY, woutX, woutY, overcompleteX, overcompleteY)
   -- init input
   input = lab.rand(inX,inY,1)
   -- bprop
   print("========================== Testing Encoder input ==========================")
   test_jac(module, input)
   print("========================== Testing weight ==========================")
   test_jac_param(module, input, module.weight, module.gradWeight)
   print("========================== Testing bias ==========================")
   test_jac_param(module, input, module.bias, module.gradBias)
end
 
-- encoder params
local winX = 10
local winY = 10
local woutX = 5
local woutY = 5
local overcompleteX = 2
local overcompleteY = 2
local inX = 44
local inY = 44
test_LcEncoder(inX, inY, winX, winY, woutX, woutY, 
               overcompleteX, overcompleteY)
