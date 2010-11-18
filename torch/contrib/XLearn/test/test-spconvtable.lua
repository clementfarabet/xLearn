require 'nn'
require 'atn'

dofile('./test/test-jac.lua')

-- test the forward and backward function with non-table version
-- and check the jacobian

-- create input state sizes
local inf = math.random(10,20)
local outf = math.random(30,40)
local oi = math.random(10,15)
local oj = math.random(10,15)
local ki = math.random(5,10)
local kj = math.random(5,10)
local di = math.random(1,3)
local dj = math.random(1,3)
local ini = (oi-1)*di+ki
local inj = (oj-1)*dj+kj

-- create classes
random.manualSeed(123)
local oldc = nn.SpatialConvolution(inf,outf,ki,kj,di,dj)
local ct = nn.SpatialConvolutionTable:FullTable(inf,outf)
random.manualSeed(123)
local newc = nn.SpatialConvolutionTable(ct,ki,kj,di,dj)

-- make weights same
--oldc.bias:copy(newc.bias)
--oldc.weight:copy(newc.weight)

-- input between -1,1
local input = lab.rand(ini,inj,inf)*2-1

-- forward check
local oldo = oldc:forward(input)
local newo = newc:forward(input)

-- backward check
local gradOut = lab.rand(oldo:size(1),oldo:size(2),oldo:size(3))*2-1
oldgi = oldc:backward(input,gradOut)
newgi = newc:backward(input,gradOut)

print(string.format('from %d, to %d feature maps\nin = %d %d\nke = %d %d\nst = %d %d',inf,outf,ini,inj,ki,kj,di,dj))
print('forward diff  = ',oldo:dist(newo))
print('backward diff = ',oldgi:dist(newgi))

function test_spconv(from, to, ini, inj, ki, kj , si, sj)
   local ct = nn.SpatialConvolutionTable:FullTable(from,to)
   local module = nn.SpatialConvolutionTable(ct, ki, kj, si, sj)
   local input = torch.Tensor(ini, inj, from):zero()
   module:reset()
   
   print('jacobian testing with fully connected table')
   io.write("input")
   test_jac(module, input)
   io.write("weight")
   test_jac_param(module, input, module.weight, module.gradWeight)
   io.write("bias")
   test_jac_param(module, input, module.bias, module.gradBias)

   testwriting(module,input)
end
function test_spconv1(from, ini, inj, ki, kj , si, sj)
   local ct = nn.SpatialConvolutionTable:OneToOneTable(from)
   local module = nn.SpatialConvolutionTable(ct, ki, kj, si, sj)
   local input = torch.Tensor(ini, inj, from):zero()
   module:reset()
   
   print('jacobian testing with 1-to-1 table')
   io.write("input")
   test_jac(module, input)
   io.write("weight")
   test_jac_param(module, input, module.weight, module.gradWeight)
   io.write("bias")
   test_jac_param(module, input, module.bias, module.gradBias)

   testwriting(module,input)
end
function test_spconv2(from, to, ini, inj, ki, kj , si, sj)
   local ct = nn.SpatialConvolutionTable:RandomTable(from,to,from-1)
   local module = nn.SpatialConvolutionTable(ct, ki, kj, si, sj)
   local input = torch.Tensor(ini, inj, from):zero()
   module:reset()
   
   print('jacobian testing with random table')
   io.write("input")
   test_jac(module, input)
   io.write("weight")
   test_jac_param(module, input, module.weight, module.gradWeight)
   io.write("bias")
   test_jac_param(module, input, module.bias, module.gradBias)

   testwriting(module,input)
end

test_spconv(3,5,ini,inj,ki,kj,di,dj)
test_spconv1(5,ini,inj,ki,kj,di,dj)
test_spconv2(5,7,ini,inj,ki,kj,di,dj)
