----------------------------------------------------------------------
-- this script demonstrates the idea of flow-graph programming,
-- using the luaFlow package.
----------------------------------------------------------------------

flow = require 'luaFlow'

-- parse args
op = OptionParser('%prog [options]')
op:add_option{'-s', '--source', action='store', dest='source', 
              help='image source, can be one of: camera | lena'}
op:add_option{'-t', '--target', action='store', dest='target', 
              help='target to run the code on: neuflow | cpu | cuda (default=cpu)'}
ops,args = op:parse_args()
ops.source = ops.source or 'camera'
ops.target = ops.target or 'cpu'

-- create nodes
cam = flow.Node.ImageSource{output  = flow.Array(3,240,240),
                            type    = ops.source}

yuv = flow.Node.ImageTransform{input   = cam.output,
                               type    = 'rgb2yuv'}

filter = flow.Node.LinearFilterBank{input   = yuv.output,
                                    table   = {{1,1}, {1,2}, {2,3}, {3,4}},
                                    weight  = flow.Array():setdata(lab.randn(7,7,4))}

tanh = flow.Node.Apply{input   = filter.output,
                       mapping = math.tanh}

mathnode = flow.Node.Math{x = tanh.output}

disp_in = flow.Node.Display{input  = yuv.output,
                            legend = 'inputs',
                            zoom   = 1,
                            min    = 0, 
                            max    = 1}

disp_out = flow.Node.Display{input  = mathnode.abs_x_,
                             legend = 'outputs',
                             zoom   = 1,
                             min    = -1, 
                             max    = 1}

-- connect nodes together
network = flow.Node.gather(disp_in, disp_out, cam, tanh, yuv, filter, mathnode)

-- print
print(network)

-- update the network in a loop
while true do
   network:update()
   if ops.source == 'lena' then break end
end
