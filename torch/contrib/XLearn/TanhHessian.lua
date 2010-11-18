
local TanhHessian, parent = torch.class('nn.TanhHessian', 'nn.Tanh')

function TanhHessian:__init()
   parent.__init(self)
   self.hessianInput = torch.Tensor()
end

function TanhHessian:write(file)
   parent.write(self, file)
   file:writeObject(self.hessianInput)
end

function TanhHessian:read(file)
   parent.read(self, file)
   self.hessianInput = file:readObject()
end
