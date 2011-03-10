
local SpatialSubSamplingHessian, parent = torch.class('nn.SpatialSubSamplingHessian', 
                                                      'nn.SpatialSubSampling')

function SpatialSubSamplingHessian:__init(nInputPlane, kW, kH, dW, dH)
   parent.__init(self, nInputPlane, kW, kH, dW, dH)

   -- 2nd derivatives
   self.hessianWeight = torch.Tensor(self.nInputPlane)
   self.hessianBias = torch.Tensor(self.nInputPlane)
   self.hessianInput = torch.Tensor()

   -- Squared vectors, for hessian backward
   self.squaredInput = torch.Tensor()
   self.squaredWeight = torch.Tensor(self.nInputPlane)

   -- Hessian learning rates
   self.weightLearningRate = torch.Tensor(self.nInputPlane)
   self.biasLearningRate = torch.Tensor(self.nInputPlane)
end

function SpatialSubSamplingHessian:zeroHessianParameters()
   self.hessianWeight:zero()
   self.hessianBias:zero()
end

function SpatialSubSamplingHessian:zeroLearningRates()
   self.weightLearningRate:zero()
   self.biasLearningRate:zero()
end

function SpatialSubSamplingHessian:oneLearningRates()
   self.weightLearningRate:fill(1)
   self.biasLearningRate:fill(1)
end

function SpatialSubSamplingHessian:boundLearningRates(mu)
   self.weightLearningRate:add(mu)
   self.biasLearningRate:add(mu)
end

function SpatialSubSamplingHessian:updateLearningRates(factor)
   self.weightLearningRate:add(factor, self.hessianWeight)
   self.biasLearningRate:add(factor, self.hessianBias)
end

function SpatialSubSamplingHessian:updateParameters(learningRate)
   -- learning rate is adapted for each weight, using the curvature information
   self.weight:addcdiv(-learningRate, self.gradWeight, self.weightLearningRate)
   self.bias:addcdiv(-learningRate, self.gradBias, self.biasLearningRate)
end

function SpatialSubSamplingHessian:write(file)
   parent.write(self, file)
   file:writeObject(self.hessianWeight)
   file:writeObject(self.hessianBias)
   file:writeObject(self.hessianInput)
   file:writeObject(self.squaredWeight)
   file:writeObject(self.squaredInput)
   file:writeObject(self.weightLearningRate)
   file:writeObject(self.biasLearningRate)
end

function SpatialSubSamplingHessian:read(file)
   parent.read(self, file)
   self.hessianWeight = file:readObject()
   self.hessianBias = file:readObject()
   self.hessianInput = file:readObject()
   self.squaredWeight = file:readObject()
   self.squaredInput = file:readObject()
   self.weightLearningRate = file:readObject()
   self.biasLearningRate = file:readObject()
end
