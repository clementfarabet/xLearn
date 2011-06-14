require 'XLearn'

dofile 'test-jac.lua'

require 'lunit'
module("TestLocalGraph", lunit.testcase, package.seeall)

-- template
function template_LocalGraph(channels, iwidth, iheight, dist, norm)
   -- init module
   local module = nn.LocalGraph{normalize=norm, dist=dist}
   -- init input
   local input = lab.rand(iwidth, iheight, channels)
   -- bprop
   local error = test_jac(module, input, 0.1, 1)
   -- assert
   assert_almost_equal(0, error, 7, "L2 distance for " .. channels .. " input features, dist = " .. dist)
   assert_equal(false, toolBox.isNaN(error), "L2 distance for " .. channels .. " input features: NaN, dist = " .. dist)
end
 
-- define tests
function test_LocalGraph_1() template_LocalGraph(3, 16, 16, 'euclid', true) end
function test_LocalGraph_2() template_LocalGraph(16, 4, 4, 'euclid', true) end
function test_LocalGraph_3() template_LocalGraph(256, 2, 2, 'euclid', false) end
function test_LocalGraph_4() template_LocalGraph(2, 16, 16, 'cosine', false) end
function test_LocalGraph_5() template_LocalGraph(64, 3, 3, 'cosine', false) end

-- run tests
lunit.main()