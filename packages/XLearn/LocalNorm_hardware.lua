local LocalNorm_hardware, parent = torch.class('nn.LocalNorm_hardware','nn.Module')

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

local num = {}
do
   num.one = 256
   num.res = 1/256
   num.size_ = 16
   num.frac_ = 8
   num.max = (2^(num.size_-1)-1) / 2^num.frac_
end

-- This is the same function we use in compiler to normalize
-- the kernel before sending to hardware
function LocalNorm_hardware:normKernel(kernel, sum)
   sum = sum or 1
   kernel:div(kernel:sum()):mul(sum)
   local m = 0
   local n = 0
   local switch = false
   local inc
   while true do
      kernel:mul(num.one):add(0.5):floor():div(num.one)
      if math.abs(kernel:sum()/sum-1) < num.res then break
      elseif kernel:sum()/sum > 1 then inc = -num.res
      else inc = num.res end
      kernel:narrow(1,1+m,1):narrow(2,1+n,1):add(inc)
      if switch then
         n = (n + 1) % kernel:size(2)
      else
         m = (m + 1) % kernel:size(1)
      end
      if m == 0 then
         switch = true
      elseif n == 0 and switch then
         switch = false
      end
   end
end


function LocalNorm_hardware:__init(ker,nf,thres,smart)

   -- needs neuFlow
   require 'NeuFlow'

   parent.__init(self)
   self.kernel = ker
   self.kernel:div(self.kernel:sum())
   -- using the same function as on hardware to transform the kernel
   self:normKernel(self.kernel)
   
   -- smart padding
   self.smartPad = smart or false
   self.coef = torch.Tensor(1,1)


   local ctable = nn.SpatialConvolutionTable:OneToOneTable(nf)
   self.padW = math.floor(ker:size(1)/2)
   self.padH = math.floor(ker:size(2)/2)
   
      
   -- create convolution for computing the mean
   self.convo = nn.Sequential()
   self.convo:add(nn.SpatialPadding(self.padW,self.padW,
				    self.padH,self.padH))
   
   self.convo:add(nn.SpatialConvolutionTable(ctable,ker:size(1),ker:size(2)))
   self.convo:add(nn.Sum(3))
   self.convo:add(nn.Replicate(nf))
   -- set kernel
   local fb = self.convo.modules[2].weight
   for i=1,fb:size(3) do fb:select(3,i):copy(self.kernel) end
   -- set bias to 0
   self.convo.modules[2].bias:zero()
      
   -- scaling to fixed point
   self.kernel:mul(256)
   self.kernel:add(0.5)
   self.kernel:floor()
   self.kernel:div(256)

   
   self.nfeatures = nf
   self.fixedThres = thres -- optional, if not provided, the global std is used

   if not ker or not nf or type(ker) ~= 'userdata' then
      error(toolBox.usage('nn.LocalNorm_hardware_back',
                          help_desc,
                          help_example,
                          {type='torch.Tensor', help='a KxK filtering kernel', req=true},
                          {type='number', help='number of input maps', req=true},
                          {type='number', help='threshold, for division'}))
   end

   -- create all little operations
   -- mean operations
   local padw = math.floor(ker:size(1)/2)
   local padh = math.floor(ker:size(2)/2)
   -- Padding to make the output from convolution the same size as the input
   self.meanmod = nn.Sequential()
   self.meanmod:add(nn.SpatialPadding(padw,padw,padh,padh))
   
   -- Sum to imitate conv_and_acc
   self.summod = nn.Sequential()
   self.summod:add(nn.Sum(3))
   self.summod:add(nn.Replicate(nf))

   -- variance operations
   -- Padding to make the output from convolution the same size as the input
   self.varmod = nn.Sequential()
   self.varmod:add(nn.SpatialPadding(padw,padw,padh,padh))

   -- all other operations
   self.squaremod = nn.Square()
   self.difmod = nn.CSubTable()
   self.divmod = nn.CDivTable()
   self.meanDiviseMod = nn.CDivTable()
   self.stdDiviseMod = nn.CDivTable()
   tan = nn.Tanh()


   self.squareMod = nn.Square()
   self.sqrtMod = nn.Sqrt()
   self.substractMod = nn.CSubTable()
   self.thresMod = nn.Threshold()
   self.diviseMod = nn.CDivTable()
   
   -- create convolution for computing the meanstd
   self.convostd = nn.Sequential()
   self.convostd:add(nn.SpatialPadding(self.padW,self.padW,
                                      self.padH,self.padH))
   self.convostd:add(nn.SpatialConvolutionTable(ctable,ker:size(1),ker:size(2)))
   self.convostd:add(nn.Sum(3))
   self.convostd:add(nn.Replicate(nf))
   -- set kernel
   local fb = self.convostd.modules[2].weight
   for i=1,fb:size(3) do fb:select(3,i):copy(self.kernel) end
   -- set bias to 0
   self.convostd.modules[2].bias:zero()

   -- some states that are handy to have variables for
   self.infixed = torch.Tensor()
   self.paddedin = torch.Tensor()
   self.inmean = torch.Tensor()
   self.inmeansc = torch.Tensor()
   self.inzmean = torch.Tensor()
   self.inzmeansq = torch.Tensor()
   self.invar = torch.Tensor()
   self.insqrt = torch.Tensor()
   self.sign = torch.Tensor()
   self.copy_inmean = torch.Tensor()
   self.copy_insqrt = torch.Tensor()


   -- these states are for software model (to be used in backward)
   self.refConvo = torch.Tensor()
   self.refMean = torch.Tensor()
   self.refZeroMean = torch.Tensor()
   self.refZeroMeanSq = torch.Tensor()
   self.refConvoVar = torch.Tensor()
   self.refVar = torch.Tensor()
   self.refStdDev = torch.Tensor()
   self.refthstd = torch.Tensor()
   self.refRes = torch.Tensor()


   -- saving coefs 
   
   -- generate coefs for scaler
   -- (dividing by num of features)
   local xN = function (x) 
		 return x / self.nfeatures
	      end
   self.xN_coefs =  math.approx_line{mapping=xN, min=num.min, 
				      max=num.max, odd = true,
				      nbSegments=8, Q=num.frac_,
				      verbose=false, a = 1/self.nfeatures, b = 0}
   self.xN_segments_array = torch.Tensor(3, #self.xN_coefs)
   
   -- put segments into array
   for i = 1,#self.xN_coefs do
      self.xN_segments_array[1][i] = self.xN_coefs[i].min
      self.xN_segments_array[2][i] = self.xN_coefs[i].a
      self.xN_segments_array[3][i] = self.xN_coefs[i].b
   end
      
   threshold = self.fixedThres or 1/256
   -- generate coefs for sqrt - no padding
   local mapping_no_pad
   mapping_no_pad = function (x) 
		       x = x / self.nfeatures
		       if x < threshold then return math.sqrt(threshold)
		       else return math.sqrt(x) end
		    end
   local name_sqrt_coefs_no_pad = 'Sqrt_th_div_'..self.nfeatures..'_s_'..threshold
   
   self.sqrtCoefs_no_pad = math.approx{mapping=mapping_no_pad, min=0, max=num.max,
				       nbSegments=8, Q=num.frac_,
				       verbose=false, epsilon=23.9/256,error_type = 0,
				       name = name_sqrt_coefs_no_pad}
   
   self.segments_array_no_pad = torch.Tensor(3, #self.sqrtCoefs_no_pad)
   
   -- put segments into array
   for i = 1,#self.sqrtCoefs_no_pad do
      self.segments_array_no_pad[1][i] = self.sqrtCoefs_no_pad[i].min
      self.segments_array_no_pad[2][i] = self.sqrtCoefs_no_pad[i].a
      self.segments_array_no_pad[3][i] = self.sqrtCoefs_no_pad[i].b
   end
   

   -- generate coefs for sqrt - with padding
   local mapping_pad
   mapping_pad = function (x) 
		    if x < threshold then return math.sqrt(threshold)
		    else return math.sqrt(x) end
		 end
   local name_sqrt_coefs_pad = 'Sqrt_th_s_'..threshold
   
   self.sqrtCoefs_pad = math.approx{mapping=mapping_pad, min=0, max=num.max,
				    nbSegments=8, Q=num.frac_,
				    verbose=false, epsilon=23.9/256,error_type = 0,
				    name = name_sqrt_coefs_pad}
   
   self.segments_array_pad = torch.Tensor(3, #self.sqrtCoefs_pad)
   
   -- put segments into array
   for i = 1,#self.sqrtCoefs_pad do
      self.segments_array_pad[1][i] = self.sqrtCoefs_pad[i].min
      self.segments_array_pad[2][i] = self.sqrtCoefs_pad[i].a
      self.segments_array_pad[3][i] = self.sqrtCoefs_pad[i].b
   end
   
end


function LocalNorm_hardware:forward(input)
   -- init coef if necessary
   if (input:size(1) ~= self.coef:size(1)) or (input:size(2) ~= self.coef:size(2)) then
      -- recompute coef only if necessary
      local intVals = torch.Tensor(input:size(1),input:size(2),self.nfeatures):fill(1)
      self.coef = self.convo:forward(intVals)
      self.coef = torch.Tensor():resizeAs(self.convo.output):copy(self.convo.output)

   end
   
   
   self.input = input
   if (input:nDimension() == 2) then
      self.input = torch.Tensor(input:size(1),input:size(2),1)
      self.input:copy(input)
   end
   
   self.infixed:resizeAs(self.input):copy(self.input)
   
   -- scaling to fixed point
   self.infixed:mul(256)
   self.infixed:add(0.5)
   self.infixed:floor()
   self.infixed:div(256)
   
   
   self.paddedin = self.meanmod:forward(self.infixed)
   self.inmean:resize(self.paddedin:size(1) - self.kernel:size(1) + 1, 
		      self.paddedin:size(2) - self.kernel:size(2) + 1, self.nfeatures)
   
   -- order of args is:    (output,      input,         kernel)
   libxlearn.convFixedPoint(self.inmean, self.paddedin, self.kernel)
   
   -- scale to fixed point
   self.inmean:mul(256)
   self.inmean:add(0.5)
   self.inmean:floor()
   self.inmean:div(256)
   
   -- sum the maps
   self.inmean = self.summod:forward(self.inmean)
   self.inmean = torch.Tensor():resizeAs(self.inmean):copy(self.inmean)

  

   -- if smartPad we divide by self.coef matrix which has 2 purposes:
   -- 1) takes care of the boundaries of the image after convolution
   -- 2) average the sum of maps after convolution
   if self.smartPad then
      self.copy_inmean:resizeAs(self.inmean):copy(self.inmean)
      self.meanDiviseMod:forward({self.copy_inmean,self.coef})
      self.inmean:copy(self.meanDiviseMod.output)
   end
    
   -- scale to fixed point
   self.inmean:mul(256)
   self.inmean:add(0.5)
   self.inmean:floor()
   self.inmean:div(256)
   
   -- prepare the output
   self.inmeansc:resize(self.inmean:size(1) , 
			self.inmean:size(2) , self.inmean:size(3))
   self.inmeansc:copy(self.inmean)

   -- only if we train we use smartpad
   -- otherwise we do as in hardware
   if (self.smartPad == false) then
      
      -- scale to fixed point
      self.inmean:mul(256)
      self.inmean:add(0.5)
      self.inmean:floor()

      -- get the mapping ax+b for scaling segments
      -- order of args is:    (output,      input,         segments)
      libxlearn.mapFixedPoint(self.inmeansc, self.inmean, self.xN_segments_array)
   
      -- scale to fixed point
      self.inmeansc:mul(256)
      self.inmeansc:add(0.5)
      self.inmeansc:floor()
      self.inmeansc:div(256)
   end

   -- substruct as it is done on hardware
   self.inzmean = self.difmod:forward({self.infixed,self.inmeansc})
   
   -- scale to fixed point
   self.inzmean:mul(256)
   self.inzmean:add(0.5)
   self.inzmean:floor()
   self.inzmean:div(256)
   
   -- get the sqruare
   self.inzmeansq = self.squaremod:forward(self.inzmean)
   
   -- scale to fixed point
   self.inzmeansq:mul(256)
   self.inzmeansq:add(0.5)
   self.inzmeansq:floor()
   self.inzmeansq:div(256)
   
   -- pad the input
   self.paddedin = self.meanmod:forward(self.inzmeansq)
   
   -- prepare the output
   self.invar:resize(self.paddedin:size(1) - self.kernel:size(1) + 1, 
 		     self.paddedin:size(2) - self.kernel:size(2) + 1, self.nfeatures)
   
   -- order of args is:    (output,      input,         kernel)
   libxlearn.convFixedPoint(self.invar, self.paddedin, self.kernel)
      
   -- scale to fixed point
   self.invar:mul(256)
   self.invar:add(0.5)
   self.invar:floor()
   self.invar:div(256)
   
   -- sum the maps
   self.invar = self.summod:forward(self.invar)
   self.invar = torch.Tensor():resizeAs(self.invar):copy(self.invar)
   
   -- scale to fixed point
   self.invar:mul(256)
   self.invar:add(0.5)
   self.invar:floor()
   self.invar:div(256)
   
   -- scale to fixed point
   self.invar:mul(256)
   self.invar:add(0.5)
   self.invar:floor()

   
      
   self.insqrt:resize(self.invar:size(1) , 
  		      self.invar:size(2) , self.invar:size(3))
   -- get the mapping ax+b for sqrt segments
   -- order of args is:    (output,      input,         segments)
   if self.smartPad then
      libxlearn.mapFixedPoint(self.insqrt, self.invar, self.segments_array_pad)
   else
      libxlearn.mapFixedPoint(self.insqrt, self.invar, self.segments_array_no_pad)
   end
   
   if self.smartPad then
      self.copy_insqrt:resizeAs(self.insqrt):copy(self.insqrt)
      self.stdDiviseMod:forward({self.copy_insqrt,self.coef})
      self.insqrt:copy(self.stdDiviseMod.output)
   end

   -- scale to fixed point
   self.inzmean:mul(256)
   self.inzmean:add(0.5)
   self.inzmean:floor()
   self.inzmean:mul(256)

   self.insqrt:mul(256)
   self.insqrt:add(0.5)
   self.insqrt:floor()
   
   -- divide zero mean by std to get std 1
   self.output = self.divmod:forward({self.inzmean, self.insqrt})
   
   -- now we need to scale the output to 
   -- fixed point and we are done, but there is a
   -- difference in the way we do floor() for negative
   -- numbers on the hardware and software.
   -- So we fix it by saving the sign then doing abs():floor()
   -- and appending the sign again.
   
   -- save sign:
   self.sign:resizeAs(self.output)
   self.sign:copy(self.output)
   for i = 1,self.sign:size(3) do 
      self.sign:select(3,i):apply(function(x) 
 				     if(x > 0) then 
 					return 1 
 				     else return -1
 				     end
 				  end)
   end
   
   self.output:abs()
   self.output:floor()
   self.output = self.divmod:forward({self.output, self.sign})
   
   self.output:div(256)
   
   if self.smartPad then
      -- do the ref for backprop
      self.refConvo = self.convo:forward(self.infixed)--(self.input)
      self.refMean = self.meanDiviseMod:forward({self.refConvo,self.coef})
      self.refZeroMean = self.substractMod:forward({self.input,self.refMean})
      self.refZeroMeanSq = self.squareMod:forward(self.refZeroMean)
      self.refConvoVar = self.convostd:forward(self.refZeroMeanSq)
      
      --self.refStdDevNotUnit = self.sqrtMod:forward(self.refConvoVar)
      --self.refStdDev = self.stdDiviseMod:forward({self.refStdDevNotUnit,self.coef})
      self.refStdDevNotUnit = self.stdDiviseMod:forward({self.refConvoVar,self.coef})
      self.refStdDev = self.sqrtMod:forward(self.refStdDevNotUnit)

      self.thresMod.threshold = threshold
      self.thresMod.val = math.sqrt(threshold)
      self.refthstd = self.thresMod:forward(self.refStdDev)
      self.refRes = self.diviseMod:forward({self.refZeroMean,self.refthstd})
   end
   
   --self.output = self.refRes--self.refZeroMeanSq--self.refZeroMean--self.refMean--self.refConvo--self.coef--self.refRes
    
   return self.output

end



function LocalNorm_hardware:backward(input, gradOutput)
      
   self.gradInput:resizeAs(input):zero()
   local gradDiv = self.diviseMod:backward({self.refZeroMean,self.refthstd},gradOutput)
   local gradZeroMean = gradDiv[1]
   local gradThres = gradDiv[2]
   local gradinThres = self.thresMod:backward(self.refStdDev,gradThres)
   local gradinStdDevNotUnit  = self.sqrtMod:backward(self.refStdDevNotUnit,gradinThres)
   local gradstdDiv = self.stdDiviseMod:backward({self.refConvoVar,self.coef}, gradinStdDevNotUnit)
   local gradinConvoVar = gradstdDiv[1]
   local gradinputZeroMeanSq = self.convostd:backward(self.refZeroMeanSq,gradinConvoVar)
   local gradZeroMeanSq_after = self.squareMod:backward(self.refZeroMean,gradinputZeroMeanSq)
   gradZeroMean:add(gradZeroMeanSq_after)
   local gradDiff = self.substractMod:backward({self.input,self.refMean},gradZeroMean)
   self.gradInput:add(gradDiff[1])
   local gradinMean = gradDiff[2]
   local gradinConvoNotUnit = self.meanDiviseMod:backward({self.refConvo,self.coef},gradinMean)
   local gradinConvo = gradinConvoNotUnit[1]
   self.gradInput:add(self.convo:backward(self.input,gradinConvo))
   
   return self.gradInput
end

function LocalNorm_hardware:updateParameters(learningRate)
   -- we do not update any weights
end

function LocalNorm_hardware:zeroGradParameters()
   self.meanmod:zeroGradParameters()
   self.difmod:zeroGradParameters()
   self.squaremod:zeroGradParameters()
   --self.sqrtmod:zeroGradParameters()
   self.varmod:zeroGradParameters()
   --self.thresmod:zeroGradParameters()
   self.divmod:zeroGradParameters()
end

function LocalNorm_hardware:write(file)
   parent.write(self,file)
   file:writeObject(self.kernel)
   file:writeInt(self.nfeatures)
   file:writeDouble(self.fixedThres)
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
   file:writeObject(self.summod)
   file:writeObject(self.coef)
   file:writeObject(self.convo)
   file:writeBool(self.smartPad)
   file:writeObject(self.meanDiviseMod)
   file:writeObject(self.stdDiviseMod)


   file:writeObject(self.convostd)
   file:writeObject(self.squareMod)
   file:writeObject(self.sqrtMod)
   file:writeObject(self.substractMod)
   file:writeObject(self.thresMod)
   file:writeObject(self.diviseMod)

   file:writeObject(self.xN_coefs)
   file:writeObject(self.xN_segments_array)
   file:writeObject(self.sqrtCoefs_no_pad)
   file:writeObject(self.segments_array_no_pad)
   file:writeObject(self.sqrtCoefs_pad)
   file:writeObject(self.segments_array_pad)


end

function LocalNorm_hardware:read(file)
   parent.read(self,file)
   self.kernel = file:readObject()
   self.nfeatures = file:readInt()
   self.fixedThres = file:readDouble()
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
   self.summod = file:readObject()
   self.coef = file:readObject()
   self.convo = file:readObject()
   self.smartPad = file:readBool()
   self.meanDiviseMod = file:readObject()
   self.stdDiviseMod = file:readObject() 

   self.convostd = file:readObject()
   self.squareMod = file:readObject()
   self.sqrtMod = file:readObject()
   self.substractMod = file:readObject()
   self.thresMod = file:readObject()
   self.diviseMod = file:readObject()

   self.xN_coefs = file:readObject()
   self.xN_segments_array = file:readObject()
   self.sqrtCoefs_no_pad = file:readObject()
   self.segments_array_no_pad = file:readObject()
   self.sqrtCoefs_pad = file:readObject()
   self.segments_array_pad = file:readObject()
   

   self.infixed = torch.Tensor()
   self.paddedin = torch.Tensor()
   self.inmeansc = torch.Tensor()
   self.insqrt = torch.Tensor()
   self.sign = torch.Tensor()
   self.copy_inmean = torch.Tensor()
   self.copy_insqrt = torch.Tensor()


   self.refConvo = torch.Tensor()
   self.refMean = torch.Tensor()
   self.refZeroMean = torch.Tensor()
   self.refZeroMeanSq = torch.Tensor()
   self.refConvoVar = torch.Tensor()
   self.refVar = torch.Tensor()
   self.refStdDev = torch.Tensor()
   self.refthstd = torch.Tensor()
   self.refRes = torch.Tensor()

end