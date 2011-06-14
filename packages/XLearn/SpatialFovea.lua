local SpatialFovea, parent = torch.class('nn.SpatialFovea', 'nn.Module')

local help_desc =
[[From a given image, generates a pyramid of scales, and process each scale
with the given list of preprocessors and processors. 
The result of each module/scale is then
upsampled to produce a homogenous list of 3D feature maps (4D tensor).

The pipeline is the following:
input  ->  pyramid{ratios}  ->  preProcessors  ->  padding  ->  processors  ->  [alignment]  ->  output

There are two operating modes: focused [training], and global [inference]. 

In inference mode,
the entire input is processed, and and alignment step is performed at the end of 
the pipeline, to be fed directly to a SpatialLinear module.

In sampling mode, the fovea is first focused on a particular (x,y) point, and no
alignment is performed at the end, as all scales should produce a 1x1 result.
To focus the fovea, simply call fovea:focus(x,y,winSize) before doing a forward.
A call to fovea:focus(nil) makes it unfocus (go back to global mode). ]]

function SpatialFovea:__init(...)
   parent.__init(self)
   -- check args
   toolBox.unpack_class(
      self,
      {...},
      'nn.SpatialFovea',
      help_desc,
      {arg='nInputPlane', type='number', help='number of input planes', req=true},
      {arg='ratios', type='table', help='list of downsampling ratios', req=true},
      {arg='processors', type='table', help='list of processors (each processor sees a single scale)', req=true},
      {arg='preProcessors', type='table', help='list of preprocessors (applied before padding)'},
      {arg='fov', type='number', help='field of view (== processors\' receptive field)', default=1},
      {arg='sub', type='number', help='global subsampling (== processors\' subsampling ratio)', default=1},
      {arg='bilinear', type='number', help='bilinear interpolation', default=false}
   )

   -- internal modules:
   self.downsamplers = {}
   self.padders = {}
   self.upsamplers = {}
   self.preProcessors = self.preProcessors or {}

   -- temporary results:
   self.pyramid = {}
   self.preProcessed = {}
   self.padded = {}
   self.narrowed = {}
   self.processed = {}
   self.upsampled = {}

   self.gradUpsampled = {}
   self.gradProcessed = {}
   self.gradNarrowed = {}
   self.gradPadded = {}
   self.gradPreProcessed = {}
   self.gradPyramid = {}

   -- inferred params
   self.padding = self.fov - self.sub

   -- check processors
   if #self.processors ~= #self.ratios then
      xerror('the number of processors provided should == the number of ratiios (scales): ' .. #self.ratios,
             'nn.SpatialFovea')
   end
end

function SpatialFovea:focus(x,y,fov)
   self.x = x
   self.y = y
   self.fov = fov or self.fov
   if self.x and self.y and self.fov then
      self.focused = true
   else
      self.focused = false
   end
end

function SpatialFovea:configure(width,height)
   -- init modules
   for idx = 1,#self.ratios do
      -- down/up ratio
      local r = self.ratios[idx]

      -- downsamplers
      if self.bilinear then
         self.downsamplers[idx] = nn.SpatialReSampling(width/r, height/r)
      else
         self.downsamplers[idx] = nn.SpatialSubSampling(self.nInputPlane, r, r, r, r)
         self.downsamplers[idx].weight:fill(1/(r^2))
         self.downsamplers[idx].bias:zero()
      end

      -- padders
      if self.padding == 0 then
         self.padders[idx] = nn.Identity()
      else
         local padl = math.floor(self.padding / 2)
         local padr = math.floor(self.padding / 2)
         self.padders[idx] = nn.SpatialPadding(padl, padr, padl, padr)
      end

      -- upsamplers
      self.upsamplers[idx] = nn.SpatialUpSampling(self.nInputPlane, r, r)
   end
end

