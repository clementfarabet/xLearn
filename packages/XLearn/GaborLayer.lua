local GaborLayer, parent = torch.class('nn.GaborLayer', 'nn.Module')

local help_desc = 
[[a spatial (2D) convolution module, filled with Gabor filters
? refer to nn.SpatialConvolution]]

local help_example = 
[[-- create the filter bank (can serve as a good basis for
-- reprensenting image features): 8x8=64 filters, each 21x21
stimulus = lab.randn(500,500,8)
mod = nn.GaborLayer(8,8,10)
result = mod:forward(stimulus)]]

function GaborLayer:__init(nbOrientations, nbFrequencies, gaussianSize)
   parent.__init(self)

   if not nbFrequencies or not nbOrientations or not gaussianSize then
      error(toolBox.usage('nn.GaborLayer', help_desc, help_example,
                          {type='number', help='number of orientations', req=true},
                          {type='number', help='number of frequencies', req=true},
                          {type='number', help='gaussian size G, the kernels will have size 2*G+1',
                           req=true}))
   end

   -- get params
   self.size = gaussianSize*2+1
   self.nbOrientations = nbOrientations
   self.nbFrequencies = nbFrequencies
   self.nbFilters = nbFrequencies*nbOrientations

   -- under the hood, we just use a SpatialConvolution module
   local fulltable = nn.SpatialConvolutionTable:FullTable(1, self.nbFilters)
   self.bank = nn.SpatialConvolutionTable(fulltable, self.size, self.size)

   -- reset() actually sets the weights to Gabors
   self:reset()
end

function GaborLayer:reset(stdv)
   -- initialize filter bank
   local kernelBank = self.bank.weight
   local i=1
   print('# GaborLayer: generating '
         ..self.nbFilters..' '..self.bank.weight:size(1)..'x'..self.bank.weight:size(1)
         ..' filters ('..self.nbOrientations..' angles, '
         ..self.nbFrequencies..' freqs)')
   for angle=0,math.pi,math.pi/self.nbOrientations do
      for k = 1,self.nbFrequencies do
         local period = (k+1)/self.nbFrequencies
         if (i==(self.nbFilters+1)) then return end
         kernelBank:select(3,i):copy(image.gabor(self.size, 1/5, angle, period, 1))
         kernelBank:select(3,i):add(-kernelBank:select(3,i):mean())
         if stdv then
            kernelBank:select(3,i):div(kernelBank:select(3,i):std()):mul(stdv)
         else
            kernelBank:select(3,i):div(kernelBank:select(3,i):std())
         end
         i=i+1
      end
   end
end

function GaborLayer:display(zoom)
   toolBox.useQT()
   local w = qtwidget.newwindow((self.size+1)*self.nbFrequencies*(zoom or 1),
                                (self.size+1)*self.nbOrientations*(zoom or 1), 'GaborLayer')
   local k = 1
   for i = 1,self.nbOrientations do
      for j = 1,self.nbFrequencies do
         local kernel = self.bank.weight:select(3,k)
         image.qtdisplay{tensor=kernel, painter=w, min=-3, max=3,
                         offset_y=(self.size+1)*(i-1), offset_x=(self.size+1)*(j-1),
                         globalzoom=zoom or 1}
         k=k+1
      end
   end
end

function GaborLayer:forward(input)
   self.output = self.bank:forward(input)
   return self.output
end

function GaborLayer:backward(input, gradOutput)
   self.gradInput = self.bank:backward(input, gradOutput)
   return self.gradInput
end

function GaborLayer:write(file)
   parent.write(self, file)
   file:writeObject(self.bank)
end

function GaborLayer:read(file)
   parent.write(self, file)
   self.bank = file:readObject()
end