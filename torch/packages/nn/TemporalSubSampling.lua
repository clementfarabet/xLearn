local TemporalSubSampling, parent = torch.class('nn.TemporalSubSampling', 'nn.Module')

local help_desc =
[[Applies a 1D sub-sampling over an input sequence composed 
of nInputFrame frames. The input tensor in forward(input) 
is expected to be a 2D tensor (inputFrameSize x nInputFrame). 
The output frame size will be the same as the input one 
(inputFrameSize).

Note that depending of the size of your kernel, several 
(of the last) frames of the sequence might be lost. 
It is up to the user to add proper padding frames in 
the input sequences.

If the input sequence is a 2D tensor inputFrameSize x nInputFrame, 
the output sequence will be inputFrameSize x nOutputFrame where

nOutputFrame = (nInputFrame - kW) / dW + 1

The parameters of the sub-sampling can be found in self.weight
(Tensor of size inputFrameSize) and self.bias 
(Tensor of size inputFrameSize). The corresponding gradients 
can be found in self.gradWeight and self.gradBias.

The output value of the layer can be precisely described as:

output[i][t] = bias[i] + weight[i] * sum_{k=1}^kW input[i][dW*(t-1)+k] ]]

function TemporalSubSampling:__init(inputFrameSize, kW, dW)
   parent.__init(self)

   -- usage
   if not inputFrameSize or not kW or not kH then
      error(toolBox.usage('nn.TemporalSubSampling', help_desc, help_example,
                          {type='number', help='input frame size expected in sequences', req=true},
                          {type='number', help='kernel width', req=true},
                          {type='number', help='stride width'}))
   end

   dW = dW or 1

   self.inputFrameSize = inputFrameSize
   self.kW = kW
   self.dW = dW

   self.weight = torch.Tensor(inputFrameSize)
   self.bias = torch.Tensor(inputFrameSize)
   self.gradWeight = torch.Tensor(inputFrameSize)
   self.gradBias = torch.Tensor(inputFrameSize)
   
   self:reset()
end

function TemporalSubSampling:reset(stdv)
   if stdv then
      stdv = stdv * math.sqrt(3)
   else
      stdv = 1/math.sqrt(self.kW)
   end
   self.weight:apply(function()
                        return random.uniform(-stdv, stdv)
                     end)
   self.bias:apply(function()
                      return random.uniform(-stdv, stdv)
                   end)   
end

function TemporalSubSampling:zeroGradParameters()
   self.gradWeight:zero()
   self.gradBias:zero()
end

function TemporalSubSampling:updateParameters(learningRate)
   self.weight:add(-learningRate, self.gradWeight)
   self.bias:add(-learningRate, self.gradBias)
end

function TemporalSubSampling:write(file)
   parent.write(self, file)
   file:writeInt(self.kW)
   file:writeInt(self.dW)
   file:writeInt(self.inputFrameSize)
   file:writeObject(self.weight)
   file:writeObject(self.bias)
   file:writeObject(self.gradWeight)
   file:writeObject(self.gradBias)
end

function TemporalSubSampling:read(file)
   parent.read(self, file)
   self.kW = file:readInt()
   self.dW = file:readInt()
   self.inputFrameSize = file:readInt()
   self.weight = file:readObject()
   self.bias = file:readObject()
   self.gradWeight = file:readObject()
   self.gradBias = file:readObject()
end
