----------------------------------------------------------------------
-- this script demonstrates the idea of flow-graph programming,
-- using the ?flow libraries.
----------------------------------------------------------------------

require 'luaflow'

-- create nodes
filter = luaflow.Node.Convolution{input  = luaflow.Array(),
                                  table  = {{1,2,3,4}, {1,2,3,4}, {1,2,3,4}, {1,2,3,4}},
                                  weight = luaflow.Array(16,10,10):fill(0.7),
                                  verbose = true}

tanh = luaflow.Node.Apply{input = filter.output,
                          mapping = math.tanh,
                          verbose = true}

-- connect nodes together
network = luaflow.Node.gather(filter,tanh)

-- drive the input of the network, and update its content
network.input:resize(4,10,10):fill(0.001)
network:update()
