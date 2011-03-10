--------------------------------------------------------------------------------
-- DataSet: a class to handle datasets for unsupervised algorithms.
-- 
-- Authors: BenCorda, ClemFarabet
--------------------------------------------------------------------------------
do
   local UnsupDataSet, parent = torch.class('UnsupDataSet','DataSet')
   local allowedExtension = "'.png', '.ppm', '.pgm', '.pnm', '.jpg'"

   function UnsupDataSet:__init(imgFolder,imgExtension)
      parent.__init(self)
      self.nextID=1
      self.folder = imgFolder
      if not imgFolder then
         error('You have to provide a folder of images')
      end
      if not string.find(allowedExtension,imgExtension) then
         print("# Allowed extensions:",allowedExtension)
         error( toolBox.NOT_IMPLEMENTED )
      end

      self.extension = imgExtension
      -- filter files with the given extension from the folder
      self.files ={}
      for f in paths.files(imgFolder) do
         if string.find(f,imgExtension) then
            ff = paths.concat(imgFolder,f)
            table.insert(self.files,ff)
         end
      end
      if #self.files == 0 then 
         print("# Warning: no file found.") 
         print("# the path of image folder has to be relative")
      end
      --switch on extension type to select loader
      if     self.extension == ".png" then self.loader = image.loadPNN
      elseif self.extension == ".ppm" then self.loader = image.loadPPM
      elseif self.extension == ".pgm" then self.loader = image.loadPPM
      elseif self.extension == ".pnm" then self.loader = image.loadPPM
      elseif self.extension == ".jpg" then self.loader = image.loadJPG
      else   
         print("# Warning: extension not understood, can't load.")
         print("# Allowed extensions:",allowedExtension)
         return
      end
   end

   function UnsupDataSet:size() 
      return #self.files
   end

   function UnsupDataSet:load(args)
      -- Clear current dataset
      self:emptySet()
      -- load all files from self.files
      if #self.files == 0 then 
         print("# Warning: no file found, can't load.") 
         return
      end
      -- load
      for i=1,#self.files do
         self[i] = image.load(self.files[i])
      end
      self.nbSamples = #self.files
   end

   function UnsupDataSet:next(args)
      local normalizer = self.normalizer or nn.LocalNorm(image.gaussian{width=5},1)
      self.normalizer = normalizer
      local width = args.width
      local height = args.height
      local chanels = args.chanels or 1

      if self.nbSamples < #self.files then
         self.nbSamples = self.nbSamples+1
         local temp = self.loader(self.files[self.nbSamples],chanels)
         self[self.nbSamples] = self.normalizer:forward(temp)
      end
      local next = self[self.nextID]

      -- crop if needed
      if width and height then
         if next:nDimension() < 3 then 
            next:resize(next:size(1),next:size(2),1) 
         end
         local newSample = torch.Tensor(width, height, next:size(3))
         local imgWidth = next:size(1)
         local imgHeight = next:size(2)
         local rangeX = imgWidth-width
         local rangeY = imgHeight-height
         local randX = toolBox.randInt(0,rangeX)
         local randY = toolBox.randInt(0,rangeY)
         image.crop(next,newSample,randX,randY,randX+width,randY+height)
         next = newSample
      end
      -- compute next elmt in a buffer ring fashion
      self.nextID = (self.nextID)%math.max(self.nbSamples,#self.files)+1
      return next
   end
end