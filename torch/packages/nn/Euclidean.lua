local Euclidean, parent = torch.class('nn.Euclidean', 'nn.Module')

local help_desc = 
[[Outputs the Euclidean distance of the input to 
outputDimension centers, i.e. this layer has the 
weights c_i, i = 1,..,outputDimension, where c_i 
are vectors of dimension inputDimension. Output 
dimension j is || c_i - x||^2, where x is the input.]]

function Euclidean:__init(inputSize,outputSize)
   parent.__init(self)

   -- usage
   if not inputSize or not outputSize then
      error(toolBox.usage('nn.Euclidean', help_desc, nil,
                          {type='number', help='input dimension', req=true},
                          {type='number', help='input dimension', req=true}))
   end

   self.weight = torch.Tensor(inputSize,outputSize) 
   self.gradWeight = torch.Tensor(inputSize,outputSize)
   
   -- state
   self.gradInput:resize(inputSize)
   self.output:resize(outputSize)

   self:reset()
end
 
 
function Euclidean:reset(stdv)
   if stdv then
      stdv = stdv * math.sqrt(3)
   else
      stdv = 1./math.sqrt(self.weight:size(1))
   end

   -- we do this so the initialization is exactly
   -- the same than in previous torch versions
   for i=1,self.weight:size(2) do
      self.weight:select(2, i):apply(function()
                                        return random.uniform(-stdv, stdv)
                                     end)
   end
end

 
function Euclidean:forward(input) 
   self.output:zero()
   for i=1,self.weight:size(2) do
	self.output[i]=input:dist(self.weight:select(2,i))
   end 
   return self.output
end

function Euclidean:backward(input, gradOutput)
  
   for i=1,self.weight:size(2) do
    local gW=self.gradWeight:select(2,i) 
    gW:add(2*gradOutput[i],self.weight:select(2,i));
    gW:add(-2*gradOutput[i],input);
   end

   self.gradInput:zero();
   for i=1,self.weight:size(2) do
    self.gradInput:add(2*gradOutput[i],input);
    self.gradInput:add(-2*gradOutput[i],self.weight:select(2,i));
   end

   return self.gradInput
end

function Euclidean:zeroGradParameters()
   self.gradWeight:zero()
end
 
function Euclidean:updateParameters(learningRate)
   self.weight:add(-learningRate, self.gradWeight)
end

function Euclidean:write(file)
   parent.write(self, file)
   file:writeObject(self.weight)
   file:writeObject(self.gradWeight)
end

function Euclidean:read(file)
   parent.read(self, file)
   self.weight = file:readObject()
   self.gradWeight = file:readObject()
end
