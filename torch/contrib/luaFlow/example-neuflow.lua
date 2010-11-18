----------------------------------------------------------------------
-- this script demonstrates the idea of flow-graph programming,
-- using the ?flow libraries.
----------------------------------------------------------------------

host = require 'luaflow'
neuflow = require 'neuflow'

-- init the device
neuflow.initialize()

-- create all nodes, and connect them
node_cam = host.Node.Camera{output=host.Array()}

node_conv = neuflow.Node.Convolution{input=node_cam.output,
                                     table  = {{1,2,3}, {1,2,3}, {1,2,3}, {1,2,3}},
                                     weight = luaflow.Array(12,10,10):fill(0.7)}

node_tanh = neuflow.Node.Apply{input=node_conv.output,
                               output=host.Array(),
                               mapping=mymap}

-- build a node out of the previously created nodes
top_node = luaflow.Node.gather(node_cam,node_conv,node_tanh)

-- trigger exec on neuFlow
neuflow.execute()

-- run the code
while true do
   node_cam.output:resize(3,10,10)
   top_node:update()
end
