local StochasticTrainer, parent = torch.class('nn.StochasticTrainer','nn.Trainer')

function StochasticTrainer:__init(module, criterion, preprocessor)
   parent.__init(self)
   -- load given module
   self.module = module
   -- load builtin criterion, or user criterion
   if criterion == 'MSE' then
      self.criterion = nn.MSECriterion()
   elseif criterion == 'NLL' then
      self.criterion = nn.ClassNLLCriterion()
      self.maxTarget = true
   else
      self.criterion = criterion    
   end
   -- use optional preprocessor
   self.preprocessor = preprocessor
   -- public options
   self.dispProgress = true
   self.skipUniformTargets = false
   self.errorArray = false
   self.maxTarget = self.maxTarget or false
   self.weightDecay = 0
   -- private
   self.trainOffset = 0
   self.testOffset = 0
end

function StochasticTrainer:train(dataset)
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
   
   while true do
      print('# Training Set:')
      print("# Stochastic gradient descent epoch # " .. self.epoch)

      self.currentError = 0
      for t = 1,dataset:size() do
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
         print("# StochasticGradient: you have reached the maximum number of epochs")
         break
      end

      if dataset.infiniteSet then
         self.trainOffset = self.trainOffset + dataset:size()
      end
   end
end


function StochasticTrainer:test(dataset)
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
      -- disp progress
      if self.dispProgress then
         toolBox.dispProgress(t, dataset:size())
      end

      -- get new sample
      local sample = dataset[self.testOffset + shuffledIndices[t]]
      local input = sample[1]
      local target = sample[2]

      -- max target ?
      if self.maxTarget then
         target = torch.Tensor(target:nElement()):copy(target)
         _,target = lab.max(target)
         target = target[1]
      end
      
      -- test sample through current model
      if self.preprocessor then input = self.preprocessor:forward(input) end
      if criterion then
         self.currentError = self.currentError + 
	    criterion:forward(module:forward(input), target)
      else
         local _,error = module:forward(input, target)
         self.currentError = self.currentError + error
      end

      -- user hook
      if self.hookTestSample then
         self.hookTestSample(self, sample)
      end
   end

   self.currentError = self.currentError / dataset:size()
   print("# test current error = " .. self.currentError)


   if self.hookTestEpoch then
      self.hookTestEpoch(self)
   end

   if dataset.infiniteSet then
      self.testOffset = self.testOffset + dataset:size()
   end

   return self.currentError
end

function StochasticTrainer:write(file)
   parent.write(self,file)
   file:writeObject(self.module)
   file:writeObject(self.criterion)
end

function StochasticTrainer:read(file)
   parent.read(self,file)
   self.module = file:readObject()
   self.criterion = file:readObject()
end
