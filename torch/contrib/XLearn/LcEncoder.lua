local LcEncoder, parent = torch.class('nn.LcEncoder','nn.Module')

function LcEncoder:__init(winX,winY,woutX,woutY,overcompleteX,overcompleteY)
   parent.__init(self)
   self.winX = winX
   self.winY = winY
   self.woutX = woutX
   self.woutY = woutY
   self.overcompleteX = overcompleteX
   self.overcompleteY = overcompleteY
   self.xStep = 1/overcompleteX
   self.yStep = 1/overcompleteY
   self.yLeft = math.floor(winY/2)
   self.xLeft = math.floor(winX/2)
   self.yRight = math.floor((winY-1)/2)
   self.xRight = math.floor((winX-1)/2)
   -- weight matrix
   self.weight = torch.Tensor(winX,winY,woutX,woutY)
   self.gradWeight = torch.Tensor(winX,winY,woutX,woutY)
   -- bias matrix
   self.bias = torch.Tensor(winX,winY)
   self.gradBias = torch.Tensor(winX,winY)
   self:forget()
end

function LcEncoder:zeroGradParameters()
   self.gradWeight:zero()
   self.gradBias:zero()
end

function LcEncoder:forget(stdv)
   if stdv then
      stdv = stdv * math.sqrt(3)
   else
      stdv = 1/math.sqrt(self.winX*self.winY)
   end
   self.weight:apply(function()
                        return random.uniform(-stdv, stdv)
                     end)
   self.bias:apply(function()
                      return random.uniform(-stdv, stdv)
                   end)
end

function LcEncoder:updateParameters(learningRate)
   self.weight:add(-learningRate, self.gradWeight)
   self.bias:add(-learningRate, self.gradBias)
   for y = 1,self.weight:size(4) do
      for x = 1,self.weight:size(3) do
         local w = self.weight:select(4,y):select(3,x)
         local norm = w:norm() + 1e-8
         w:div(norm)
      end
   end
end

function LcEncoder:getOutSizeFromInSize(input)
   local inX = input:size(1)
   local inY = input:size(2)
   local outSize = torch.LongStorage(3)
   outSize[1] = (inX - self.winX+1) / self.xStep
   outSize[2] = (inY - self.winY+1) / self.yStep
   outSize[3] = 1
   return outSize
end

function LcEncoder:write(file)
   parent.write(self,file)
   file:writeInt(self.winX)
   file:writeInt(self.winY)
   file:writeInt(self.woutX)
   file:writeInt(self.woutY)
   file:writeInt(self.overcompleteX)
   file:writeInt(self.overcompleteY)
   file:writeDouble(self.xStep)
   file:writeDouble(self.yStep)
   file:writeDouble(self.yLeft)
   file:writeDouble(self.xLeft)
   file:writeDouble(self.yRight)
   file:writeDouble(self.xRight)
   file:writeObject(self.weight)
   file:writeObject(self.gradWeight)
   file:writeObject(self.bias)
   file:writeObject(self.gradBias)
end

function LcEncoder:read(file)
   parent.read(self,file)
   self.winX = file:readInt()
   self.winY = file:readInt()
   self.woutX = file:readInt()
   self.woutY = file:readInt()
   self.overcompleteX = file:readInt()
   self.overcompleteY = file:readInt()
   self.xStep = file:readDouble()
   self.yStep = file:readDouble()
   self.yLeft = file:readDouble()
   self.xLeft = file:readDouble()
   self.yRight = file:readDouble()
   self.xRight = file:readDouble()
   self.weight = file:readObject()
   self.gradWeight = file:readObject()
   self.bias = file:readObject()
   self.gradBias = file:readObject()
end

function LcEncoder:showFeatures(scale)
   local resX = 600
   local resY = 600
   self.window =  self.window or gfx.Window(resX, resY, 'LcEncoder')
   local step_x = 0
   local step_y = 0
   local spacePixel = 5
   local optiscale = (resX - self.woutX*spacePixel)/(self.woutX*self.winX)

   local scale = scale or optiscale

   for x=1,self.woutX do
      posX = (self.winX*scale + spacePixel)*(x-1)
      for y=1,self.woutY do
         posY = (self.winY*scale + spacePixel)*(y-1)
         filter = self.weight:select(4,y):select(3,x)
         filter = (filter - filter:min()) / filter:max()
         self.window:blit(filter,scale,posX,posY)
      end
   end
end

-- function to display weight using cairo
function LcEncoder:showFeatures(args)
   local resX = 600
   local resY = 600
   local step_x = 0
   local step_y = 0
   local min = args.min or self.weight:min()
   local max = args.max or self.weight:max()
   self.window =  self.window or gfx.Window(resX, resY, 'LcEncoder')
   local spacePixel = 5
   local optiscale = (resX - self.woutX*spacePixel)/(self.woutX*self.winX)
   local scale = args.scale or optiscale

   for x=1,self.woutX do
      posX = (self.winX*scale + spacePixel)*(x-1)
      for y=1,self.woutY do
         posY = (self.winY*scale + spacePixel)*(y-1)
         filter = self.weight:select(4,y):select(3,x)
         local tmp  = image.scaleForDisplay{tensor=filter, min=min, max=max}
         self.window:blit(tmp,scale,posX,posY)
      end
   end
end