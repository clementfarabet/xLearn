local SplitTable, parent = torch.class('nn.SplitTable', 'nn.Module')

function SplitTable:__init(dimension)
   parent.__init(self)
   self.modules = {} 
   self.dimension = dimension
end

function SplitTable:forward(input)
   local currentOutput={}; 
   local slices=input:size(self.dimension);
   for i=1,slices do
      currentOutput[#currentOutput+1] = input:select(self.dimension,i)
   end
   self.output= currentOutput
   return self.output
end 


function SplitTable:backward(input, gradOutput)

   local slices=input:size(self.dimension)
   self.gradInput:resizeAs(input)

   local offset = 1
   for i=1,slices do 
      local currentGradInput = gradOutput[i];        
      self.gradInput:select(self.dimension,i):copy(currentGradInput)
   end
   return self.gradInput
end

function SplitTable:zeroGradParameters()
end

function SplitTable:updateParameters(learningRate)
end

function SplitTable:write(file)
   parent.write(self, file)
   file:writeObject(self.modules)
   file:writeInt(self.dimension)   
end

function SplitTable:read(file)
   parent.read(self, file)
   self.modules = file:readObject()
   self.dimension = file:readInt()
end
