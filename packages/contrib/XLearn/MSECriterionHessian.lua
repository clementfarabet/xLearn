
local MSECriterionHessian, parent = torch.class('nn.MSECriterionHessian', 
                                                'nn.MSECriterion')

function MSECriterionHessian:__init()
   parent.__init(self)
   self.hessianInput = torch.Tensor()
end

function MSECriterionHessian:backwardHessian(input, target)
   self.hessianInput:resizeAs(input):fill(1)
   return self.hessianInput
end