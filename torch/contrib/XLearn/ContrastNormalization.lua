local ContrastNormalization, parent = torch.class('nn.ContrastNormalization','nn.Module')

local help_desc = 
[[a spatial (2D) contrast normalizer
? computes the local mean and local std deviation
  across all input features, using the given 2D kernel
? the local mean is then removed from all maps, and the std dev
  used to divide the inputs, with a threshold
? if no threshold is given, the global std dev is used
? zero-padding is used to preserve sizes (this is
  faster than weight-replication, but has strong side effects,
  nn.LocalNorm to use weight-replication)]]

local help_example = 
[[-- create a contrast normalizer, with a 9x9 gaussian kernel
-- works on 8 input feature maps, therefore the mean+dev will
-- be estimated on 9x9x8 cubes
stimulus = lab.randn(500,500,8)
gaussian = image.gaussian{width=9}
mod = nn.ContrastNormalization(gaussian, 8)
result = mod:forward(stimulus)]]

function ContrastNormalization:__init(ker,nf,thres)
   parent.__init(self)
   self.kernel = ker
   self.nfeatures = nf
   self.fixedThres = thres -- optional, if not provided, the global std is used

   if not ker or not nf or type(ker) ~= 'userdata' then
      error(toolBox.usage('nn.ContrastNormalization',
                          help_desc,
                          help_example,
                          {type='torch.Tensor', help='a KxK filtering kernel', req=true},
                          {type='number', help='number of input maps', req=true},
                          {type='number', help='threshold, for division'}))
   end

   local ct = nn.SpatialConvolutionTable:OneToOneTable(nf)
   -- create all little operations
   -- mean operations
   local padw = math.floor(ker:size(1)/2)
   local padh = math.floor(ker:size(2)/2)
   self.meanmod = nn.Sequential()
   self.meanmod:add(nn.SpatialPadding(padw,padw,padh,padh))
   self.meanmod:add(nn.SpatialConvolutionTable(ct,ker:size(1),ker:size(2),1,1))
   self.meanmod:add(nn.Sum(3))
   self.meanmod:add(nn.Replicate(nf))
   local mkw = self.meanmod.modules[2].weight
   for i=1,mkw:size(3) do mkw:select(3,i):copy(ker) end
   mkw:div(mkw:sum())
   self.meanmod.modules[2].bias:zero()
   -- variance operations
   self.varmod = nn.Sequential()
   self.varmod:add(nn.SpatialPadding(padw,padw,padh,padh))
   self.varmod:add(nn.SpatialConvolutionTable(ct,ker:size(1),ker:size(2),1,1))
   self.varmod:add(nn.Sum(3))
   self.varmod:add(nn.Replicate(nf))
   local vkw = self.varmod.modules[2].weight
   for i=1,vkw:size(3) do vkw:select(3,i):copy(ker) end
   vkw:div(vkw:sum())
   self.varmod.modules[2].bias:zero()
   -- all other operations
   self.squaremod = nn.Square()
   self.sqrtmod = nn.Sqrt()
   self.difmod = nn.CSubTable()
   self.divmod = nn.CDivTable()
   self.thresmod = nn.Threshold()
   -- some states that are handy to have variables for
   self.inmean = torch.Tensor()
   self.inzmean = torch.Tensor()
   self.inzmeansq = torch.Tensor()
   self.invar = torch.Tensor()
   self.instd = torch.Tensor()
   self.thstd = torch.Tensor()
end

function ContrastNormalization:forward(input)
   self.inmean = self.meanmod:forward(input)

   self.inzmean = self.difmod:forward({input,self.inmean})
   self.inzmeansq = self.squaremod:forward(self.inzmean)
   self.invar = self.varmod:forward(self.inzmeansq)
   self.instd = self.sqrtmod:forward(self.invar)

   -- this has to be set to a constant value (like 0.5) for jacobian
   -- testing, otherwise, every fprop with a perturbed input is
   -- actually a different function and finite difference
   -- approximation fails
   local meanstd = self.instd:mean()
   self.thresmod.threshold = self.fixedThres or meanstd
   self.thresmod.val = self.fixedThres or meanstd
   self.thstd = self.thresmod:forward(self.instd)

   self.output = self.divmod:forward({self.inzmean,self.thstd})

   return self.output
end

function ContrastNormalization:backward(input, gradOutput)
   self.gradInput:resizeAs(input):zero()

   local gdivmod = self.divmod:backward({self.inzmean,self.thstd},gradOutput)
   local gthstd = gdivmod[2]
   local ginzmean = gdivmod[1]
   local ginstd = self.thresmod:backward(self.instd,gthstd)
   local ginvar = self.sqrtmod:backward(self.invar,ginstd)
   local ginzmeansq = self.varmod:backward(self.inzmeansq,ginvar)
   ginzmean:add(self.squaremod:backward(self.inzmean,ginzmeansq))
   local gdifmod = self.difmod:backward({input,self.inmean},ginzmean)
   local ginmean = gdifmod[2]
   self.gradInput:add(gdifmod[1])
   self.gradInput:add(self.meanmod:backward(input,ginmean))
   return self.gradInput
end

function ContrastNormalization:updateParameters(learningRate)
   -- we do not update any weights
end

function ContrastNormalization:zeroGradParameters()
   self.meanmod:zeroGradParameters()
   self.difmod:zeroGradParameters()
   self.squaremod:zeroGradParameters()
   self.sqrtmod:zeroGradParameters()
   self.varmod:zeroGradParameters()
   self.thresmod:zeroGradParameters()
   self.divmod:zeroGradParameters()
end

function ContrastNormalization:write(file)
   parent.write(self,file)
   file:writeObject(self.kernel)
   file:writeInt(self.nfeatures)
   file:writeObject(self.meanmod)
   file:writeObject(self.varmod)
   file:writeObject(self.squaremod)
   file:writeObject(self.sqrtmod)
   file:writeObject(self.difmod)
   file:writeObject(self.divmod)
   file:writeObject(self.thresmod)
   file:writeObject(self.inmean)
   file:writeObject(self.inzmean)
   file:writeObject(self.inzmeansq)
   file:writeObject(self.invar)
   file:writeObject(self.instd)
   file:writeObject(self.thstd)
end

function ContrastNormalization:read(file)
   parent.read(self,file)
   self.kernel = file:readObject()
   self.nfeatures = file:readInt()
   self.meanmod = file:readObject()
   self.varmod = file:readObject()
   self.squaremod = file:readObject()
   self.sqrtmod = file:readObject()
   self.difmod = file:readObject()
   self.divmod = file:readObject()
   self.thresmod = file:readObject()
   self.inmean = file:readObject()
   self.inzmean = file:readObject()
   self.inzmeansq = file:readObject()
   self.invar = file:readObject()
   self.instd = file:readObject()
   self.thstd = file:readObject()
end
