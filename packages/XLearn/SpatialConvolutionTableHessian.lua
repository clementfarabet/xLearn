
local SpatialConvolutionTableHessian, parent = torch.class('nn.SpatialConvolutionTableHessian', 
                                                           'nn.SpatialConvolutionTable')

function SpatialConvolutionTableHessian:__init(connMatrix, kW, kH, dW, dH)
   parent.__init(self, connMatrix, kW, kH, dW, dH)

   -- 2nd derivatives
   self.hessianWeight = torch.Tensor(kW, kH, self.connTable:size(1))
   self.hessianBias = torch.Tensor(self.nOutputPlane)
   self.hessianInput = torch.Tensor()

   -- Squared vectors, for hessian backward
   self.squaredInput = torch.Tensor()
   self.squaredWeight = torch.Tensor(kW, kH, self.connTable:size(1))

   -- Hessian learning rates
   self.weightLearningRate = torch.Tensor(kW, kH, self.connTable:size(1))
   self.biasLearningRate = torch.Tensor(self.nOutputPlane)
end

function SpatialConvolutionTableHessian:zeroHessianParameters()
   self.hessianWeight:zero()
   self.hessianBias:zero()
end

function SpatialConvolutionTableHessian:zeroLearningRates()
   self.weightLearningRate:zero()
   self.biasLearningRate:zero()
end

function SpatialConvolutionTableHessian:oneLearningRates()
   self.weightLearningRate:fill(1)
   self.biasLearningRate:fill(1)
end

function SpatialConvolutionTableHessian:boundLearningRates(mu)
   self.weightLearningRate:add(mu)
   self.biasLearningRate:add(mu)
end

function SpatialConvolutionTableHessian:updateLearningRates(factor)
   self.weightLearningRate:add(factor, self.hessianWeight)
   self.biasLearningRate:add(factor, self.hessianBias)
end

function SpatialConvolutionTableHessian:updateParameters(learningRate)
   -- learning rate is adapted for each weight, using the curvature information
   self.weight:addcdiv(-learningRate, self.gradWeight, self.weightLearningRate)
   self.bias:addcdiv(-learningRate, self.gradBias, self.biasLearningRate)
end

function SpatialConvolutionTableHessian:write(file)
   parent.write(self, file)
   file:writeObject(self.hessianWeight)
   file:writeObject(self.hessianBias)
   file:writeObject(self.hessianInput)
   file:writeObject(self.squaredWeight)
   file:writeObject(self.squaredInput)
   file:writeObject(self.weightLearningRate)
   file:writeObject(self.biasLearningRate)
end

function SpatialConvolutionTableHessian:read(file)
   parent.read(self, file)
   self.hessianWeight = file:readObject()
   self.hessianBias = file:readObject()
   self.hessianInput = file:readObject()
   self.squaredWeight = file:readObject()
   self.squaredInput = file:readObject()
   self.weightLearningRate = file:readObject()
   self.biasLearningRate = file:readObject()
end
