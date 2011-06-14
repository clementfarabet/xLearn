require 'opencl'
local CLSpatialConvolution, parent = torch.class('nn.CLSpatialConvolution', 'nn.Module')

local help_desc =
   [[Withnin a CLcontext (using GPU)
Applies a 2D convolution over an input image composed of 
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
TODO]]

function CLSpatialConvolution:__init(...)--(nInputPlane, nOutputPlane, kW, kH, dW, dH)
   parent.__init(self)
   toolBox.unpack_class(
      self,
      {...},
      'nn.CLSpatialConvolution', 
      help_desc,
      {arg='context', type='opencl.Context', help='opencl context'},
      {arg='nInputPlane', type='number', help='number of input planes', req=true},
      {arg='nOutputPlane', type='number', help='number of outputs planes', req=true},
      {arg='kW', type='number', help='kernel width', req=true},
      {arg='kH', type='number', help='kernel height', req=true},
      {arg='dW', type='number', help='stride width', default=1},
      {arg='dH', type='number', help='stride height', default=1}
   )
   if not self.context then
      print('warning: <CLSpatialConvolution> constructor with no context, new one is created...')
      self.context = opencl.getContext{}
   end
   if self.dW ~= 1 then
      xerror('stride ~=1 Not Implemented','nn.CLSpatialConvolution')
   end
   if self.dH ~= 1 then
      xerror('stride ~=1 Not Implemented','nn.CLSpatialConvolution')
   end

   self.weight = opencl.Tensor(self.context, self.kW, self.kH, 
                               self.nInputPlane, self.nOutputPlane)
   self.gradWeight = opencl.Tensor(self.context, self.kW, self.kH, 
                                   self.nInputPlane, self.nOutputPlane)
   self.bias = opencl.Tensor(self.context, self.nOutputPlane)
   self.gradBias =  opencl.Tensor(self.context, self.nOutputPlane)
   
   self:reset()
end


function CLSpatialConvolution:reset(stdv)
   if stdv then
      stdv = stdv * math.sqrt(3)
   else
      stdv = 1/math.sqrt(self.kW*self.kH*self.nInputPlane)
   end
   local weight = torch.Tensor(self.kW, self.kH, self.nInputPlane, self.nOutputPlane)
   local bias = torch.Tensor(self.nOutputPlane)
   weight:apply(function()
                        return random.uniform(-stdv, stdv)
                     end)
   self.weight:copy(weight)
   bias:apply(function()
                      return random.uniform(-stdv, stdv)
                   end)  
   self.bias:copy(bias) 
end

function CLSpatialConvolution:zeroGradParameters()
   self.gradWeight:zero()
   self.gradBias:zero()
end

function CLSpatialConvolution:updateParameters(learningRate)
   self.weight:add(-learningRate, self.gradWeight)
   self.bias:add(-learningRate, self.gradBias)
end

function CLSpatialConvolution:decayParameters(decay)
   self.weight:add(-decay, self.weight)
   self.bias:add(-decay, self.bias)
end

function CLSpatialConvolution:makeKernel()
   local tmpl =[[
         __kernel void convo(__global float *input,
                             __global float *weights,
                             __global float *output)
         {
            unsigned gz = get_global_id(2);
            unsigned tx = get_local_id(0); 
            unsigned ty = get_local_id(1);
            unsigned gx = get_global_id(0);
            unsigned gy = get_global_id(1);
            // -- starting input data for the current block
            input += gy*$inW + gx;
            weights += gz*$nInputPlane*$kH*$kW;
            // -- FIXME last dim should be multiple of 16
            __local float shared_in[$SH_IN][$SW_IN];

            float sum = 0;
            
            for(unsigned x = 0; x < $nInputPlane; ++x)
            {
               // load into local memory --FIXME UNROLL this
               for(unsigned sy=0; sy<$SH_IN; sy += $BLOCK_H)
               for(unsigned sx=0; sx<$SW_IN; sx += $BLOCK_W) 
               {
                  unsigned tsx = sx + tx;
                  unsigned tsy = sy + ty;
                  if (tsx < $SW_IN && tsy < $SH_IN
                      && (sx + gx) < $inW  && (sy + gy) < $inH)
                     shared_in[tsy][tsx] = input[sy*$inW + sx];
               }
               // -- synchronize memory load within the block
               barrier(CLK_LOCAL_MEM_FENCE);

               // compute the actual convo
               for(unsigned kh=0; kh< $kH; ++kh)
               for(unsigned kw=0; kw< $kW; ++kw)
                  sum += shared_in[ty+kh][tx+kw] * weights[kh*$kW + kw];

               // next  feature map
               input += $inH*$inW;
               // next filter
               weights += $kH*$kW;
            }
            // output only the good ones
            if (gx < $outW && gy < $outH)
               output[((gz*$outH) + gy)*$outW + gx] = sum;
         }
   ]]
   local lookup= {
      BLOCK_H = opencl.BLOCK_H,
      BLOCK_W = opencl.BLOCK_W,
      SH_IN = math.min(self.input:size(2),opencl.BLOCK_H + self.kH - 1),
      SW_IN = math.min(self.input:size(1),opencl.BLOCK_W + self.kW - 1),
      nOutputPlane = self.nOutputPlane,
      nInputPlane = self.nInputPlane,
      outH = self.output:size(2),
      outW = self.output:size(1),
      kH = self.kH,
      kW = self.kW,
      inH = self.input:size(2),
      inW = self.input:size(1),
   }
   local types = {'float*','float*','float*','float'}
   local args ={'in','w','out'}
--    require 'cosmo'
--    print('source',cosmo.fill(tmpl,lookup))
   self._ker = opencl.Kernel{context = self.context,
                             name = 'convo',
                             template = tmpl,
                             lookup = lookup,
                             types = types,
                             args = args}
   self._run = self._ker:makeClosure()

end

function CLSpatialConvolution:forward(input)
   if torch.typename(input) ~= 'opencl.Tensor' then
      xerror('<CLSpatialConvolution:forward> input must be opencl.Tensor.')
   end
   -- STOPPED HERE
   local outW,outH
   outW = (input:size(1) - self.kW) / self.dW + 1
   outH = (input:size(2) - self.kH) / self.dH + 1
   --print('output',outW, outH, self.nOutputPlane)
   self.output = opencl.Tensor(self.context, outW, outH, self.nOutputPlane)
   self.input = input
   -- build the kernel
   self:makeKernel()
   -- Set and log Global and Local work size dimensions
   local ls = torch.LongTensor(3):fill(1)
   ls[1] = opencl.BLOCK_W
   ls[2] = opencl.BLOCK_H
   local gs = torch.LongTensor(3):fill(1)
   gs[1] = math.ceil(outW/ls[1])*ls[1]
   gs[2] = math.ceil(outH/ls[2])*ls[2]
   gs[3] = self.nOutputPlane
   --print('local grid:',ls[1],ls[2],'global grid:',gs[1],gs[2],gs[3])
   self._run(gs,ls,{self.input.data,self.weight.data,self.output.data})
   return self.output
end

function CLSpatialConvolution:backward(input, gradOuput)
   if torch.typename(input) ~= 'opencl.Tensor' then
      xerror('<CLSpatialConvolution:forward> input must be opencl.Tensor.')
   end
   if torch.typename(gradOuput) ~= 'opencl.Tensor' then
      xerror('<CLSpatialConvolution:forward> gradOuput must be opencl.Tensor.')
   end

end

function CLSpatialConvolution:write(file)
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

function CLSpatialConvolution:read(file)
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
