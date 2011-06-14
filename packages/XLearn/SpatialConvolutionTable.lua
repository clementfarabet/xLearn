local SpatialConvolutionTable, parent = torch.class('nn.SpatialConvolutionTable', 'nn.Module')

local help_desc =
[[Applies a 2D convolution over an input image composed of 
several input planes. The input tensor in forward(input) 
is expected to be a 3D tensor (width x height x nInputPlane).

A table of connections is used to specify the topology of the 
layer. If a plain fully connected module is enough, 
nn.SpatialConvolution should be used. This table should be
a 2D tensor (2 x nb_kernels), where table[k][1] points to an
input, and table[k][2] points to an output.

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
gradients can be found in self.gradWeight and self.gradBias.]]

local help_example =
[[-- create a filter bank with 8 inputs, 32 outputs, and
-- random connections with a fanin of 4, filters are 9x9
stimulus = lab.randn(500,500,8)
randTable = nn.SpatialConvolutionTable:RandomTable(8,32,4)
mod = nn.SpatialConvolutionTable(randTable, 9, 9)
result = mod:forward(stimulus)]]

function SpatialConvolutionTable:FullTable(nin, nout)
   local ft = torch.Tensor(nin*nout,2)
   local p = 1
   for j=1,nout do
      for i=1,nin do
	 ft[p][1] = i
	 ft[p][2] = j
	 p = p + 1
      end
   end
   return ft
end

function SpatialConvolutionTable:OneToOneTable(nfeat)
   local ft = torch.Tensor(nfeat,2)
   for i=1,nfeat do
      ft[i][1] = i
      ft[i][2] = i
   end
   return ft
end

function SpatialConvolutionTable:RandomTable(nin, nout, nto)
   local nker = nto * nout
   local tbl = torch.Tensor(nker, 2)
   allcombi = toolBox.combinations(nin,nto)
   if allcombi:nElement() < nker then print("# WARNING: not enough combination possible, " .. 
                                            "redondancy not allowed") end
   local rdnIdx = lab.randperm(allcombi:size(1))
   local counter = 1
   for i=1,nout do
      combi = allcombi[rdnIdx[i]]
      for j=1,nto do
         tbl[counter][1] = combi[j]
         tbl[counter][2] = i
         counter = counter + 1
      end
   end
   return tbl
end

function SpatialConvolutionTable:KorayTable(nin, nout, nto)
   local nker = nto * nout
   local tbl = torch.Tensor(nker, 2)
   local fi = lab.randperm(nin)
   local frcntr = 1
   local tocntr = 1
   local nfi = math.floor(nin/nto) -- number of distinct nto chunks 
   local rfi = math.mod(nin,nto) -- number of remaining from maps
   local totbl = tbl:select(2,2)
   local frtbl = tbl:select(2,1)
   local fitbl = fi:narrow(1, 1, (nfi * nto)) -- part of fi that covers distinct chunks
   local ufrtbl= frtbl:unfold(1, nto, nto)
   local utotbl= totbl:unfold(1, nto, nto)
   local ufitbl= fitbl:unfold(1, nto, nto)
   
   -- start filling frtbl
   for i=1,nout do -- fro each unit in target map
      ufrtbl:select(1,i):copy(ufitbl:select(1,frcntr))
      frcntr = frcntr + 1
      if frcntr-1 ==  nfi then -- reset fi
	 fi:copy(lab.randperm(nin))
	 frcntr = 1
      end
   end
   for tocntr=1,utotbl:size(1) do
      utotbl:select(1,tocntr):fill(tocntr)
   end
   return tbl
end

function SpatialConvolutionTable:__init(conMatrix, kW, kH, dW, dH)
   parent.__init(self)

   -- usage
   if not conMatrix or not kW or not kH or type(conMatrix) ~= 'userdata' then
      error(toolBox.usage('nn.SpatialConvolutionTable', help_desc, help_example,
                          {type='torch.Tensor', help='a Nx2 array, N being the number of kernels',
                                                req=true},
                          {type='number', help='kernel width', req=true},
                          {type='number', help='kernel height', req=true},
                          {type='number', help='stride width'},
                          {type='number', help='stride height'}))
   end

   dW = dW or 1
   dH = dH or 1

   self.kW = kW
   self.kH = kH
   self.dW = dW
   self.dH = dH
   self.connTable = conMatrix
   self.nInputPlane = self.connTable:select(2,1):max()
   self.nOutputPlane = self.connTable:select(2,2):max()

   self.weight = torch.Tensor(kW, kH, self.connTable:size(1))
   self.bias = torch.Tensor(self.nOutputPlane)
   self.gradWeight = torch.Tensor(kW, kH, self.connTable:size(1))
   self.gradBias = torch.Tensor(self.nOutputPlane)
   
   self:reset()
end

function SpatialConvolutionTable:reset(stdv)
   if stdv then
      stdv = stdv * math.sqrt(3)
      self.weight:apply(function()
			   return random.uniform(-stdv, stdv)
			end)
      self.bias:apply(function()
			 return random.uniform(-stdv, stdv)
		      end)
   else
      local ninp = torch.Tensor(self.nOutputPlane):zero()
      for i=1,self.connTable:size(1) do ninp[self.connTable[i][2]] =  ninp[self.connTable[i][2]]+1 end
      for k=1,self.connTable:size(1) do
	 stdv = 1/math.sqrt(self.kW*self.kH*ninp[self.connTable[k][2]])
	 self.weight:select(3,k):apply(function() return random.uniform(-stdv,stdv) end)
      end
      for k=1,self.bias:size(1) do
	 stdv = 1/math.sqrt(self.kW*self.kH*ninp[k])
	 self.bias[k] = random.uniform(-stdv,stdv)
      end
   end
end

function SpatialConvolutionTable:zeroGradParameters()
   self.gradWeight:zero()
   self.gradBias:zero()
end

function SpatialConvolutionTable:updateParameters(learningRate)
   self.weight:add(-learningRate, self.gradWeight)
   self.bias:add(-learningRate, self.gradBias)
end

function SpatialConvolutionTable:decayParameters(decay)
   self.weight:add(-decay, self.weight)
   self.bias:add(-decay, self.bias)
end

function SpatialConvolutionTable:write(file)
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
   file:writeObject(self.connTable)
end

function SpatialConvolutionTable:read(file)
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
   self.connTable = file:readObject()
end