function SpatialFovea:forward(input)
   -- input must be 3D
   if input:nDimension() ~= 3 then
      xerror('input must be 3d','nn.SpatialFovea')
   end
   local width = input:size(1)
   local height = input:size(2)
   local nmaps = input:size(3)
   local nscales = #self.ratios
   if input:size(3) ~= self.nInputPlane then
      xerror('input must have ' .. self.nInputPlane .. ' input planes' ,'nn.SpatialFovea')
   end
   self:configure(width,height)

   -- (1) generate pyramid
   for idx = 1,nscales do
      self.pyramid[idx] = self.downsamplers[idx]:forward(input)
   end

   -- (2) preprocess
   for idx = 1,nscales do
      if self.preProcessors[idx] then
         self.preProcessed[idx] = self.preProcessors[idx]:forward(self.pyramid[idx])
      else
         self.preProcessed[idx] = self.pyramid[idx]
      end
   end

   -- (3) pad inputs
   for idx = 1,nscales do
      self.padded[idx] = self.padders[idx]:forward(self.preProcessed[idx])
   end

   -- (4) is fovea focused ?
   if self.focused then
      for idx = 1,nscales do
         local fov = self.fov
         local ox = math.floor(math.floor((self.x-1) / self.ratios[idx]) / self.sub) * self.sub + 1
         local oy = math.floor(math.floor((self.y-1) / self.ratios[idx]) / self.sub) * self.sub + 1
         self.narrowed[idx] = self.padded[idx]:narrow(1,ox,fov):narrow(2,oy,fov)
      end
   else
      for idx = 1,nscales do
         self.narrowed[idx] = self.padded[idx]
      end
   end

   -- (5) apply processors to pyramid
   for idx = 1,nscales do
      self.processed[idx] = self.processors[idx]:forward(self.narrowed[idx])
   end

   -- (6) upscale, only if fovea is not focused
   if self.focused then
      for idx = 1,nscales do
         self.upsampled[idx] = self.processed[idx]
      end
   else
      for idx = 1,nscales do
         self.upsampled[idx] = self.upsamplers[idx]:forward(self.processed[idx])
      end
   end

   -- (7) concatenate all maps into a single 3D volume
   local currentslice = 1
   for idx = 1,nscales do
      currentslice = currentslice + self.processed[idx]:size(3)
   end
   self.output:resize(self.upsampled[1]:size(1), self.upsampled[1]:size(2), currentslice-1)
   currentslice = 1
   for idx = 1,nscales do
      local omap = self.output:narrow(3, currentslice, self.upsampled[idx]:size(3))
      omap:copy( self.upsampled[idx] )
      currentslice = currentslice + self.upsampled[idx]:size(3)
   end
   return self.output
end

function SpatialFovea:backward(input, gradOutput)
   -- nb of scales
   local nscales = #self.ratios

   -- (7) extract different scales
   local currentslice = 1
   for idx = 1,nscales do
      self.gradUpsampled[idx] = gradOutput:narrow(3, currentslice, self.processed[idx]:size(3))
      currentslice = currentslice + self.upsampled[idx]:size(3)
   end

   -- (6) bprop through upsamplers
   if self.focused then
      for idx = 1,nscales do
         self.gradProcessed[idx] = self.gradUpsampled[idx]
      end
   else
      for idx = 1,nscales do
         self.gradProcessed[idx] = self.upsamplers[idx]:backward(self.processed[idx], self.gradUpsampled[idx])
      end
   end

   -- (5) bprop through processors
   for idx = 1,nscales do
      self.gradNarrowed[idx] = self.processors[idx]:backward(self.narrowed[idx], self.gradProcessed[idx])
   end

   -- (4) is fovea focused ?
   if self.focused then
      for idx = 1,nscales do
         self.gradPadded[idx] = self.gradPadded[idx] or torch.Tensor()
         self.gradPadded[idx]:resizeAs(self.padded[idx]):zero()
         local fov = self.fov
         local ox = math.floor(math.floor((self.x-1) / self.ratios[idx]) / self.sub) * self.sub + 1
         local oy = math.floor(math.floor((self.y-1) / self.ratios[idx]) / self.sub) * self.sub + 1
         self.gradPadded[idx]:narrow(1,ox,fov):narrow(2,oy,fov):copy(self.gradNarrowed[idx])
      end
   else
      for idx = 1,nscales do
         self.gradPadded[idx] = self.gradNarrowed[idx]
      end
   end

   -- (3) bprop through padders
   for idx = 1,nscales do
      self.gradPreProcessed[idx] = self.padders[idx]:backward(self.preProcessed[idx], self.gradPadded[idx])
   end

   -- (2) bprop through preProcessors
   for idx = 1,nscales do
      if self.preProcessors[idx] then
         self.gradPyramid[idx] = self.preProcessors[idx]:backward(self.pyramid[idx], self.gradPreProcessed[idx])
      else
         self.gradPyramid[idx] = self.gradPreProcessed[idx]
      end
   end

   -- (1) bprop through pyramid
   self.gradInput:resizeAs(self.gradPyramid[1]):zero()
   for idx = 1,nscales do
      self.gradInput:add( self.downsamplers[idx]:backward(input, self.gradPyramid[idx]) )
   end
   return self.gradInput
