----------------------------------------------------------------------
-- this script demonstrates the idea of flow-graph programming,
-- using the luaFlow package.
----------------------------------------------------------------------

flow = require 'luaFlow'

-- create nodes
cam = flow.Node.ImageSource{output  = flow.Array(3,83,83),
                            type    = 'camera'}

yuv = flow.Node.ImageTransform{input   = cam.output,
                               type    = 'rgb2y'}

convnet = flow.Node.load{input   = yuv.output,
                         file    = 'networks/iccv-convnet.xfl'}

disp_in = flow.Node.Display{input  = yuv.output,
                            zoom   = 1,
                            min    = 0, 
                            max    = 1}

-- connect nodes together
network = flow.Node.gather(disp_in, cam, yuv, convnet)

-- print
print(network)

-- update the network in a loop
while true do
   network:update()
end
