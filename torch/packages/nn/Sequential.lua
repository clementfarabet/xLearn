local Sequential, parent = torch.class('nn.Sequential', 'nn.Module')

function Sequential:__init()
   self.modules = {}
end

function Sequential:add(module)
   if #self.modules == 0 then
      self.gradInput = module.gradInput
   end
   table.insert(self.modules, module)
   self.output = module.output
end

function Sequential:size()
   return #self.modules
end

function Sequential:get(index)
   return self.modules[index]
end

function Sequential:forward(input,...)
   local currentOutput = input
   for i=1,#self.modules do 
      currentOutput, error = self.modules[i]:forward(currentOutput,...)
   end 
   self.output = currentOutput
   return currentOutput, error
end

function Sequential:backward(input, gradOutput)
   local currentGradOutput = gradOutput
   local currentModule = self.modules[#self.modules]
   for i=#self.modules-1,1,-1 do
      local previousModule = self.modules[i]
      currentGradOutput = currentModule:backward(previousModule.output, currentGradOutput)
      currentModule = previousModule
   end
   currentGradOutput = currentModule:backward(input, currentGradOutput)
   self.gradInput = currentGradOutput
   return currentGradOutput
end

function Sequential:zeroGradParameters()
  for i=1,#self.modules do
     self.modules[i]:zeroGradParameters()
  end
end

function Sequential:updateParameters(learningRate)
   for i=1,#self.modules do
      self.modules[i]:updateParameters(learningRate)
   end
end

function Sequential:decayParameters(decay)
   for i=1,#self.modules do
      self.modules[i]:decayParameters(decay)
   end
end

function Sequential:empty()
  for i=1,#self.modules do
     self.modules[i]:empty()
  end
end

function Sequential:write(file)
   parent.write(self, file)
   file:writeObject(self.modules)
end

function Sequential:read(file)
   parent.read(self, file)
   self.modules = file:readObject()
end

function Sequential:share(mlp,...)
   for i=1,#self.modules do
      self.modules[i]:share(mlp.modules[i],...); 
   end
end
