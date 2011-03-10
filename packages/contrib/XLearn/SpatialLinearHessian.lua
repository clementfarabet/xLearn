local SpatialLinearHessian, parent = torch.class('nn.SpatialLinearHessian', 'nn.SpatialLinear')

function SpatialLinearHessian:__init(fanin, fanout)
   parent.__init(self, fanin, fanout)

   -- 2nd derivatives
   self.hessianWeight = torch.Tensor(self.fanin, self.fanout)
   self.hessianBias = torch.Tensor(self.fanout)
   self.hessianInput = torch.Tensor(1,1,fanin)

   -- Squared vectors, for hessian backward
   self.squaredInput = torch.Tensor(1,1,fanin)
   self.squaredWeight = torch.Tensor(self.fanin, self.fanout)

   -- Hessian learning rates
   self.weightLearningRate = torch.Tensor(self.fanin, self.fanout)
   self.biasLearningRate = torch.Tensor(self.fanout)
end

function SpatialLinearHessian:zeroHessianParameters()
   self.hessianWeight:zero()
   self.hessianBias:zero()
end

function SpatialLinearHessian:zeroLearningRates()
   self.weightLearningRate:zero()
   self.biasLearningRate:zero()
end

function SpatialLinearHessian:oneLearningRates()
   self.weightLearningRate:fill(1)
   self.biasLearningRate:fill(1)
end

function SpatialLinearHessian:boundLearningRates(mu)
   self.weightLearningRate:add(mu)
   self.biasLearningRate:add(mu)
end

function SpatialLinearHessian:updateLearningRates(factor)
   self.weightLearningRate:add(factor, self.hessianWeight)
   self.biasLearningRate:add(factor, self.hessianBias)
end

function SpatialLinearHessian:updateParameters(learningRate)
   -- learning rate is adapted for each weight, using the curvature information
   self.weight:addcdiv(-learningRate, self.gradWeight, self.weightLearningRate)
   self.bias:addcdiv(-learningRate, self.gradBias, self.biasLearningRate)
end

function SpatialLinearHessian:backwardHessian(input, hessianOutput)   
   -- resize input
   self.hessianInput:resize(input:size(1), input:size(2), self.fanin)

   -- square weight and input:
   self.squaredInput:resizeAs(input):copy(input):cmul(input)
   self.squaredWeight:copy(self.weight):cmul(self.weight)
   
   for x=1,input:size(1) do
      for y=1,input:size(2) do
         self.hessianWeight:addT1outT1(1, self.squaredInput[x][y], hessianOutput[x][y])
         self.hessianBias:add(hessianOutput[x][y])
         
         self.hessianInput[x][y]:zero()
         self.hessianInput[x][y]:addT2dotT1(1, self.squaredWeight, hessianOutput[x][y])
      end
   end

   return self.gradInput, self.hessianInput
end

function SpatialLinearHessian:write(file)
   parent.write(self, file)
   file:writeObject(self.hessianWeight)
   file:writeObject(self.hessianBias)
   file:writeObject(self.hessianInput)
   file:writeObject(self.squaredWeight)
   file:writeObject(self.squaredInput)
   file:writeObject(self.weightLearningRate)
   file:writeObject(self.biasLearningRate)
end

function SpatialLinearHessian:read(file)
   parent.read(self, file)
   self.hessianWeight = file:readObject()
   self.hessianBias = file:readObject()
   self.hessianInput = file:readObject()
   self.squaredWeight = file:readObject()
   self.squaredInput = file:readObject()
   self.weightLearningRate = file:readObject()
   self.biasLearningRate = file:readObject()
end
