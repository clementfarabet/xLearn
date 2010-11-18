local Add, parent = torch.class('nn.Add', 'nn.Module')

------------------------------------------------------------
-- HELP
local help_desc = 
[[Applies a bias term to the incoming data, i.e. _y_i= x_i + b_i, 
or if scalar=true then uses a single bias term, _y_i= x_i + b.]]

local help_example = 
[[y=torch.Tensor(5);  
mlp=nn.Sequential()
mlp:add(nn.Add(5))

function gradUpdate(mlp, x, y, criterion, learningRate) 
  local pred = mlp:forward(x)
  local err = criterion:forward(pred, y)
  local gradCriterion = criterion:backward(pred, y)
  mlp:zeroGradParameters()
  mlp:backward(x, gradCriterion)
  mlp:updateParameters(learningRate)
  return err
end

for i=1,10000 do
 x=lab.rand(5)
 y:copy(x); 
 for i=1,5 do y[i]=y[i]+i; end
 err=gradUpdate(mlp,x,y,nn.MSECriterion(),0.01)
end
print(mlp:get(1).bias)

-- gives the output:
 1.0000
 2.0000
 3.0000
 4.0000
 5.0000
[torch.Tensor of dimension 5]

--i.e. the network successfully learns the input x has been shifted 
to produce the output y.]]
-- HELP
------------------------------------------------------------

function Add:__init(inputSize,scalar)
   parent.__init(self)

   -- usage
   if not inputSize then
      error(toolBox.usage('nn.Add', help_desc, help_example,
                          {type='number', help='input dimension', req=true},
                          {type='boolean', help='use a single bias (scalar)'}))
   end

   local size=inputSize
   if scalar then size=1; end
   self.bias = torch.Tensor(size)
   self.gradBias = torch.Tensor(size)
     
   -- state
   self.gradInput:resize(inputSize)
   self.output:resize(inputSize) 

   self:reset()
end

 
function Add:reset(stdv)
   if stdv then 
      stdv = stdv * math.sqrt(3)
   else
      stdv = 1./math.sqrt(self.bias:size(1))
   end

   for i=1,self.bias:size(1) do
	   self.bias[i] = random.uniform(-stdv, stdv)
   end
end

function Add:forward(input)
   self.output:copy(input);
   if self.gradBias:size(1)==1 then
     self.output:add(self.bias[1]);
   else
     self.output:add(self.bias);
   end
   return self.output
end 

function Add:backward(input, gradOutput)
   if self.gradBias:size(1)==1 then
	self.gradBias[1]=self.gradBias[1]+ gradOutput:sum();
   else
   	self.gradBias:add(gradOutput)
   end
   self.gradInput:copy(gradOutput) 
   return self.gradInput
end

function Add:zeroGradParameters()
   self.gradBias:zero()
end

function Add:updateParameters(learningRate)
   self.bias:add(-learningRate, self.gradBias)
end

function Add:write(file)
   parent.write(self, file)
   file:writeObject(self.bias)
   file:writeObject(self.gradBias)
end

function Add:read(file)
   parent.read(self, file) 
   self.bias = file:readObject()
   self.gradBias = file:readObject()
end
