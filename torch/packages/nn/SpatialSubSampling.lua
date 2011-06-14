local SpatialSubSampling, parent = torch.class('nn.SpatialSubSampling', 'nn.Module')

local help_desc =
[[Applies a 2D sub-sampling over an input image composed of
several input planes. The input tensor in forward(input) is 
expected to be a 3D tensor (width x height x nInputPlane). 
The number of output planes will be the same as nInputPlane.

Note that depending of the size of your kernel, several 
(of the last) columns or rows of the input image might be lost. 
It is up to the user to add proper padding in images.

If the input image is a 3D tensor width x height x nInputPlane, 
the output image size will be owidth x oheight x nInputPlane where

owidth  = (width  - kW) / dW + 1
oheight = (height - kH) / dH + 1 .

The parameters of the sub-sampling can be found in self.weight 
(Tensor of size nInputPlane) and self.bias (Tensor of size nInputPlane). 
The corresponding gradients can be found in self.gradWeight and self.gradBias.

The output value of the layer can be precisely described as:

output[i][j][k] = bias[k]
   + weight[k] sum_{s=1}^kW sum_{t=1}^kH input[dW*(i-1)+s][dH*(j-1)+t][k] ]]

function SpatialSubSampling:__init(nInputPlane, kW, kH, dW, dH)
   parent.__init(self)

   -- usage
   if not nInputPlane or not kW or not kH then
      error(toolBox.usage('nn.SpatialSubSampling', help_desc, nil,
                          {type='number', help='number of input planes', req=true},
                          {type='number', help='kernel width', req=true},
                          {type='number', help='kernel height', req=true},
                          {type='number', help='stride width'},
                          {type='number', help='stride height'}))
   end

   dW = dW or 1
   dH = dH or 1

   self.nInputPlane = nInputPlane
   self.kW = kW
   self.kH = kH
   self.dW = dW
   self.dH = dH

   self.weight = torch.Tensor(nInputPlane)
   self.bias = torch.Tensor(nInputPlane)
   self.gradWeight = torch.Tensor(nInputPlane)
   self.gradBias = torch.Tensor(nInputPlane)
   
   self:reset()
end

function SpatialSubSampling:reset(stdv)
   if stdv then
      stdv = stdv * math.sqrt(3)
   else
      stdv = 1/math.sqrt(self.kW*self.kH)
   end
   self.weight:apply(function()
                        return random.uniform(-stdv, stdv)
                     end)
   self.bias:apply(function()
                      return random.uniform(-stdv, stdv)
                   end)   
end

function SpatialSubSampling:zeroGradParameters()
   self.gradWeight:zero()
   self.gradBias:zero()
end

function SpatialSubSampling:updateParameters(learningRate)
   self.weight:add(-learningRate, self.gradWeight)
   self.bias:add(-learningRate, self.gradBias)
end

function SpatialSubSampling:decayParameters(decay)
   self.weight:add(-decay, self.weight)
   self.bias:add(-decay, self.bias)
end

function SpatialSubSampling:write(file)
   parent.write(self, file)
   file:writeInt(self.kW)
   file:writeInt(self.kH)
   file:writeInt(self.dW)
   file:writeInt(self.dH)
   file:writeInt(self.nInputPlane)
   file:writeObject(self.weight)
   file:writeObject(self.bias)
   file:writeObject(self.gradWeight)
   file:writeObject(self.gradBias)
end

function SpatialSubSampling:read(file)
   parent.read(self, file)
   self.kW = file:readInt()
   self.kH = file:readInt()
   self.dW = file:readInt()
   self.dH = file:readInt()
   self.nInputPlane = file:readInt()
   self.weight = file:readObject()
   self.bias = file:readObject()
   self.gradWeight = file:readObject()
   self.gradBias = file:readObject()
end
