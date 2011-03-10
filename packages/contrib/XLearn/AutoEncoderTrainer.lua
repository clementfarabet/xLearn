local AutoEncoderTrainer, parent = torch.class('nn.AutoEncoderTrainer','nn.Trainer')

function AutoEncoderTrainer:__init(encoder, decoder, 
                                   codeCriterion, decoderCriterion, encoderCriterion,
                                   -- optional params
                                   fileName, maxiterCode)
   parent.__init(self)
   self.decoder = decoder
   self.encoder = encoder
   self.codeCriterion = codeCriterion   
   self.decoderCriterion = decoderCriterion   
   self.encoderCriterion = encoderCriterion
   -- learning rate
   self.decoderLearningRate = decoderLearningRate or 0.02
   self.encoderLearningRate = encoderLearningRate or 0.02
   self.codeLearningRate = codeLearningRate or 0.005
   -- learning decay
   self.decoderLearningDecay = decoderLearningDecay or 0.01
   self.encoderLearningDecay = encoderLearningDecay or 0.01
   self.codeLearningDecay = codeLearningDecay or 0.001
   -- energy weights
   self.alphaDecoder = alphaDecoder or 1
   self.alphaEncoder = alphaEncoder or 1
   self.alphaCode = alphaCode or 0.5
   -- disk output file
   self.fileName = fileName or 'Trained_AutoEncoder.torch'
   -- #iterations for finding sparse code
   self.maxiterCode = maxiterCode or 40
end

function AutoEncoderTrainer:findSparseCode(initialCode,reconstructTarget)
   local decoder = self.decoder
   for _=1,self.maxiterCode do
      local recons = decoder:forward(initialCode)
      local gradOutDec = self.decoderCriterion:backward(recons, reconstructTarget)
      local gradSparse = self.codeCriterion:backward(initialCode,self.codeTarget)
      local gradCode = decoder:backward(initialCode,gradOutDec)
      gradCode:add(self.alphaCode, gradSparse)
      initialCode:add(-self.codeLearningRate,gradCode)
   end
end

function AutoEncoderTrainer:learnDecoder(code, reconstruction,reconstructTarget)
   local decoder = self.decoder
   decoder:zeroGradParameters()
   local gradOutDec = self.decoderCriterion:backward(reconstruction, reconstructTarget)
   decoder:backward(code,gradOutDec)
   decoder:updateParameters(self.decoderLearningRate)
end

function AutoEncoderTrainer:learnEncoder(sparseCode, sparseCodeLearnt, reconstructTarget)
   local encoder = self.encoder
   encoder:zeroGradParameters()
   local gradOutEnc = self.encoderCriterion:backward(sparseCode, sparseCodeLearnt)
   encoder:backward(reconstructTarget,gradOutEnc)
   encoder:updateParameters(self.encoderLearningRate)
end

function AutoEncoderTrainer:train(dataset,width,height)
   local shuffledIndices = {}
   local epoch = 1
   if not self.shuffleIndices then
      for t = 1,dataset:size() do
         shuffledIndices[t] = t
      end
   else
      shuffledIndices = lab.randperm(dataset:size())
   end

   -- local vars for training
   local codeP = torch.Tensor()

   print("# Autoencoder: training")
   while true do
      local currentError = 0
      for t = 1,dataset:size() do
         toolBox.dispProgress(t, dataset:size())
         local example = dataset:next{width=width,height=width}
         local code = encoder:forward(example)
         codeP:resizeAs(code):copy(code)
         -- find sparse code
         self:findSparseCode(codeP,example)
         -- learn decoder
         local reconstruction = decoder:forward(codeP)
         self:learnDecoder(codeP,reconstruction,example)
         -- learn encoder  
         self:learnEncoder(code, codeP, example)
         currentError = currentError + decoderCriterion:forward(decoder:forward(codeP), example)

         if self.hookExample then
            self.hookExample(self, example, t)
         end
      end

      if self.hookIteration then
         self.hookIteration(self, epoch)
      end
      
      currentError = currentError / dataset:size()
      print("# current error = " .. currentError)

      -- eventually save the modules
      if self.savetime and epoch % self.savetime == 0 then
         self:save()
      end

      epoch = epoch + 1
      decoderLearningRate = self.decoderLearningRate/(1+epoch*
                                                      self.decoderLearningDecay)
      encoderLearningRate = self.encoderLearningRate/(1+epoch*
                                                      self.encoderLearningDecay)
      codeLearningRate = self.codeLearningRate/(1+epoch*
                                                      self.codeLearningDecay)
      if self.maxIteration > 0 and epoch > self.maxIteration then
         print("# Autoencoder: you have reached the maximum number of epochs")
         break
      end  
   end
end

function AutoEncoderTrainer:checkGoodSize(sample)
   local sampleSize =  sample:size()
   local winX = self.decoder.winX
   local winY = self.decoder.winY
   local woutX = self.decoder.woutX
   local woutY = self.decoder.woutY   
   local goodForTrainingSizeX = 3*(winX+woutX-1) + 2*(winX-1)
   local goodForTrainingSizeY = 3*(winY+woutY-1) + 2*(winY-1)
   if sampleSize[1] < goodForTrainingSizeX or sampleSize[2] < goodForTrainingSizeY then
      print("# Warning dataset too small for good training:")
      print("#   input data size:",sampleSize[1],sampleSize[2])
      print("# good minimum size:",goodForTrainingSizeX,goodForTrainingSizeY)
   end
