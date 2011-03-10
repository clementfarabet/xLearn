local LcDecoder, parent = torch.class('nn.LcDecoder','nn.Module')

function LcDecoder:__init(winX,winY,woutX,woutY,overcompleteX,overcompleteY)
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
   self:forget()
end

function LcDecoder:zeroGradParameters()
   self.gradWeight:zero()
end

function LcDecoder:forget(stdv)
   if stdv then
      stdv = stdv * math.sqrt(3)
   else
      stdv = 1/math.sqrt(self.winX*self.winY)
   end
   self.weight:apply(function()
                        return random.uniform(-stdv, stdv)
                     end)

   self:updateParameters(0)
end

function LcDecoder:updateParameters(learningRate)
   self.weight:add(-learningRate, self.gradWeight)
   for y = 1,self.weight:size(4) do
      for x = 1,self.weight:size(3) do
         local w = self.weight:select(4,y):select(3,x)
         local norm = w:norm() + 1e-8
         w:div(norm)
      end
   end
end

function LcDecoder:getOutSizeFromInSize(input)
   local inX = input:size(1)
   local inY = input:size(2)
   local outSize = torch.LongStorage(3)
   outSize[1] = self.xStep * inX + self.winX-1
   outSize[2] = self.yStep * inY + self.winY-1
   outSize[3] = 1
   return outSize
end

function LcDecoder:write(file)
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

end

function LcDecoder:read(file)
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
end


-- function to display weight using cairo
function LcDecoder:showFeatures(args)
   local resX = 600
   local resY = 600
   self.window =  self.window or gfx.Window(resX, resY, 'LcDecoder')
   local step_x = 0
   local step_y = 0
   local spacePixel = 5
   local min = args.min or self.weight:min()
   local max = args.max or self.weight:max()
   local optiscale = (resX - self.woutX*spacePixel)/(self.woutX*self.winX)

   local scale = args.scale or optiscale

   for x=1,self.woutX do
      posX = (self.winX*scale + spacePixel)*(x-1)
      for y=1,self.woutY do
         posY = (self.winY*scale + spacePixel)*(y-1)
         filter = self.weight:select(4,y):select(3,x)
         -- filter = (filter - filter:min()) / filter:max()
         local tmp  = image.scaleForDisplay{tensor=filter, min=min, max=max}
         self.window:blit(tmp,scale,posX,posY)
      end
   end
end
