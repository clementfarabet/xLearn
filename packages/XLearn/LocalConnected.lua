local LocalConnected, parent = torch.class('nn.LocalConnected','nn.Module')

function LocalConnected:__init(winX,winY,woutX,woutY,overcompleteX,overcompleteY)
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

function LocalConnected:zeroGradParameters()
   self.gradWeight:zero()
end

function LocalConnected:forget(stdv)
   if stdv then
      stdv = stdv * math.sqrt(3)
   else
      stdv = 1/math.sqrt(self.winX*self.winY)
   end
   self.weight:apply(function()
                        return random.uniform(-stdv, stdv)
                     end)
end

function LocalConnected:updateParameters(learningRate)
   self.weight:add(-learningRate, self.gradWeight)
end

function LocalConnected:learn(learningRate)
   error( toolBox.NOT_IMPLEMENTED )
   --self.weight:add(-learningRate, self.gradWeight)
end

function LocalConnected:getOutSizeFromInSize(input)
   local inX = input:size(1)
   local inY = input:size(2)
   local outSize = torch.LongStorage(3)
   outSize[1] = (inX - self.winX+1) / self.xStep
   outSize[2] = (inY - self.winY+1) / self.yStep
   outSize[3] = 1
   return outSize
end

function LocalConnected:forward(input)
   self.output = torch.Tensor(self:getOutSizeFromInSize(input))
   self.input = input
   local ix
   local wx
   local iy
   local wy
   local fanin
   local weights
   for x = 1,self.output:size(1) do
      ix = math.ceil(x*self.xStep)
      wx = x%self.woutX
      if wx == 0 then wx = self.woutX end
      for y = 1,self.output:size(2) do
         iy = math.ceil(y*self.yStep)
         wy = y%self.woutY
         -- since 1 based when wy=0 turn out to be the last one
         -- meaning weights[:][:][:][self.woutY]
         if wy == 0 then wy = self.woutY end
         fanin = input:narrow(1,ix,self.winX):narrow(2,iy,self.winY)
         weights = self.weight:select(4,wy):select(3,wx)
         self.output[x][y][1] = fanin:dot(weights)
      end
   end
   return self.output
end

function LocalConnected:backward(input, gradOutput)
   -- compute dE/dx 
   self.gradInput = torch.Tensor(input:size()):zero()
   for x = 1,gradOutput:size(1) do
      local ix = math.ceil(x*self.xStep)
      local wx = x%self.woutX
      if wx == 0 then wx = self.woutX end
      for y = 1,gradOutput:size(2) do
         local iy = math.ceil(y*self.yStep)
         local wy = y%self.woutY
         if wy == 0 then wy = self.woutY end
         local fanin = self.gradInput:narrow(1,ix,self.winX):narrow(2,iy,self.winY)
         local weights = self.weight:select(4,wy):select(3,wx)
         fanin:add(gradOutput[x][y][1],weights)
      end
   end
   -- compute dE/dW
   self:zeroGradParameters()
   for x = 1,gradOutput:size(1) do
      local ix = math.ceil(x*self.xStep)
      local wx = x%self.woutX
      if wx == 0 then wx = self.woutX end
      for y = 1,gradOutput:size(2) do
         local iy = math.ceil(y*self.yStep)
         local wy = y%self.woutY
         if wy == 0 then wy = self.woutY end
         local fanin = input:narrow(1,ix,self.winX):narrow(2,iy,self.winY)
         local gradWeight = self.gradWeight:select(4,wy):select(3,wx)
         gradWeight:add(gradOutput[x][y][1],fanin)
      end
   end  
   return self.gradInput
end

function LocalConnected:write(file)
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

function LocalConnected:read(file)
   parent.read(self,file)
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
   -- weight matrix
   self.weight = file:readObject()
   self.gradWeight = file:readObject()
end

function LocalConnected:showFeatures(scale)
   local resX = 600
   local resY = 600
   self.window =  self.window or gfx.Window(resX, resY)
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


-- test jaco

--x and x`=x-epsi

-------------
-- notes
-------------
-- wd0 : output x size woutY
-- wd1 : output y size woutX
-- wd2 : input x size  winY
-- wd3 : input y size  winX

-- compy : overcomplete factor on y  overcompleteY
-- compx : over complete factor on x overcompleteX

-- yl : left part of input y skipped for learning
-- xl : left part of input x skipped for learning
-- yr : right part of input y skipped for learning
-- xr : right part of input x skipped for learning