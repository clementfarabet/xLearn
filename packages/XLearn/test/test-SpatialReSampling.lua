require 'XLearn'

dofile 'test-jac.lua'

function test_ReSampling(channels, iwidth, iheight, owidth, oheight)
   -- init module
   module = nn.SpatialReSampling(owidth, oheight)
   -- init input
   input = lab.rand(iwidth, iheight, channels)
   -- bprop
   print("# Testing SpatialReSampling")
   test_jac(module, input)
end
 
-- run test !
test_ReSampling(3, 1, 1, 32, 32)
test_ReSampling(3, 4, 6, 16, 22)
test_ReSampling(10, 8, 3, 10, 12)
