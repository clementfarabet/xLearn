--------------------------------------------------------------------------------
-- VideoDataSet: a class to handle VIdeo as a datasets for learning algorithms.
-- 
-- Authors: BenCorda, ClemFarabet
--------------------------------------------------------------------------------
do
   local VideoDataSet, parent = torch.class('VideoDataSet','DataSet')
   
   function VideoDataSet:__init(movie,size,select)
      require 'libmpeg2'
      parent.__init(self)
      self.file = movie
      self.decoder = mpeg2.Decoder(movie)
      self.frame = torch.Tensor(1)
      self.sizeSet = size
      self.select = select or 'none' -- choices are 'random' or 'center' or 'none'
   end

   function VideoDataSet:size() 
      return self.sizeSet
   end

   function VideoDataSet:next(args)
      self.width = args.width or self.width
      self.height = args.height or self.height
      self.chanels = args.chanels or 1
      self.select = args.select or self.select

      if self.decoder:newFrame(self.frame) == 0 then
         print("#Warning, end of video resting from begining...")
         self.decoder = mpeg2.Decoder(self.file)
         return self:next{}
      end

      local xleft = 0
      local yleft = 0
      local sX = self.frame:size(1)
      local sY = self.frame:size(2)
      if self.width > sX or self.height > sY then 
          print("#Warning, patch size too big reset to frame size...")
      else
         if self.select == 'center' then
            -- select patch in the exact center of the frame
            local cX = math.floor(sX/2)
            local cY = math.floor(sY/2)
            xleft = cX - math.floor(self.width/2)
            yleft = cY - math.floor(self.height/2)
            self.frame = self.frame:narrow(1,xleft,self.width)
            self.frame = self.frame:narrow(2,yleft,self.height)
         elseif self.select == 'random' then
            xleft = toolBox.randInt(1,sX-self.width)
            yleft = toolBox.randInt(1,sY-self.height)
            self.frame = self.frame:narrow(1,xleft,self.width)
            self.frame = self.frame:narrow(2,yleft,self.height)
         elseif self.select == 'none' then
            self.frame = self.frame:narrow(1,1,self.width)
            self.frame = self.frame:narrow(2,1,self.height)
         else
            print("# Error, select type unknonw choices are 'random' or 'center' or 'none'")
            error( toolBox.NOT_IMPLEMENTED )
         end
      end
      if self.chanels == 1 then
         -- convert to grayscale
         local r = self.frame:select(3,1)
         local g = self.frame:select(3,2)
         local b = self.frame:select(3,3)
         tmp = torch.Tensor(r:size(1),r:size(2),1):zero()
         tmp:add(0.3,r)
         tmp:add(0.59,g)
         tmp:add(0.11,b)
         self.frame = tmp
      elseif not self.chanels == 3 then
         print("# Error, Channels can only be 1 or 3 for now")
         error( toolBox.NOT_IMPLEMENTED )
      end
      -- you can create a new process method if you like
      if self.preProcess then
         self.preProcess(self, self.frame)
      end
      return self.frame
   end

end