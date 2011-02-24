local StochasticTrainer, parent = torch.class('nn.StochasticTrainer','nn.Trainer')

function StochasticTrainer:__init(module, criterion, preprocessor)
   parent.__init(self)
   self.module = module
   if criterion == 'MSE' then
      self.criterion = nn.MSECriterion()
   elseif criterion == 'NLL' then
      self.criterion = nn.ClassNLLCriterion()
      self.maxTarget = true
   else
      self.criterion = criterion    
   end
   self.preprocessor = preprocessor
   self.trainOffset = 0
   self.testOffset = 0
end

function StochasticTrainer:train(dataset)
   self.epoch = 1
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
         toolBox.dispProgress(t, dataset:size())
         local sample = dataset[self.trainOffset + shuffledIndices[t]]
         local input = sample[1]
         local target = sample[2]
         if self.maxTarget then
            target = torch.Tensor(target:nElement()):copy(target)
            _,target = lab.max(target)
            target = target[1]
         end

         -- optional preprocess
         if self.preprocessor then input = self.preprocessor:forward(input) end

         -- forward through model
         self.currentError = self.currentError + criterion:forward(module:forward(input), target)

         -- backward through model
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

      self.epoch = self.epoch + 1
      currentLearningRate = self.learningRate/(1+self.epoch*self.learningRateDecay)

      if self.maxEpoch > 0 and self.epoch > self.maxEpoch then
         print("# StochasticGradient: you have reached the maximum number of epochs")
         break
      end

      if dataset.infinite then
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
      toolBox.dispProgress(t, dataset:size())
      local sample = dataset[self.testOffset + shuffledIndices[t]]
      local input = sample[1]
      local target = sample[2]
      if self.maxTarget then
         target = torch.Tensor(target:nElement()):copy(target)
         _,target = lab.max(target)
         target = target[1]
      end
      if self.preprocessor then input = self.preprocessor:forward(input) end
      self.currentError = self.currentError + criterion:forward(module:forward(input), target)
      if self.hookTestSample then
         self.hookTestSample(self, sample)
      end
   end

   self.currentError = self.currentError / dataset:size()
   print("# current error = " .. self.currentError)


   if self.hookTestEpoch then
      self.hookTestEpoch(self)
   end

   if dataset.infinite then
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
