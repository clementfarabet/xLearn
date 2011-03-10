local StochasticHessianTrainer, parent = torch.class('nn.StochasticHessianTrainer','nn.Trainer')

function StochasticHessianTrainer:__init(module, criterion)
   parent.__init(self)
   self.module = module
   self.criterion = criterion
   self.learningRate = 0.1
   self.learningRateDecay = 0
   self.hessianUpdateFrequency = 0  -- 0 == the hessian will never be updated, unless unexisting
   self.hessianSamplesUsed = 100  -- nb of samples used to estimate the hessian
   self.boundOnLearningRates = 0.01
   self.maxEpoch = 0 -- 0 == no max
end

------------------------------------------------------------
--- This method computes the diagonal hessian, following
-- the stochastic diaginal levenberg marquardt method.
--
function StochasticHessianTrainer:computeDiagonalHessian(dataset,offset)
   local shuffledIndices = {}
   if not self.shuffleIndices then
      for t = 1,dataset:size() do
         shuffledIndices[t] = t
      end
   else
      shuffledIndices = lab.randperm(dataset:size())
   end

   local offset1 = offset or 1

   -- reset learning rates
   local module = self.module
   module:zeroLearningRates()

   print("# Estimating Diagonal Hessian over " .. self.hessianSamplesUsed .. " samples")

   for t = offset1,offset1-1+self.hessianSamplesUsed do
      local sample = dataset[shuffledIndices[t]]
      local input = sample[1]
      local target = sample[2]

      -- (1) forward
      module:forward(input)

      -- (2) backprop gradients
      module:zeroGradParameters()
      module:backward(input, self.criterion:backward(module.output, target))

      -- (3) backprop 2nd derivatives (to build up diagonal hessian)
      module:zeroHessianParameters()
      module:backwardHessian(input, self.criterion:backwardHessian(module.output, target))

      -- (4) update learning rates:
      -- each update is divided by the number of samples used to estimate the hessian
      module:updateLearningRates(1/self.hessianSamplesUsed)
   end

   -- bound the learning rates
   module:boundLearningRates(self.boundOnLearningRates)

   -- print stats
   print('# 2nd derivates range:',module:getMinMaxSecondDerivatives())
end

function StochasticHessianTrainer:train(dataset)
   local epoch = 1
   local currentLearningRate = self.learningRate
   local module = self.module
   local criterion = self.criterion
   self.trainset = dataset

   local shuffledIndices = {}
   if not self.shuffleIndices then
      for t = 1,dataset:size() do
         shuffledIndices[t] = t
      end
   else
      shuffledIndices = lab.randperm(dataset:size())
   end

   while true do
      print('# Training Set:')

      if self.hessianUpdateFrequency == 0 then
         print('# Hessian not computed, using uniform (1st order) learning rates')
         self.module:oneLearningRates()
      else
         self:computeDiagonalHessian(dataset,1)
      end

      print("# Stochastic gradient descent epoch # " .. epoch)

      self.currentError = 0
      for t = 1,dataset:size() do
         toolBox.dispProgress(t, dataset:size())
         local sample = dataset[shuffledIndices[t]]
         local input = sample[1]
         local target = sample[2]
         self.currentError = self.currentError + criterion:forward(module:forward(input), target)

         -- backprop gradients
         module:zeroGradParameters()
         module:backward(input, criterion:backward(module.output, target))

         if self.hookTrainSample then
            self.hookTrainSample(self, sample)
         end
         module:updateParameters(currentLearningRate)
      end

      self.currentError = self.currentError / dataset:size()
      print("# current error = " .. self.currentError)

      if self.hookTrainEpoch then
         self.hookTrainEpoch(self)
      end

      epoch = epoch + 1
      currentLearningRate = self.learningRate/(1+epoch*self.learningRateDecay)

      if self.maxEpoch > 0 and epoch > self.maxEpoch then
         print("# StochasticHessianGradient: you have reached the maximum number of epochs")
         break
      end      
   end
end


function StochasticHessianTrainer:test(dataset)
   print('# Testing Set:')

   local module = self.module
   local shuffledIndices = {}
   local criterion = self.criterion
   self.currentError = 0
   self.testset = dataset


   local shuffledIndices = {}
   if not self.shuffleIndices then
      for t = 1,dataset:size() do
         shuffledIndices[t] = t
      end
   else
      shuffledIndices = lab.randperm(dataset:size())
   end
   
   for t = 1,dataset:size() do
      toolBox.dispProgress(t, dataset:size())
      local sample = dataset[shuffledIndices[t]]
      local input = sample[1]
      local target = sample[2]
      if self.hookTestSample then
         self.hookTestSample(self, sample)
      end
      self.currentError = self.currentError + criterion:forward(module:forward(input), target)
   end

   self.currentError = self.currentError / dataset:size()
   print("# current error = " .. self.currentError)


   if self.hookTestEpoch then
      self.hookTestEpoch(self)
   end

   return self.currentError
end

function StochasticHessianTrainer:write(file)
   parent.write(self,file)
   file:writeObject(self.module)
   file:writeObject(self.criterion)
end

function StochasticHessianTrainer:read(file)
   parent.read(self,file)
   self.module = file:readObject()
   self.criterion = file:readObject()
end
