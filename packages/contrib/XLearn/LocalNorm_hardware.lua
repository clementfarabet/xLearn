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


function LocalNorm_hardware:__init(ker,nf,thres)

   local neuf = xrequire 'NeuFlow'
   if not neuf then
      error('<LocalNorm_hardware> could not load NeuFlow, package required')
   end

   parent.__init(self)
   self.kernel = ker --torch.Tensor:resizeAs(ker):copy(ker)
   self.kernel:div(self.kernel:sum())

   -- using the same function as on hardware to transform the kernel
   self:normKernel(self.kernel)
   
   -- scaling to fixed point
   self.kernel:mul(256)
   self.kernel:add(0.5)
   self.kernel:floor()
   self.kernel:div(256)

   
   self.nfeatures = nf
   self.fixedThres = thres -- optional, if not provided, the global std is used

   if not ker or not nf or type(ker) ~= 'userdata' then
      error(toolBox.usage('nn.LocalNorm_hardware',
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
--    self.thresmod = nn.Threshold()

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
--    self.instd = torch.Tensor()
--    self.thstd = torch.Tensor()
end


function LocalNorm_hardware:forward(input)

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

   -- scale to fixed point
   self.inmean:mul(256)
   self.inmean:add(0.5)
   self.inmean:floor()
   self.inmean:div(256)
   
   -- generate coefs for scaler
   -- (dividing by num of features)
   local xN = function (x) 
                 return x / self.nfeatures
              end
   local xN_coefs =  math.approx_line{mapping=xN, min=num.min, 
                                      max=num.max, odd = true,
                                      nbSegments=8, Q=num.frac_,
                                      verbose=false, a = 1/self.nfeatures, b = 0}

   local xN_segments_array = torch.Tensor(3, #xN_coefs)

   -- put segments into array
   for i = 1,#xN_coefs do
      xN_segments_array[1][i] = xN_coefs[i].min
      xN_segments_array[2][i] = xN_coefs[i].a
      xN_segments_array[3][i] = xN_coefs[i].b
   end
   
   -- scale to fixed point
   self.inmean:mul(256)
   self.inmean:add(0.5)
   self.inmean:floor()
   -- prepare the output
   self.inmeansc:resize(self.inmean:size(1) , 
			self.inmean:size(2) , self.inmean:size(3))
   -- get the mapping ax+b for scaling segments
   -- order of args is:    (output,      input,         segments)
   libxlearn.mapFixedPoint(self.inmeansc, self.inmean, xN_segments_array)

   -- scale to fixed point
   self.inmeansc:mul(256)
   self.inmeansc:add(0.5)
   self.inmeansc:floor()
   self.inmeansc:div(256)

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

   -- generate coefs for sqrt
   local mapping
   threshold = self.fixedThres or 1/256
   mapping = function (x) 
		x = x / self.nfeatures
		if x < threshold then return threshold 
		else return math.sqrt(x) end
	     end
   sqrtCoefs = math.approx{mapping=mapping, min=0, max=num.max,
			   nbSegments=8, Q=num.frac_,
			   verbose=false, epsilon=25/256,error_type = 0,
 			   name = 'Sqrt_th_div_'..self.nfeatures..'_'..threshold}
   
   local segments_array = torch.Tensor(3, #sqrtCoefs)
   
   -- put segments into array
   for i = 1,#sqrtCoefs do
      segments_array[1][i] = sqrtCoefs[i].min
      segments_array[2][i] = sqrtCoefs[i].a
      segments_array[3][i] = sqrtCoefs[i].b
   end
   
   -- scale to fixed point
   self.invar:mul(256)
   self.invar:add(0.5)
   self.invar:floor()
   
   self.insqrt:resize(self.invar:size(1) , 
 		      self.invar:size(2) , self.invar:size(3))
   -- get the mapping ax+b for sqrt segments
   -- order of args is:    (output,      input,         segments)
   libxlearn.mapFixedPoint(self.insqrt, self.invar, segments_array)
   
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
   
   return self.output
end


function LocalNorm_hardware:forward_DEBUG(input)
   print('in LocalNorm_hardware')
   
   self.infixed:resizeAs(input):copy(input)
   
   -- DEBUG
   --print('input before:')
   --print(self.infixed)
   
   -- scaling to fixed point
   self.infixed:mul(256)
   self.infixed:add(0.5)
   self.infixed:floor()
   self.infixed:div(256)

   -- DEBUG
   --print('input after scaling to fixed point:')
   --print(self.infixed)
   
   -- self.output = self.infixed
   
   self.paddedin = self.meanmod:forward(self.infixed)

   -- self.infixed - input (28x28x1)
   -- self.paddedin - input (31x31x1)
   -- self.kernel - weights (7x7)
   -- self.inmean - output  (28x28x1)
   
   self.inmean:resize(self.paddedin:size(1) - self.kernel:size(1) + 1, 
 		      self.paddedin:size(2) - self.kernel:size(2) + 1, self.nfeatures)

   -- order of args is:    (output,      input,         kernel)
   libxlearn.convFixedPoint(self.inmean, self.paddedin, self.kernel)
   
   -- scale to fixed point
   self.inmean:mul(256)
   self.inmean:add(0.5)
   self.inmean:floor()
   self.inmean:div(256)

   self.inmean = self.summod:forward(self.inmean)
   
   self.inmean = torch.Tensor():resizeAs(self.inmean):copy(self.inmean)

   -- scale to fixed point
   self.inmean:mul(256)
   self.inmean:add(0.5)
   self.inmean:floor()
   self.inmean:div(256)
   
   -- DEBUG
   --print('after convolution:')
   --print(self.inmean)
   
   --self.output = self.inmean

   -- generate coefs for scaler
   -- (dividing by num of features)
   local xN = function (x) 
                 return x / self.nfeatures
              end
   local xN_coefs = math.approx_line{mapping=xN, min=num.min, 
				     max=num.max, odd = true,
				     nbSegments=8, Q=num.frac_,
				     verbose=false, a = 1/self.nfeatures, b = 0}

   --    -- DEBUG
   print('LocalNorm_hardware: coefs for scaling (divide by num of features):')
   p(xN_coefs)
   
   local xN_segments_array = torch.Tensor(3, #xN_coefs)

   -- put segments into array
   for i = 1,#xN_coefs do
      xN_segments_array[1][i] = xN_coefs[i].min
      xN_segments_array[2][i] = xN_coefs[i].a
      xN_segments_array[3][i] = xN_coefs[i].b
   end
   
   -- DEBUG
   print('LocalNorm_hardware: coefs for scaling as array:')
   print(xN_segments_array)
   
   -- scale to fixed point
   self.inmean:mul(256)
   self.inmean:add(0.5)
   self.inmean:floor()
   
   -- DEBUG
   --print('fixed point before scaling:')
   --print(self.inmean)


   self.inmeansc:resize(self.inmean:size(1) , 
			self.inmean:size(2) , self.inmean:size(3))
   -- get the mapping ax+b for scaling segments
   -- order of args is:    (output,      input,         segments)
   libxlearn.mapFixedPoint(self.inmeansc, self.inmean, xN_segments_array)

   --self.output = self.inmeansc
   
   -- scale to fixed point
   self.inmeansc:mul(256)
   self.inmeansc:add(0.5)
   self.inmeansc:floor()
   self.inmeansc:div(256)



    -- DEBUG
   --print('after mapping scaler:')
   --print(self.inmeansc)

   
   

   -- substruct as it is done on hardware
   self.inzmean = self.difmod:forward({self.infixed,self.inmeansc})
   
   -- scale to fixed point
   self.inzmean:mul(256)
   self.inzmean:add(0.5)
   self.inzmean:floor()
   self.inzmean:div(256)


   --self.output =  self.inzmean
   
    -- get the sqruare
    self.inzmeansq = self.squaremod:forward(self.inzmean)
    -- scale to fixed point
    self.inzmeansq:mul(256)
    self.inzmeansq:add(0.5)
    self.inzmeansq:floor()
    self.inzmeansq:div(256)

    --self.output = self.inzmeansq
   
    self.paddedin = self.meanmod:forward(self.inzmeansq)
   
    self.invar:resize(self.paddedin:size(1) - self.kernel:size(1) + 1, 
 		      self.paddedin:size(2) - self.kernel:size(2) + 1, self.nfeatures)
   
    -- order of args is:    (output,      input,         kernel)
    libxlearn.convFixedPoint(self.invar, self.paddedin, self.kernel)
   
    --self.output = self.invar

    -- scale to fixed point
   self.invar:mul(256)
   self.invar:add(0.5)
   self.invar:floor()
   self.invar:div(256)

   self.invar = self.summod:forward(self.invar)
   
   self.invar = torch.Tensor():resizeAs(self.invar):copy(self.invar)

   -- scale to fixed point
   self.invar:mul(256)
   self.invar:add(0.5)
   self.invar:floor()
   self.invar:div(256)


   
    -- generate coefs for sqrt
    local mapping
    threshold = threshold or 1/256
    mapping = function (x) 
                 x = x / self.nfeatures
                 if x < threshold then return threshold 
                 else return math.sqrt(x) end
              end
    sqrtCoefs = math.approx{mapping=mapping, min=0, max=num.max,
                            nbSegments=8, Q=num.frac_,
                            verbose=false, epsilon=25/256,error_type = 0,
 			   name = 'Sqrt_th_div_'..self.nfeatures}
    -- DEBUG
    --print('LocalNorm_hardware: coefs for sqrt:')
    --p(sqrtCoefs)
   
    local segments_array = torch.Tensor(3, #sqrtCoefs)

    -- put segments into array
    for i = 1,#sqrtCoefs do
       segments_array[1][i] = sqrtCoefs[i].min
       segments_array[2][i] = sqrtCoefs[i].a
       segments_array[3][i] = sqrtCoefs[i].b
    end
    -- DEBUG
    --print('LocalNorm_hardware: coefs for sqrt as array:')
    --print(segments_array)
   
    -- scale to fixed point
    self.invar:mul(256)
    self.invar:add(0.5)
    self.invar:floor()
   
    self.insqrt:resize(self.invar:size(1) , 
 		      self.invar:size(2) , self.invar:size(3))
    -- get the mapping ax+b for sqrt segments
    -- order of args is:    (output,      input,         segments)
    libxlearn.mapFixedPoint(self.insqrt, self.invar, segments_array)
   
    --self.output = self.insqrt

--    --print('A:')
--    --print(self.inzmean)


    self.inzmean:mul(256)
    self.inzmean:add(0.5)
    self.inzmean:floor()
    self.inzmean:mul(256)
   
--    --print('A scaled:')
--    --print(self.inzmean)
   

--    --print('B:')
--    --print(self.insqrt)

    self.insqrt:mul(256)
    self.insqrt:add(0.5)
    self.insqrt:floor()

--    --print('B scaled:')
--    --print(self.insqrt)

    self.output = self.divmod:forward({self.inzmean, self.insqrt})
   
--    --print('C = A / B:')
--    --print(self.output)


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

	  


--    --print('sign:')
--    --print(self.sign)

    self.output:abs()
    self.output:floor()
   
    self.output = self.divmod:forward({self.output, self.sign})
   
    self.output:div(256)

   return self.output
end



function LocalNorm_hardware:backward(input, gradOutput)
   --self.gradInput:resizeAs(input):zero()

   -- local gdivmod = self.divmod:backward({self.inzmean,self.thstd},gradOutput)
--    local gthstd = gdivmod[2]
--    local ginzmean = gdivmod[1]
--    local ginstd = self.thresmod:backward(self.instd,gthstd)
--    local ginvar = self.sqrtmod:backward(self.invar,ginstd)
--    local ginzmeansq = self.varmod:backward(self.inzmeansq,ginvar)
--    ginzmean:add(self.squaremod:backward(self.inzmean,ginzmeansq))
--    local gdifmod = self.difmod:backward({input,self.inmean},ginzmean)
--    local ginmean = gdifmod[2]
--    self.gradInput:add(gdifmod[1])
--    self.gradInput:add(self.meanmod:backward(input,ginmean))
   
   self.gradInput:resizeAs(gradOutput):copy(gradOutput)
   
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

function LocalNorm_hardware:read(file)
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
   
   self.infixed = torch.Tensor()
   self.paddedin = torch.Tensor()
   self.inmeansc = torch.Tensor()
   self.insqrt = torch.Tensor()
   self.sign = torch.Tensor()

   self.summod = nn.Sequential()
   self.summod:add(nn.Sum(3))
   self.summod:add(nn.Replicate(self.nfeatures))
   
end
