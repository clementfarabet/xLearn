local SparseLinear, parent = torch.class('nn.SparseLinear', 'nn.Module')

local help_desc =
[[Applies a linear transformation to the incoming sparse data,
i.e. y= Ax+b. The input tensor given in forward(input) must be 
a sparse vector represented as 2D tensor of the form torch.Tensor(N, 2)
where the pairs represent indices and values. The SparseLinear layer 
is useful when the number of input dimensions is very large and the 
input data is sparse.]]

local help_example =
[[-- You can create a sparse linear layer in the following way:
module= nn.SparseLinear(10000,2)  -- 10000 inputs, 2 outputs

-- The sparse linear module may be used as part of a 
-- larger network, and apart from the form of the input, 
-- SparseLinear operates in exactly the same way as the Linear layer.

-- A sparse input vector may be created as so..
x=lab.new({1, 0.1},{2, 0.3},{10, 0.3},{31, 0.2})

print(x)

  1.0000   0.1000
  2.0000   0.3000
 10.0000   0.3000
 31.0000   0.2000
[torch.Tensor of dimension 4x2]

-- The first column contains indices, the second column 
-- contains values in a a vector where all other elements 
-- are zeros. The indices should not exceed the stated dimensions 
-- of the input to the layer (10000 in the example). ]]

function SparseLinear:__init(inputSize, outputSize)
   parent.__init(self)

   -- usage
   if not inputSize or not outputSize then
      error(toolBox.usage('nn.SparseLinear', help_desc, help_example,
                          {type='number', help='input dimension', req=true},
                          {type='number', help='output dimension', req=true}))
   end

   self.weightDecay = 0
   self.weight = torch.Tensor(outputSize, inputSize)
   self.bias = torch.Tensor(outputSize)
   self.gradWeight = torch.Tensor(outputSize, inputSize)
   self.gradBias = torch.Tensor(outputSize)
   self.lastInput = torch.Tensor()
   -- state
   self.gradInput:resize(inputSize)
   self.output:resize(outputSize)

   self:reset()
end


function SparseLinear:reset(stdv)
   if stdv then
      stdv = stdv * math.sqrt(3)
   else
      stdv = 1./math.sqrt(self.weight:size(1))
   end

   -- we do this so the initialization is exactly
   -- the same than in previous torch versions
   for i=1,self.weight:size(1) do
      self.weight:select(1, i):apply(function()
                                        return random.uniform(-stdv, stdv)
                                     end)
      self.bias[i] = random.uniform(-stdv, stdv) * 0.000001
   end
end

function SparseLinear:zeroGradParameters()
   --self.gradWeight:zero()
   self.gradBias:zero()
end

function SparseLinear:write(file)
   parent.write(self, file)
   file:writeDouble(self.weightDecay)
   file:writeObject(self.weight)
   file:writeObject(self.bias)
   file:writeObject(self.gradWeight)
   file:writeObject(self.gradBias)
end

function SparseLinear:read(file)
   parent.read(self, file)
   self.weightDecay = file:readDouble()
   self.weight = file:readObject()
   self.bias = file:readObject()
   self.gradWeight = file:readObject()
   self.gradBias = file:readObject()

   self.lastInput = torch.Tensor()
   self.gradInput = torch.Tensor()
   self.output = torch.Tensor()
   self.output:resizeAs(self.bias)
end
