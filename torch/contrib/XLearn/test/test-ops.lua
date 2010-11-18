require 'XLearn'

dofile 'test-jac.lua'

function test_ops()
   test_cadd()
   test_cmul()
   test_csub()
   test_cdiv()
end

function test_lcn()
   test_power()
   test_square()
   test_sqrt()
   test_replicate()
   test_threshold()
   test_contrastnormalization()
end

function test_power()
   local in1 = lab.rand(10,20)
   local mod = nn.Power(2)
   local out = mod:forward(in1)
   print('*********************************')
   print('testing for ' .. torch.typename(mod))
   print(string.format(" - distance %8.6e\n",out:dist(in1:cmul(in1))))
   test_power_jac()
end

function test_square()
   local in1 = lab.rand(10,20)
   local mod = nn.Square()
   local out = mod:forward(in1)
   print('*********************************')
   print('testing for ' .. torch.typename(mod))
   print(string.format(" - distance %8.6e\n",out:dist(in1:cmul(in1))))
   test_square_jac()
end

function test_sqrt()
   local in1 = lab.rand(10,20)
   local mod = nn.Sqrt()
   local out = mod:forward(in1)
   print('*********************************')
   print('testing for ' .. torch.typename(mod))
   print(string.format(" - distance %8.6e\n",out:dist(in1:sqrt())))
   test_sqrt_jac()
end

function test_replicate()
   print('*********************************')
   test_replicate_jac()
end

function test_threshold()
   print('*********************************')
   test_threshold_jac()
end

function test_contrastnormalization()
   print('*********************************')
   test_contrastnormalization_jac()
end

function test_cadd()
   local in1 = lab.rand(10,20)
   local in2 = lab.rand(20,10)
   local mod = nn.CAddTable()
   local out = mod:forward({in1,in2})
   print('*********************************')
   print('testing for ' .. torch.typename(mod))
   print(string.format(" - distance %8.6e\n",out:dist(in1:add(in2))))
   test_cadd_jac()
end

function test_cmul()
   local in1 = lab.rand(10,20)
   local in2 = lab.rand(20,10)
   local mod = nn.CMulTable()
   local out = mod:forward({in1,in2})
   print('*********************************')
   print('testing for ' .. torch.typename(mod))
   print(string.format(" - distance %8.6e\n",out:dist(in1:cmul(in2))))
   test_cmul_jac()
end

function test_csub()
   local in1 = lab.rand(10,20)
   local in2 = lab.rand(20,10)
   local mod = nn.CSubTable()
   local out = mod:forward({in1,in2})
   print('*********************************')
   print('testing for ' .. torch.typename(mod))
   print(string.format(" - distance %8.6e\n",out:dist(in1:add(-in2))))
   test_csub_jac()
end

function test_cdiv()
   local in1 = lab.rand(10,20)
   local in2 = lab.rand(20,10)
   local mod = nn.CDivTable()
   local out = mod:forward({in1,in2})
   print('*********************************')
   print('testing for ' .. torch.typename(mod))
   print(string.format(" - distance %8.6e\n",out:dist(in1:cdiv(in2))))
   test_cdiv_jac()
end

function test_cadd_jac()
   local ini = math.random(5,10)
   local inj = math.random(5,10)
   local ink = math.random(5,10)
   local input = torch.Tensor(ini, inj, ink):zero()

   local module = nn.CAddTable()
   local mlp = nn.Sequential()
   mlp:add(nn.SplitTable(3))
   mlp:add(module)

   print('testing jacobian for ' .. torch.typename(module))
   test_jac(mlp, input)
   testwriting(mlp,input)
end

function test_cmul_jac()
   local ini = math.random(5,10)
   local inj = math.random(5,10)
   local ink = math.random(5,10)
   local input = torch.Tensor(ini, inj, ink):zero()

   local module = nn.CMulTable()
   local mlp = nn.Sequential()
   mlp:add(nn.SplitTable(3))
   mlp:add(module)
   
   print('testing jacobian for ' .. torch.typename(module))
   test_jac(mlp, input)
   testwriting(mlp,input)
end

function test_csub_jac()
   local ini = math.random(5,10)
   local inj = math.random(5,10)
   local ink = 2
   local input = torch.Tensor(ini, inj, ink):zero()

   local module = nn.CSubTable()
   local mlp = nn.Sequential()
   mlp:add(nn.SplitTable(3))
   mlp:add(module)

   print('testing jacobian for ' .. torch.typename(module))
   test_jac(mlp, input)
   testwriting(mlp,input)
end

function test_cdiv_jac()
   local ini = math.random(5,10)
   local inj = math.random(5,10)
   local ink = 2
   local input = torch.Tensor(ini, inj, ink):zero()

   local module = nn.CDivTable()
   local mlp = nn.Sequential()
   mlp:add(nn.SplitTable(3))
   mlp:add(module)

   print('testing jacobian for ' .. torch.typename(module))
   test_jac(mlp, input,1,3)
   testwriting(mlp,input)
end

function test_power_jac()
   local ini = math.random(5,10)
   local inj = math.random(5,10)
   local ink = math.random(5,10)
   local pw = random.uniform()*math.random(1,10)
   local input = torch.Tensor(ini, inj, ink):zero()

   local module = nn.Power(pw)

   print('testing jacobian for ' .. torch.typename(module))
   test_jac(module, input,0,3)
   testwriting(module,input)
end

function test_square_jac()
   local ini = math.random(5,10)
   local inj = math.random(5,10)
   local ink = math.random(5,10)
   local input = torch.Tensor(ini, inj, ink):zero()

   local module = nn.Square()

   print('testing jacobian for ' .. torch.typename(module))
   test_jac(module, input)
   testwriting(module,input)
end

function test_sqrt_jac()
   local ini = math.random(5,10)
   local inj = math.random(5,10)
   local ink = math.random(5,10)
   local input = torch.Tensor(ini, inj, ink):zero()

   local module = nn.Sqrt()

   print('testing jacobian for ' .. torch.typename(module))
   test_jac(module, input,0,3)
   testwriting(module,input,0,3)
end

function test_replicate_jac()
   local ini = math.random(5,10)
   local inj = math.random(5,10)
   local ink = math.random(5,10)
   local input = torch.Tensor(ini, inj):zero()

   local module = nn.Replicate(ink)

   print('testing jacobian for ' .. torch.typename(module))
   test_jac(module, input)
   testwriting(module,input)
end

function test_threshold_jac()
   local ini = math.random(5,10)
   local inj = math.random(5,10)
   local ink = math.random(5,10)
   local input = torch.Tensor(ini, inj, ink):zero()

   local module = nn.Threshold(random.uniform(-2,2),random.uniform(-2,2))

   print('testing jacobian for ' .. torch.typename(module))
   test_jac(module, input)
   testwriting(module,input)
end

function test_contrastnormalization_jac()
   local ini = math.random(5,10)
   local inj = math.random(5,10)
   local ink = math.random(5,10)
   local input = torch.Tensor(ini, inj, ink):zero()

   local ker = lab.rand(5,5)
   local module = nn.ContrastNormalization(ker,ink)

   print('!!! set the threshold in forward to a fixed value for jacobian testing !!!')
   print('testing jacobian for ' .. torch.typename(module))
   test_jac(module, input)
   testwriting(module,input)
end


test_ops()
