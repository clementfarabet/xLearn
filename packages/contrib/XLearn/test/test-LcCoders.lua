--- !/usr/bin/env lua
--- -*- coding: utf-8 -*-
require 'XLearn'

function void(...)
end
debugprint = print
debugprint = void

function test_LcEncoder(inX, inY, winX, winY, woutX, woutY, 
                           overcompleteX, overcompleteY)
   -- init fprop
   encoder = nn.LcEncoder(winX, winY, woutX, woutY, overcompleteX, overcompleteY)
   net = nn.LocalConnected(winX, winY, woutX, woutY, overcompleteX, overcompleteY)
   net.weight:copy(encoder.weight)
   input = lab.rand(inX,inY)
   -- fprop
   print("========================== Testing Encoder fprop ==========================")
   fpropEnc = encoder:forward(input)
   fpropLc = net:forward(input)
   dist = fpropEnc:dist(fpropLc,2)
   io.write(string.format(" - distance %8.6e\n",dist))
   if dist > 10-5 then
      error('Test Encoder fprop Failed')
   end

   -- init bprop
   input = lab.rand(inX,inY)
   gradOutput = lab.rand(fpropLc:size())
   -- bprop
   print("========================== Testing Encoder bprop ==========================")   
   bpropEnc = encoder:backward(input,gradOutput)
   bpropLc = net:backward(input,gradOutput)
   dist = bpropEnc:dist(bpropLc,2)
   io.write(string.format(" - distance %8.6e\n",dist))
   if dist > 10-5 then
      error('Test Encoder fprop Failed')
   end
end

function test_LcDecoder(inX, inY, winX, winY, woutX, woutY, 
                           overcompleteX, overcompleteY)
   -- init fprop
   decoder = nn.LcDecoder(winX, winY, woutX, woutY, overcompleteX, overcompleteY)
   net = nn.LocalConnected(winX, winY, woutX, woutY, overcompleteX, overcompleteY)
   net.weight:copy(decoder.weight)
   input = lab.rand(inX,inY,1)
   -- fprop
   print("========================== Testing Decoder fprop ==========================")
   debugprint('input size:',input:size())
   fpropDec = decoder:forward(input)
   debugprint('dec output(fprop) size:',fpropDec:size())
   dummy = torch.Tensor(fpropDec:size())
   bpropLc = net:backward(dummy,input)
   debugprint('lc output(bprop) size:',bpropLc:size())
   dist = fpropDec:dist(bpropLc,2)
   io.write(string.format(" - distance %8.6e\n",dist))
   if dist > 10-5 then
      error('Test Encoder fprop Failed')
   end

   -- init bprop
   input = lab.rand(inX,inY,1)
   gradOutput = lab.rand(fpropDec:size())
   -- bprop
   print("========================== Testing Decoder bprop ==========================")
   bpropDec = decoder:backward(input,gradOutput)
   fpropLc = net:forward(gradOutput)
   dist = fpropLc:dist(bpropDec,2)
   io.write(string.format(" - distance %8.6e\n",dist))
   if dist > 10-5 then
      error('Test Encoder fprop Failed')
   end
end
 
-- autoencoder params
-- local winX = 10
-- local winY = 10
-- local woutX = 10
-- local woutY = 10
-- local overcompleteX = 2 
-- local overcompleteY = 2
-- local inX = 39
-- local inY = 39
-- test_LcEncoder(inX, inY, winX, winY, woutX, woutY, 
--                overcompleteX, overcompleteY)
-- test_LcDecoder(inX, inY, winX, winY, woutX, woutY, 
--                overcompleteX, overcompleteY)


------------------------
-- exhaustive testing --
------------------------
winXs = {1,2,5,10}
winYs = {1,2,5,10}
woutXs = {1,2,5,10}
woutYs = {1,2,5,10}
overcompletes = {.5,1,2}--{.5,1,4} -- to do compute the overcomplete in a better way 4 will crash sometime
local inX = 50
local inY = 50
local tests = 0
for _,winX in pairs(winXs) do
   for _,winY in pairs(winYs) do
      for _,woutX in pairs(woutXs) do
         for _,woutY in pairs(woutYs) do
            for _,overcompleteX in pairs(overcompletes) do
               if (winX >= overcompleteX) then
                  for _,overcompleteY in pairs(overcompletes) do
                     if ( winY >= overcompleteY) then
                        print('winX:',winX,'winY:',winY,'woutX:',woutX,'woutY:',woutY,
                              'overcompleteX',overcompleteX, 'overcompleteY',overcompleteY)
                        test_LcEncoder(inX, inY, winX, winY, woutX, woutY, 
                                       overcompleteX, overcompleteY)
                        test_LcDecoder(inX, inY, winX, winY, woutX, woutY, 
                                       overcompleteX, overcompleteY)
                        tests = tests+1
                     end
                  end
               end
            end
         end
      end
   end
end


print("==================================================================================")
print("==================================================================================")
str = string.format("********************        %d tests done, no error        **********************",tests)
print(str)
print("==================================================================================")
print("==================================================================================")
