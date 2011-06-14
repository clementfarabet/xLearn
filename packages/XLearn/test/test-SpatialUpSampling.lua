require 'XLearn'

dofile 'test-jac.lua'

function test_UpSampling(channels, iwidth, iheight, dw, dh)
   -- init module
   module = nn.SpatialUpSampling(channels, dw, dh)
   -- init input
   input = lab.rand(iwidth, iheight, channels)
   -- bprop
   print("# Testing SpatialUpSampling")
   test_jac(module, input)
end
 
-- run test !
test_UpSampling(3, 8, 8, 2, 2)
test_UpSampling(3, 4, 4, 3, 3)
test_UpSampling(3, 3, 7, 4, 4)
