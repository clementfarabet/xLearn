local TemporalConvolution, parent = torch.class('nn.TemporalConvolution', 'nn.Module')

local help_desc =
[[
Applies a 1D convolution over an input sequence composed 
of nInputFrame frames. The input tensor in forward(input) 
is expected to be a 2D tensor (inputFrameSize x nInputFrame).

Note that depending of the size of your kernel, several 
(of the last) frames of the sequence might be lost. 
It is up to the user to add proper padding frames in 
the input sequences.

If the input sequence is a 2D tensor inputFrameSize x nInputFrame,
the output sequence will be outputFrameSize x nOutputFrame where

nOutputFrame = (nInputFrame - kW) / dW + 1

The parameters of the convolution can be found in self.weight 
(Tensor of size inputFrameSize x kW x outputFrameSize) 
and self.bias (Tensor of size outputFrameSize). The 
corresponding gradients can be found in self.gradWeight 
and self.gradBias.

The output value of the layer can be precisely described as:

output[i][t] = bias[i]
  + sum_j sum_{k=1}^kW weight[j][k][i]
                                * input[j][dW*(t-1)+k] ]]

local help_example =
[[inp=5;  -- dimensionality of one sequence element 
outp=1; -- number of derived features for one sequence element
kw=1;   -- kernel only operates on one sequence element at once
dw=1;   -- we step once and go on to the next sequence element

mlp=nn.TemporalConvolution(inp,outp,kw,dw)

require "lab"
x=lab.rand(inp,7) -- a sequence of 7 elements
print(mlp:forward(x))

-- which gives:
-0.9109 -0.9872 -0.6808 -0.9403 -0.9680 -0.6901 -0.6387
[torch.Tensor of dimension 1x7]

-- This is equivalent to:
weights=lab.reshape(mlp.weight,inp) -- weights applied to all
bias= mlp.bias[1];
for i=1,x:size(2) do -- for each sequence element
 element= x:t()[i]; -- features of ith sequence element
 print(element:dot(weights) + bias)
end

-- which gives:
-0.91094998687717
-0.98721705771773
-0.68075004276185
-0.94030132495887
-0.96798754116609
-0.69008470895581
-0.63871422284166 ]]

function TemporalConvolution:__init(inputFrameSize, outputFrameSize, kW, dW)
   parent.__init(self)

   -- usage
   if not inputFrameSize or not outputFrameSize or not kW or not kH then
      error(toolBox.usage('nn.TemporalConvolution', help_desc, help_example,
                          {type='number', help='input frame size expected in sequences', req=true},
                          {type='number', help='output frame size produced', req=true},
                          {type='number', help='kernel width', req=true},
                          {type='number', help='stride width'}))
   end

   dW = dW or 1

   self.inputFrameSize = inputFrameSize
   self.outputFrameSize = outputFrameSize
   self.kW = kW
   self.dW = dW

   self.weight = torch.Tensor(inputFrameSize, kW, outputFrameSize)
   self.bias = torch.Tensor(outputFrameSize)
   self.gradWeight = torch.Tensor(inputFrameSize, kW, outputFrameSize)
   self.gradBias = torch.Tensor(outputFrameSize)
   
   self:reset()
end

function TemporalConvolution:reset(stdv)
   if stdv then
      stdv = stdv * math.sqrt(3)
   else
      stdv = 1/math.sqrt(self.kW*self.inputFrameSize)
   end
   self.weight:apply(function()
                        return random.uniform(-stdv, stdv)
                     end)
   self.bias:apply(function()
                      return random.uniform(-stdv, stdv)
                   end)   
end

function TemporalConvolution:zeroGradParameters()
   self.gradWeight:zero()
   self.gradBias:zero()
end

function TemporalConvolution:updateParameters(learningRate)
   self.weight:add(-learningRate, self.gradWeight)
   self.bias:add(-learningRate, self.gradBias)
end

function TemporalConvolution:decayParameters(decay)
   self.weight:add(-decay, self.weight)
   self.bias:add(-decay, self.bias)
end

function TemporalConvolution:write(file)
   parent.write(self, file)
   file:writeInt(self.kW)
   file:writeInt(self.dW)
   file:writeInt(self.inputFrameSize)
   file:writeInt(self.outputFrameSize)
   file:writeObject(self.weight)
   file:writeObject(self.bias)
   file:writeObject(self.gradWeight)
   file:writeObject(self.gradBias)
end

function TemporalConvolution:read(file)
   parent.read(self, file)
   self.kW = file:readInt()
   self.dW = file:readInt()
   self.inputFrameSize = file:readInt()
   self.outputFrameSize = file:readInt()
   self.weight = file:readObject()
   self.bias = file:readObject()
   self.gradWeight = file:readObject()
   self.gradBias = file:readObject()
end
