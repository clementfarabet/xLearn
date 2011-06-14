local Fovea, parent = torch.class('nn.Fovea', 'nn.Module')

local help_desc =
[[From a given image, generates a pyramid of increasingly large contexts, with
decreasing resolutions, centered around an x,y location. All the contexts are
resized to the global parameter patchsize: from the smallest patch
(highest-resolution) to the complete image (lowest-resolution); they are then
concatenated along the 4th dimension of a tensor, or the 3rd if the parameter
flatten is set.]]

function Fovea:__init(...)
   parent.__init(self)
   -- check args
   toolBox.unpack_class(
      self,
      {...},
      'nn.Fovea',
      help_desc,
      {arg='patchsize', type='number', help='size of patches to extract from images', req=true},
      {arg='nbscales', type='number', help='number of scales going from full image to patchsize', req=true},
      {arg='flatten', type='boolean', help='flatten output (concatenate all features along the 3rd dim)', default=false}
   )
   print('created Fovea')
end

function Fovea:forward(input,centerX,centerY)

   if input:nDimension() ~= 3 then
      xerror('expecting 3D input', '<Fovea.forward>')
   end
   local channels = input:size(3)

   centerX = centerX or math.ceil(input:size(1)/2)
   centerY = centerY or math.ceil(input:size(2)/2)

   if self.flatten then
      self.output:resize(self.patchsize,self.patchsize,channels*self.nbscales)
   else
      self.output:resize(self.patchsize,self.patchsize,channels,self.nbscales)
   end

   local offset = 1
   local size = math.min(input:size(1),input:size(2))
   local scalestep = (size - self.patchsize)/(self.nbscales-1)
   -- get all scales and downsample to the patchsize
   for i=0,self.nbscales-1 do
      local psize = math.floor(size-i*scalestep)
      local left = math.max(1,math.floor(centerX - psize/2) + 1)
      left = math.min(left,math.floor(input:size(1) - psize)+1)
      local top = math.max(1,math.floor(centerY - psize/2) + 1)
      top = math.min(top,math.floor(input:size(2) - psize)+1)

      -- scale and copy into output
      if self.flatten then
         image.scale(input:narrow(1,left,psize):narrow(2,top,psize),
                     self.output:narrow(3,i*channels+1,channels),
                     'bilinear')
      else
         image.scale(input:narrow(1,left,psize):narrow(2,top,psize),
                     self.output:select(4,i+1),
                     'bilinear')
      end
   end
   return self.output
end

function Fovea:backward(input, gradOutput)
   xerror('Not Implemented','nn.Fovea')
end


function Fovea:write(file)
   parent.write(self, file)
   file:writeInt(self.patchsize)
   file:writeInt(self.channels)
   file:writeInt(self.nbscales)
end

function Fovea:read(file)
   parent.read(self, file)
   self.patchsize = file:readInt()
   self.channels = file:readInt()
   self.nbscales = file:readInt()
end
