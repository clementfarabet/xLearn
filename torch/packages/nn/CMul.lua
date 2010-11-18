local CMul, parent = torch.class('nn.CMul', 'nn.Module')

------------------------------------------------------------
-- HELP
local help_desc = 
[[Applies a component-wise multiplication to the 
incoming data, i.e. y_i = w_i x_i.]]

local help_example = 
[[mlp=nn.Sequential()
mlp:add(nn.CMul(5))

y=torch.Tensor(5); 
sc=torch.Tensor(5); for i=1,5 do sc[i]=i; end -- scale input with this

function gradUpdate(mlp,x,y,criterion,learningRate) 
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
 y:copy(x); y:cmul(sc);
 err=gradUpdate(mlp,x,y,nn.MSECriterion(),0.01)
end
print(mlp:get(1).weight)

-- gives the output:
 1.0000
 2.0000
 3.0000
 4.0000
 5.0000
[torch.Tensor of dimension 5]

-- i.e. the network successfully learns the input x has
-- been scaled by those scaling factors to produce the output y.]]
-- HELP
------------------------------------------------------------

function CMul:__init(inputSize)
   parent.__init(self)

   -- usage
   if not inputSize then
      error(toolBox.usage('nn.CMul', help_desc, help_example,
                          {type='number', help='input dimension', req=true}))
   end

   self.weight = torch.Tensor(inputSize)
   self.gradWeight = torch.Tensor(inputSize)
   
   -- state
   self.gradInput:resize(inputSize)
   self.output:resize(inputSize) 

   self:reset()
end

 
function CMul:reset(stdv)
   self.weight:apply(function()
                                  return 1;
                        end)
end

function CMul:forward(input)
   self.output:copy(input);
   self.output:cmul(self.weight);
   return self.output
end

function CMul:backward(input, gradOutput)
   self.gradWeight:addcmul(1, input, gradOutput)
  
   self.gradInput:zero()
   self.gradInput:addcmul(1, self.weight, gradOutput)
   return self.gradInput
end

function CMul:zeroGradParameters()
   self.gradWeight:zero()
end

function CMul:updateParameters(learningRate)
   self.weight:add(-learningRate, self.gradWeight)
end

function CMul:write(file)
   parent.write(self, file)
   file:writeObject(self.weight)
   file:writeObject(self.gradWeight)
end

function CMul:read(file)
   parent.read(self, file) 
   self.weight = file:readObject()
   self.gradWeight = file:readObject()
end