end

function SpatialFovea:reset(stdv)
   for idx = 1,#self.processors do
      self.processors[idx]:reset(stdv)
   end
end

function SpatialFovea:zeroGradParameters()
   for idx = 1,#self.processors do
      self.processors[idx]:zeroGradParameters()
   end
end

function SpatialFovea:updateParameters(learningRate)
   for idx = 1,#self.processors do
      self.processors[idx]:updateParameters(learningRate)
   end
end

function SpatialFovea:decayParameters(decay)
   for idx = 1,#self.processors do
      self.processors[idx]:decayParameters(decay)
   end
end

function SpatialFovea:write(file)
   parent.write(self, file)
   file:writeInt(self.nInputPlane)
   file:writeInt(self.padding)
   file:writeInt(self.fov)
   file:writeInt(self.sub)
   file:writeObject(self.ratios)
   file:writeObject(self.downsamplers)
   file:writeObject(self.padders)
   file:writeObject(self.upsamplers)
   file:writeObject(self.processors)
   file:writeObject(self.preProcessors)
   file:writeObject(self.pyramid)
   file:writeObject(self.preProcessed)
   file:writeObject(self.padded)
   file:writeObject(self.narrowed)
   file:writeObject(self.processed)
   file:writeObject(self.upsampled)
end

function SpatialFovea:read(file)
   if nn.SpatialFovea_legacy then
      self:read_legacy(file)
      return
   end
   parent.read(self, file)
   self.nInputPlane = file:readInt()
   self.padding = file:readInt()
   self.fov = file:readInt()
   self.sub = file:readInt()
   self.ratios = file:readObject()
   self.downsamplers = file:readObject()
   self.padders = file:readObject()
   self.upsamplers = file:readObject()
   self.processors = file:readObject()
   self.preProcessors  = file:readObject()
   self.pyramid = file:readObject()
   self.preProcessed = file:readObject()
   self.padded = file:readObject()
   self.narrowed = file:readObject()
   self.processed = file:readObject()
   self.upsampled = file:readObject()
   self.gradUpsampled = {}
   self.gradProcessed = {}
   self.gradNarrowed = {}
   self.gradPadded = {}
   self.gradPreProcessed = {}
   self.gradPyramid = {}
end

function SpatialFovea:read_legacy(file)
   local parentsargs = {}
   parent.read(parentsargs, file)
   local nInputPlane = file:readInt()
   local padding = file:readInt()
   local fov = file:readInt()
   local ratios = file:readObject()
   local sub = fov-padding

   local downsamplers = file:readObject()
   local upsamplers = file:readObject()
   local processors = file:readObject()
   local preProcessors = file:readObject()

   self:__init(nInputPlane,ratios,processors,preProcessors,fov,sub)
   for k,a in pairs(parentsargs) do
      self[k] = a
   end
end
