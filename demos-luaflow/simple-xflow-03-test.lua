----------------------------------------------------------------------
-- this script demonstrates the idea of flow-graph programming,
-- using the luaFlow package.
----------------------------------------------------------------------

flow = require 'luaFlow'

-- implement a node from an xFlow program (v0.3)
node = flow.Node.load{file = '../xFlow/examples/test-simple.xfl', 
                      verbose=true, version=0.3}

-- a dummy listener is used to force computations
-- (if nobody listens to the node's outputs, they won't be computed)
listener = flow.Node.Display{input  = node.outputs['out1'],
                             legend = 'output'}

-- insert some data (lena)
node.inputs.in1:setdata(image.lena(300,300))
while true do
   node:update()
   listener:update()
end
