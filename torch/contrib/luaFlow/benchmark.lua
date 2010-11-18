
require 'luaflow'
require 'torch'
require 'nn'

-- dot prod
print('test 1: CMUL')
a = luaflow.Array(100,100):fill(0.4)
b = luaflow.Array(100,100):fill(1.00001)
loops = 100000

time = os.clock()
for i = 1,loops do
   a:mul(b)
end

print('+ luaflow = ' .. (os.clock() - time) .. 's for ' .. loops*(a.size/1e9) .. 'GOPs')

a = torch.Tensor(100,100):fill(0.4)
b = torch.Tensor(100,100):fill(1.00001)
loops = 100000

time = os.clock()
for i = 1,loops do
   a:cmul(b)
end

print('+ torch = ' .. (os.clock() - time) .. 's for ' .. loops*(a:storage():size()/1e9) .. 'GOPs')

-- conv
print('test 2: CONV2D')
a = luaflow.Array(105,105):fill(0.7)
ker = luaflow.Array(7,7):fill(0.1)
out = luaflow.Array()
loops = 2000

time = os.clock()
for i = 1,loops do
   out:conv2D(a,ker)
end

print('+ luaflow = ' .. (os.clock() - time) .. 's for ' .. loops*(a.size*ker.size/1e9) .. 'GOPs')

-- conv torch
a = torch.Tensor(105,105,1):fill(0.7)
loops = 2000
conv = nn.SpatialConvolution(1,1,7,7)
conv.weight:fill(0.1)

time = os.clock()
for i = 1,loops do
   conv:forward(a)
end

print('+ torch = ' .. (os.clock() - time) .. 's for ' .. 
   loops*(a:storage():size()*conv.weight:storage():size()/1e9) .. 'GOPs')