end
function AutoEncoderTrainer:trainOld(dataset,width,height)
   -- check for optimal minimum size
   self:checkGoodSize(dataset:next{width=width,height=width})

   local epoch = 1
   local decoderLearningRate = self.decoderLearningRate
   local encoderLearningRate = self.encoderLearningRate
   local codeLearningRate = self.codeLearningRate

   local shuffledIndices = {}
   if not self.shuffleIndices then
      for t = 1,dataset:size() do
         shuffledIndices[t] = t
      end
   else
      shuffledIndices = lab.randperm(dataset:size())
   end

   -- local vars for training
   local codeP = torch.Tensor()

   print("# Autoencoder: training")
   while true do
      local currentError = 0
      for t = 1,dataset:size() do
         toolBox.dispProgress(t, dataset:size())
         local example = dataset:next{width=width,height=width}
         local code = encoder:forward(example)
         codeP:resizeAs(code):copy(code)
         -- find sparse code
         for _=1,self.maxiterCode do
            local exampleP = decoder:forward(codeP)
            local gradOutDec = decoderCriterion:backward(exampleP, example)
            local gradSparse = codeCriterion:backward(codeP)
            local gradCode = decoder:backward(codeP,gradOutDec)
            gradCode:add(self.alphaCode, gradSparse)
            --  (==> enc-cost bprop code codep enc-energy)    WTF ???
            ---;; code gradient from similarity to prediction WTF ?????
            codeP:add(-self.codeLearningRate,gradCode)
         end

         -- learn decoder
         decoder:zeroGradParameters()
         local gradOutDec = decoderCriterion:backward(decoder:forward(codeP), example)
         --local coefs = torch.Tensor(gradOutDec:size(1),gradOutDec:size(2)):zero()
         --coefs:narrow(1,winX-1,gradOutDec:size(1)-2*(winX-1)):narrow(2,winY-1,gradOutDec:size(2)-2*(winY-1)):fill(1)
         --gradOutDec:cmul(coefs) 
         decoder:backward(codeP,gradOutDec)
         decoder:updateParameters(self.decoderLearningRate)
         -- learn encoder  
         encoder:zeroGradParameters()
         local gradOutEnc = encoderCriterion:backward(code, codeP)
         --coefs = torch.Tensor(gradOutEnc:size(1),gradOutEnc:size(2)):zero()
         --coefs:narrow(1,winX-1,gradOutEnc:size(1)-2*(winX-1)):narrow(2,winY-1,gradOutEnc:size(2)-2*(winY-1)):fill(1)
         --gradOutEnc:cmul(coefs)
         encoder:backward(example,gradOutEnc)
         encoder:updateParameters(self.encoderLearningRate)

         currentError = currentError + decoderCriterion:forward(decoder:forward(codeP), example)
         if self.hookExample then
            self.hookExample(self, example, t)
         end
      end

      if self.hookIteration then
         self.hookIteration(self, epoch)
      end
      
      currentError = currentError / dataset:size()
      print("# current error = " .. currentError)

      -- eventually save the modules
      if self.savetime and epoch % self.savetime == 0 then
         self:save()
      end

      epoch = epoch + 1
      decoderLearningRate = self.decoderLearningRate/(1+epoch*
                                                      self.decoderLearningDecay)
      encoderLearningRate = self.encoderLearningRate/(1+epoch*
                                                      self.encoderLearningDecay)
      codeLearningRate = self.codeLearningRate/(1+epoch*
                                                      self.codeLearningDecay)
      if self.maxIteration > 0 and epoch > self.maxIteration then
         print("# Autoencoder: you have reached the maximum number of epochs")
         break
      end  
   end
end

function AutoEncoderTrainer:save(fileName)
   local fileName = fileName or self.fileName
   self.fileName = fileName
   print('saving AutoEncoder to:',fileName)
   local file = torch.DiskFile(fileName, 'w')
   self:write(file)
   file:close()
end

function AutoEncoderTrainer:open(fileName)
   local fileName = fileName or self.fileName
   self.fileName = fileName
   print('loading from File:',fileName)
   local file = torch.DiskFile(fileName, 'r')
   self:read(file)
   file:close()
end

function AutoEncoderTrainer:write(file)
   parent.write(self,file)
   file:writeObject(self.decoder)
   file:writeObject(self.encoder)
   file:writeObject(self.codeCriterion)
   file:writeObject(self.decoderCriterion)
   file:writeObject(self.encoderCriterion)
   file:writeDouble(self.decoderLearningRate)
   file:writeDouble(self.encoderLearningRate)
   file:writeDouble(self.codeLearningRate)
   file:writeDouble(self.decoderLearningDecay)
   file:writeDouble(self.encoderLearningDecay)
   file:writeDouble(self.codeLearningDecay)
   file:writeDouble(self.alphaDecoder)
   file:writeDouble(self.alphaEncoder)
   file:writeDouble(self.alphaCode)
   file:writeString(self.fileName.."\n")
   file:writeInt(self.maxiterCode)
end

function AutoEncoderTrainer:read(file)
   parent.read(self,file)
   self.decoder = file:readObject()
   self.encoder = file:readObject()
   self.codeCriterion = file:readObject()
   self.decoderCriterion = file:readObject()
   self.encoderCriterion = file:readObject()
   self.decoderLearningRate = file:readDouble()
   self.encoderLearningRate = file:readDouble()
   self.codeLearningRate = file:readDouble()
   self.decoderLearningDecay = file:readDouble()
   self.encoderLearningDecay = file:readDouble()
   self.codeLearningDecay = file:readDouble()
   self.alphaDecoder = file:readDouble()
   self.alphaEncoder = file:readDouble()
   self.alphaCode = file:readDouble()
   self.fileName = file:readString("*l")
   self.maxiterCode = file:readInt()
end
