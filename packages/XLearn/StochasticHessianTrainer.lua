local StochasticHessianTrainer, parent = torch.class('nn.StochasticHessianTrainer','nn.StochasticTrainer')

function StochasticHessianTrainer:__init(module, criterion, preprocessor)
   parent.__init(self,module,criterion,preprocessor)
   -- public options
   self.hessianUpdateFrequency = 0  -- 0 == the hessian will only be updated once, -1 == never, N == every N samples
   self.hessianSamplesUsed = 100  -- nb of samples used to estimate the hessian
   self.boundOnLearningRates = 0
end

------------------------------------------------------------
--- This method computes the diagonal hessian, following
-- the stochastic diaginal levenberg marquardt method.
--
function StochasticHessianTrainer:computeDiagonalHessian(dataset,indices,offset)

   print("# Estimating Diagonal Hessian over " .. self.hessianSamplesUsed .. " samples")

   -- reset learning rates
   local module = self.module
   local criterion = self.criterion
   module:zeroLearningRates()

   offset = offset or 1
   for t = offset,offset-1+self.hessianSamplesUsed do
      local sample = dataset[indices[t]]
      local input = sample[1]
      local target = sample[2]

      -- (1) forward
      -- (if no criterion, it is assumed to be contained in the model)
      if criterion then
	 module:forward(input)
	 error = criterion:forward(modelOut, target)
      else
	 module:forward(input, target, sample_x, sample_y)
      end
      
      -- (2) backprop gradients
      -- (if no criterion, it is assumed that criterionGrad is internally generated)
      module:zeroGradParameters()
      if criterion then
	 local criterionGrad = self.criterion:backward(module.output, target)
	 module:backward(input, criterionGrad)
      else
	 module:backward(input)
      end

      -- (3) backprop 2nd derivatives (to build up diagonal hessian)
      module:zeroHessianParameters()
      if criterion then 
	 local criterionHessian = self.criterion:backwardHessian(module.output, target)
	 module:backwardHessian(input, criterionHessian)
      else
	 module:backwardHessian(input)
      end
      -- (4) update learning rates:
      -- each update is divided by the number of samples used to estimate the hessian
      module:updateLearningRates(1/self.hessianSamplesUsed)
   end

   -- print raw stats
   print('# 2nd derivates range:',module:getMinMaxSecondDerivatives())

   -- bound the learning rates
   module:boundLearningRates(self.boundOnLearningRates)

   -- print stats
   print('# bounded 2nd derivates range:',module:getMinMaxSecondDerivatives())
end

function StochasticHessianTrainer:train(dataset)
   self.epoch = self.epoch or 1
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
   
   if self.hessianUpdateFrequency == -1 then
      print('# Hessian not computed, using uniform (1st order) learning rates')
      self.module:oneLearningRates()
   end

   while true do
      print('# Training Set:')

      if self.hessianUpdateFrequency == 0 then
         print('# estimating diagonal of the Hessian once for the entire epoch, using ' 
               .. self.hessianSamplesUsed .. ' samples')
         self:computeDiagonalHessian(dataset,shuffledIndices,1)
      end

      print("# Stochastic gradient descent epoch # " .. self.epoch)

      self.currentError = 0
      for t = 1,dataset:size() do

         -- compute diagonal hessian ?
         if ((t-1)%self.hessianUpdateFrequency) == 0 then
            print('')
            print('# estimating diagonal of the Hessian, using ' 
                  .. self.hessianSamplesUsed .. ' samples')
            self:computeDiagonalHessian(dataset,shuffledIndices,t)
         end

         -- disp progress
         if self.dispProgress then
            toolBox.dispProgress(t, dataset:size())
         end

         -- load new sample
         local sample = dataset[self.trainOffset + shuffledIndices[t]]
         local input = sample[1]
         local target = sample[2]
         local sample_x = sample.x
         local sample_y = sample.y

         -- get max of target ?
         if self.maxTarget then
            target = torch.Tensor(target:nElement()):copy(target)
            _,target = lab.max(target)
            target = target[1]
         end

         -- is target uniform ?
         local isUniform = false
         if self.errorArray and target:min() == target:max() then
            isUniform = true
         end

         -- perform SGD step
         if not (self.skipUniformTargets and isUniform) then
            -- optional preprocess
            if self.preprocessor then input = self.preprocessor:forward(input) end

            -- forward through model and criterion 
            -- (if no criterion, it is assumed to be contained in the model)
            local modelOut, error
            if criterion then
               modelOut = module:forward(input)
               error = criterion:forward(modelOut, target)
            else
               modelOut, error = module:forward(input, target, sample_x, sample_y)
            end

            -- accumulate error
            self.currentError = self.currentError + error

            -- backward through model
            -- (if no criterion, it is assumed that derror is internally generated)
            module:zeroGradParameters()
            if criterion then
               local derror = criterion:backward(module.output, target)
               module:backward(input, derror)
            else
               module:backward(input)
            end

            -- weight decay ?
            if self.weightDecay ~= 0 then
               module:decayParameters(self.weightDecay)
            end

            -- update parameters in the model
            module:updateParameters(currentLearningRate)
         end

         -- call user hook, if any
         if self.hookTrainSample then
            self.hookTrainSample(self, sample)
         end
      end

      self.currentError = self.currentError / dataset:size()
      print("# train current error = " .. self.currentError)

      if self.hookTrainEpoch then
         self.hookTrainEpoch(self)
      end

      self.epoch = self.epoch + 1
      currentLearningRate = self.learningRate/(1+self.epoch*self.learningRateDecay)

      if self.maxEpoch > 0 and self.epoch > self.maxEpoch then
         print("# StochasticHessianGradient: you have reached the maximum number of epochs")
         break
      end

      if dataset.infiniteSet then
         self.trainOffset = self.trainOffset + dataset:size()
      end
   end
end
