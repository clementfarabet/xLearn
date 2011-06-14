local SequentialHessian, parent = torch.class('nn.SequentialHessian', 'nn.Sequential')

function SequentialHessian:__init()
   parent.__init(self)
end

function SequentialHessian:zeroLearningRates()
   for i=1,#self.modules do
      if self.modules[i].zeroLearningRates then
         self.modules[i]:zeroLearningRates()
      end
   end
end

function SequentialHessian:oneLearningRates()
   for i=1,#self.modules do
      if self.modules[i].oneLearningRates then
         self.modules[i]:oneLearningRates()
      end
   end
end

function SequentialHessian:boundLearningRates(mu)
   for i=1,#self.modules do
      if self.modules[i].boundLearningRates then
         self.modules[i]:boundLearningRates(mu)
      end
   end
end

function SequentialHessian:updateLearningRates(factor)
   for i=1,#self.modules do
      if self.modules[i].updateLearningRates then
         self.modules[i]:updateLearningRates(factor)
      end
   end
end

function SequentialHessian:getMinMaxSecondDerivatives()
   local min = 1/0
   local max = -1/0
   for i=#self.modules,1,-1 do
      -- check if we have nested sequentials
      if self.modules[i].getMinMaxSecondDerivatives then
	 local cur_min, cur_max = 
	    self.modules[i]:getMinMaxSecondDerivatives() 
	 min = math.min(min,cur_min)
	 max = math.max(max,cur_max)
      else
	 if self.modules[i].weightLearningRate then
	    local cur_min = self.modules[i].weightLearningRate:min()
	    local cur_max = self.modules[i].weightLearningRate:max()
	    min = math.min(min,cur_min)
	    max = math.max(max,cur_max)
	 end
	 if self.modules[i].biasLearningRate then
	    local cur_min = self.modules[i].biasLearningRate:min()
	    local cur_max = self.modules[i].biasLearningRate:max()
	    min = math.min(min,cur_min)
	    max = math.max(max,cur_max)
	 end
      end
   end
   return min,max
end

function SequentialHessian:zeroHessianParameters()
   for i=1,#self.modules do
      if self.modules[i].zeroHessianParameters then
         self.modules[i]:zeroHessianParameters()
      end
   end
end

function SequentialHessian:backwardHessian(input, hessianOutput)
   local currentHessianOutput = hessianOutput
   local currentModule = self.modules[#self.modules]
   local k = 1
   for i=#self.modules-1,1,-1 do
      local previousModule = self.modules[i]
      currentHessianOutput = currentModule:backwardHessian(previousModule.output, currentHessianOutput)
      currentModule = previousModule
   end
   -- the following IF is temporary, to allow last modules to be Hessian-less.
   if currentModule.backwardHessian then
      currentHessianOutput = currentModule:backwardHessian(input, currentHessianOutput)
   end
  return currentHessianOutput
end
