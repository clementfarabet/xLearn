require 'nn'
require 'random'

function get_jac_bprop (module, input, param, dparam)
   local doparam = 0
   if param then
      doparam = 1
   end
   param = param or input
   -- output deriv
   local dout = torch.Tensor():resizeAs(module:forward(input))
   -- 1D view
   local sdout = torch.Tensor(dout:storage(),1,dout:nElement())
   -- jacobian matrix to calculate
   local jacobian = torch.Tensor(param:nElement(),dout:nElement()):zero()

   for i=1,sdout:nElement() do
      --print('bprop')
      toolBox.dispProgress(i, sdout:nElement())
      dout:zero()
      sdout[i] = 1
      module:zeroGradParameters()
      local din = module:backward(input, dout)
      if doparam == 1 then
	 --print(dparam)
	 jacobian:select(2,i):copy(dparam)
      else
	 jacobian:select(2,i):copy(din)
      end
   end
   return jacobian
end

function get_jac_fprop(module, input, param)
   param = param or input
   -- perturbation amount
   local small = 1/8--1e-6
   -- 1D view of input
   local tst = param:storage()
   local sin = torch.Tensor(tst,1,tst:size())
   -- jacobian matrix to calculate
   local jacobian = torch.Tensor():resize(param:nElement(),module:forward(input):nElement())
   
   local outa = torch.Tensor(jacobian:size(2))
   local outb = torch.Tensor(jacobian:size(2))
   
   for i=1,sin:nElement() do
      --print('fprop')
      toolBox.dispProgress(i, sin:nElement())
      

      sin[i] = sin[i] - small

      
      -- scale to fixed point
   --input:mul(256)
   --input:add(0.5)
   --input:floor()
   --input:div(256)
      outa:copy(module:forward(input))
      sin[i] = sin[i] + 2*small


      -- scale to fixed point
   --input:mul(256)
   --input:add(0.5)
   --input:floor()
   --input:div(256)
      outb:copy(module:forward(input))
      sin[i] = sin[i] - small

      outb:add(-1,outa):div(2*small)
      jacobian:select(1,i):copy(outb)
   end
   return jacobian
end

function test_jac (module, input, minval, maxval)
   minval = minval or -2
   maxval = maxval or 2
   local inrange = maxval - minval
   input:copy(lab.rand(input:nElement()):mul(inrange):add(minval))
   --input:fill(10)
   -- scale to fixed point
   --input:mul(256)
   --input:add(0.5)
   --input:floor()
   --input:div(256)

   local jac_fprop = get_jac_fprop(module,input)
   local jac_bprop = get_jac_bprop(module,input)
   print('dif map:')
   print(jac_fprop - jac_bprop)
   print('dif map mean:')
   print((jac_fprop - jac_bprop):abs():mean())
   io.write(string.format(" - distance %8.6e\n",jac_fprop:dist(jac_bprop,2)))
   
end

function test_jac_param (module, input, param, dparam, minval, maxval)
   minval = minval or -2
   maxval = maxval or 2
   local inrange = maxval - minval
   input:copy(lab.rand(input:nElement()):mul(inrange):add(minval))
   param:copy(lab.rand(param:nElement()):mul(inrange):add(minval))
   --print(input:sum())
   jac_bprop = get_jac_bprop(module, input, param, dparam)
   jac_fprop = get_jac_fprop(module, input, param)
   --print('jac_bprop',jac_bprop)
   --print('jac_fprop',jac_fprop)
   io.write(string.format(" - distance %8.6e\n",jac_fprop:dist(jac_bprop,2)))
end

function test_conv(from, to, ini, inj, ki, kj , si, sj)
   local module = nn.SpatialConvolution(from , to, ki, kj, si, sj)
   local input = torch.Tensor(ini, inj, from):zero()
   module:reset()
   
   io.write("input")
   test_jac(module, input)
   io.write("weight")
   test_jac_param(module, input, module.weight, module.gradWeight)
   io.write("bias")
   test_jac_param(module, input, module.bias, module.gradBias)
end

function test_abs_module()
   require 'atn'
   local module = nn.AbsModule()
   local ini = math.floor(math.random(5,10))
   local inj = math.floor(math.random(5,10))
   local from = math.floor(math.random(5,10))
   local input = torch.Tensor(ini, inj, from):zero()
   
   io.write("input")
   test_jac(module, input)
end


function testwriting(module,input, minval, maxval)
   minval = minval or -2
   maxval = maxval or 2
   local inrange = maxval - minval

   -- run module
   module:forward(input)
   local go = torch.Tensor():resizeAs(module.output):copy(lab.rand(module.output:nElement()):mul(inrange):add(minval))
   module:backward(input,go)

   local fo = torch.Tensor():resizeAs(module.output):copy(module.output)
   local bo = torch.Tensor():resizeAs(module.gradInput):copy(module.gradInput)

   -- write module
   local f = torch.DiskFile('tmp.bin','w'):binary()
   f:writeObject(module)
   f:close()
   -- read module
   local m = torch.DiskFile('tmp.bin'):binary():readObject()
   m:forward(input)
   m:backward(input,go)

   local fo2 = torch.Tensor():resizeAs(m.output):copy(m.output)
   local bo2 = torch.Tensor():resizeAs(m.gradInput):copy(m.gradInput)
   print('before after writing forward distance ' .. fo:dist(fo2))
   print('before after writing backward distance ' .. bo:dist(bo2))
end
