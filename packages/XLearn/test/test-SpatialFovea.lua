require 'XLearn'

dofile 'test-jac.lua'

function test_SpatialFovea(channels, iwidth, iheight, fx, fy)
   -- init module
   module = nn.SpatialFovea{nInputPlane = channels,
                            ratios = {1,2},
                            preProcessors = {nn.Identity(),
                                             nn.Identity()},
                            processors = {nn.SpatialConvolution(channels,4,3,3),
                                          nn.SpatialConvolution(channels,4,3,3)},
                            fov = 3,
                            sub = 1 }
   module:reset()
   -- init input
   input = lab.rand(iwidth, iheight, channels)
   -- bprop
   if fx and fy then
      print("# Testing SpatialSpatialFovea with focus")
      module:focus(fx,fy,3)
      test_jac(module, input)
   else
      print("# Testing SpatialSpatialFovea")
      test_jac(module, input)
   end
end
 
-- run tests !
test_SpatialFovea(3, 16, 16)
test_SpatialFovea(3, 16, 16, 1, 1)
test_SpatialFovea(3, 16, 16, 16, 1)
test_SpatialFovea(3, 16, 16, 1, 16)
test_SpatialFovea(3, 16, 16, 8, 8)
