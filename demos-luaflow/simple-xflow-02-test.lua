----------------------------------------------------------------------
-- this script demonstrates the idea of flow-graph programming,
-- using the luaFlow package.
----------------------------------------------------------------------

flow = require 'luaFlow'

-- create nodes
cam = flow.Node.ImageSource{output  = flow.Array(3,83,83),
                            type    = 'camera'}

convnet = flow.Node.load{input   = cam.output,
                         version = 0.2,
                         file    = 'networks/xflow-first-example.xfl'}

disp = flow.Node.Display{input  = convnet.output, zoom   = 2}

-- connect nodes together
network = flow.Node.gather(disp, cam, convnet)

-- print
print(network)

-- update the network in a loop
while true do
   network:update()
end
