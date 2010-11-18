
-- torch
require 'torch'

-- macro
p = function (list) for i,v in ipairs(list) do print(v) end end

-- array size
test_size = 1000000
loop_size = 10

-- simple test, to eval performance
aa = torch.Tensor(test_size):fill(1)
bb = torch.Tensor(test_size):fill(1)
for i = 1,test_size do bb[i] = i end
cc = torch.Tensor(test_size):zero()
temp = torch.Tensor(test_size):zero()

print('# Test 1: mul+add in Torch')
t = os.clock()
for i = 1,loop_size do
   temp:zero():add(aa):cmul(bb)
   cc:div(test_size):add(temp)
end
print('  time = ' .. os.clock() - t)

print('# Test 2: mul+add in pure Lua')
a = {} ; for i=1,test_size do a[i] = 1 end
b = {} ; for i=1,test_size do b[i] = i end
c = {} ; for i=1,test_size do c[i] = 0 end

t = os.clock()
for i = 1,loop_size do
   for j = 1,#a do
      c[j] = a[j] * b[j] + c[j]/test_size
   end
end
print('  time = ' .. os.clock() - t)
