

local AbsModuleHessian, parent = torch.class('nn.AbsModuleHessian', 'nn.AbsModule')

function AbsModuleHessian:__init()
   parent.__init(self)
   self.hessianInput = torch.Tensor()
end

function AbsModuleHessian:write(file)
   parent.write(self, file)
   file:writeObject(self.hessianInput)
end

function AbsModuleHessian:read(file)
   parent.read(self, file)
   self.hessianInput = file:readObject()
end
