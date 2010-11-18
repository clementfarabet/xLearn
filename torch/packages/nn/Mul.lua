local Mul, parent = torch.class('nn.Mul', 'nn.Module')

------------------------------------------------------------
-- HELP
local help_desc = 
[[Applies a single scaling factor to the incoming data,
i.e. y= w x, where w is a scalar]]

local help_example = 
[[y=torch.Tensor(5);  
mlp=nn.Sequential()
mlp:add(nn.Mul(5))

function gradUpdate(mlp, x, y, criterion, learningRate) 
  local pred = mlp:forward(x)
  local err = criterion:forward(pred,y)
  local gradCriterion = criterion:backward(pred,y);
  mlp:zeroGradParameters();
  mlp:backward(x, gradCriterion);
  mlp:updateParameters(learningRate);
  return err
end


for i=1,10000 do
 x=lab.rand(5)
 y:copy(x); y:mul(math.pi);
 err=gradUpdate(mlp,x,y,nn.MSECriterion(),0.01)
end
print(mlp:get(1).weight)

-- gives the output:
3.1416
[torch.Tensor of dimension 1]

--i.e. the network successfully learns the input 
x has been scaled by pi.]]
-- HELP
------------------------------------------------------------

function Mul:__init(inputSize)
   parent.__init(self)

   -- usage
   if not inputSize then
      error(toolBox.usage('nn.Mul', help_desc, help_example,
                          {type='number', help='input dimension', req=true}))
   end

   self.weight = torch.Tensor(1)
   self.gradWeight = torch.Tensor(1)
   
   -- state
   self.gradInput:resize(inputSize)
   self.output:resize(inputSize) 

   self:reset()
end

 
function Mul:reset(stdv)
   if stdv then
      stdv = stdv * math.sqrt(3)
   else
      stdv = 1./math.sqrt(self.weight:size(1))
   end

   self.weight[1] = random.uniform(-stdv, stdv);
end

function Mul:forward(input)
   self.output:copy(input);
   self.output:mul(self.weight[1]);
   return self.output 
end

function Mul:backward(input, gradOutput) 
   self.gradWeight[1] =   self.gradWeight[1] + input:dot(gradOutput);
   self.gradInput:zero()
   self.gradInput:add(self.weight[1], gradOutput)
   return self.gradInput
end

function Mul:zeroGradParameters()
   self.gradWeight:zero()
end

function Mul:updateParameters(learningRate)
   self.weight:add(-learningRate, self.gradWeight)
end

function Mul:write(file)
   parent.write(self, file)
   file:writeObject(self.weight)
   file:writeObject(self.gradWeight)
end

function Mul:read(file)
   parent.read(self, file) 
   self.weight = file:readObject()
   self.gradWeight = file:readObject()
end
