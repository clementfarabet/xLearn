--- !/usr/bin/env lua
--- -*- coding: utf-8 -*-
require 'XLearn'

-- load jacobian tester
dofile('test-jac.lua')

-- test the forward and backward function of the class LocalConnected
-- and check the jacobian

--------------------------------------------
-- create input state sizes
--------------------------------------------
--random.manualSeed(1323)
-- fanin size
local winX = math.random(5,15)
local winY = math.random(5,15)
-- fanout size
local woutX = math.random(5,15)
local woutY = math.random(5,15)
-- input reuse for different filters
local overcompleteX = math.random(1,3) 
local overcompleteY = math.random(1,3)
-- input size
local inX = woutX*overcompleteX + winX -1 --math.random(20,40)
local inY = woutY*overcompleteY + winY -1 --math.random(20,40)


function test_localconnect(inX, inY, winX, winY, woutX, woutY, 
                           overcompleteX, overcompleteY)

   print(string.format('in = %d %d\nfanin = %d %d\nfanout = %d %d',
                       inX,inY,winX,winY,woutX,woutY))
   -- create classes
   local module = nn.LocalConnected(winX, winY, woutX, woutY, 
                                    overcompleteX, overcompleteY)
   -- input
   local input = torch.Tensor(inX,inY,1):zero()
   module:forget()
   

   print('jacobian testing local connected module')
   print("========================== Testing input ==========================")
   test_jac(module, input)
   io.write("========================== Testing weight =========================\n")
   test_jac_param(module, input, module.weight, module.gradWeight)
   --testwriting(module,input)
end

winX = 7
winY = 7
woutX = 5
woutY = 5
overcompleteX = 2
overcompleteY = 2
inX = 20
inY = 20

-- default
-- winX = 5
-- winY = 5
-- woutX = 5
-- woutY = 5
-- overcompleteX = 1
-- overcompleteY = 1
-- inX = 9
-- inY = 9

test_localconnect(inX, inY, winX, winY, woutX, woutY, 
                  overcompleteX, overcompleteY)