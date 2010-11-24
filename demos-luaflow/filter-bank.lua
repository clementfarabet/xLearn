----------------------------------------------------------------------
-- this script demonstrates the idea of flow-graph programming,
-- using the luaFlow package.
----------------------------------------------------------------------

flow = require 'luaFlow'

-- create nodes
cam = flow.Node.ImageSource{output  = flow.Array(3,240,240),
                            type    = 'camera'}

yuv = flow.Node.ImageTransform{input   = cam.output,
                               type    = 'rgb2yuv'}

filter = flow.Node.LinearFilterBank2D{input   = yuv.output,
                                      table   = {{1,1}, {1,2}, {2,3}, {3,4}},
                                      weight  = flow.Array():setdata(lab.randn(7,7,4))}

tanh = flow.Node.Apply{input   = filter.output,
                       mapping = math.tanh}

disp_in = flow.Node.Display{input  = yuv.output,
                            zoom   = 1,
                            min    = 0, 
                            max    = 1}

disp_out = flow.Node.Display{input  = tanh.output,
                            zoom   = 1,
                            min    = -1, 
                            max    = 1}

-- connect nodes together
network = flow.Node.gather(disp_in, disp_out, cam, tanh, yuv, filter)

-- print
print(network)

-- update the network in a loop
while true do
   network:update()
end
