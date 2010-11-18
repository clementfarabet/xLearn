local SpatialConvolution, parent = torch.class('nn.SpatialConvolution', 'nn.Module')

local help_desc =
[[Applies a 2D convolution over an input image composed of 
several input planes. The input tensor in forward(input) 
is expected to be a 3D tensor (width x height x nInputPlane).

Note that depending of the size of your kernel, several 
(of the last) columns or rows of the input image might be lost. 
It is up to the user to add proper padding in images.

If the input image is a 3D tensor width x height x nInputPlane, 
the output image size will be owidth x oheight x nOutputPlane where

owidth  = (width  - kW) / dW + 1
oheight = (height - kH) / dH + 1 .

The parameters of the convolution can be found in self.weight 
(Tensor of size kH x kW x nInputPlane x nOutputPlane) and 
self.bias (Tensor of size nOutputPlane). The corresponding 
gradients can be found in self.gradWeight and self.gradBias.

The output value of the layer can be precisely described as:

output[i][j][k] = bias[k]
  + sum_l sum_{s=1}^kW sum_{t=1}^kH weight[s][t][l][k]
                     * input[dW*(i-1)+s)][dH*(j-1)+t][l] ]]

local help_example =
[[-- create a filter bank with 8 inputs, 32 outputs, with
-- 11x11 filters (this will generate 8x32 filters)
stimulus = lab.randn(500,500,8)
mod = nn.SpatialConvolutionTable(8, 32, 9, 9)
result = mod:forward(stimulus)]]

function SpatialConvolution:__init(nInputPlane, nOutputPlane, kW, kH, dW, dH)
   parent.__init(self)

   -- usage
   if not nInputPlane or not nOutputPlane or not kW or not kH then
      error(toolBox.usage('nn.SpatialConvolution', help_desc, help_example,
                          {type='number', help='number of input planes', req=true},
                          {type='number', help='number of outputs planes', req=true},
                          {type='number', help='kernel width', req=true},
                          {type='number', help='kernel height', req=true},
                          {type='number', help='stride width'},
                          {type='number', help='stride height'}))
   end

   dW = dW or 1
   dH = dH or 1

   self.nInputPlane = nInputPlane
   self.nOutputPlane = nOutputPlane
   self.kW = kW
   self.kH = kH
   self.dW = dW
   self.dH = dH

   self.weight = torch.Tensor(kW, kH, nInputPlane, nOutputPlane)
   self.bias = torch.Tensor(nOutputPlane)
   self.gradWeight = torch.Tensor(kW, kH, nInputPlane, nOutputPlane)
   self.gradBias = torch.Tensor(nOutputPlane)
   
   self:reset()
end

function SpatialConvolution:reset(stdv)
   if stdv then
      stdv = stdv * math.sqrt(3)
   else
      stdv = 1/math.sqrt(self.kW*self.kH*self.nInputPlane)
   end
   self.weight:apply(function()
                        return random.uniform(-stdv, stdv)
                     end)
   self.bias:apply(function()
                      return random.uniform(-stdv, stdv)
                   end)   
end

function SpatialConvolution:zeroGradParameters()
   self.gradWeight:zero()
   self.gradBias:zero()
end

function SpatialConvolution:updateParameters(learningRate)
   self.weight:add(-learningRate, self.gradWeight)
   self.bias:add(-learningRate, self.gradBias)
end

function SpatialConvolution:write(file)
   parent.write(self, file)
   file:writeInt(self.kW)
   file:writeInt(self.kH)
   file:writeInt(self.dW)
   file:writeInt(self.dH)
   file:writeInt(self.nInputPlane)
   file:writeInt(self.nOutputPlane)
   file:writeObject(self.weight)
   file:writeObject(self.bias)
   file:writeObject(self.gradWeight)
   file:writeObject(self.gradBias)
end

function SpatialConvolution:read(file)
   parent.read(self, file)
   self.kW = file:readInt()
   self.kH = file:readInt()
   self.dW = file:readInt()
   self.dH = file:readInt()
   self.nInputPlane = file:readInt()
   self.nOutputPlane = file:readInt()
   self.weight = file:readObject()
   self.bias = file:readObject()
   self.gradWeight = file:readObject()
   self.gradBias = file:readObject()
end
