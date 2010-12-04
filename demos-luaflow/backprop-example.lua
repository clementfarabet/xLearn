----------------------------------------------------------------------
-- this script demonstrates the idea of flow-graph programming,
-- using the luaFlow package.
----------------------------------------------------------------------

flow = require 'luaFlow'

-- create nodes
filter = flow.Node.LinearFilterBank{input   = flow.Array(3,30,30),
                                    table   = {{1,1}, {1,2}, {2,3}, {3,4}},
                                    weight  = flow.Array():setdata(lab.randn(7,7,4))}

-- obtain derivate network:
energyError = flow.Array()
dfilter = flow.Node.getDerivative{inputs = {energyError},
                                  node  = filter}

-- print
print(filter.name)

-- feed the input of the net
filter.input:fill(0.4)

-- update the network (forward pass):
filter:update()

-- dummy energy error (for the sake of the example)
energyError:resize(filter.output.dims):fill(0)

-- compute all derivatives (backward pass):
dfilter:update()
