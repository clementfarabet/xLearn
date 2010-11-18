--------------------------------------------------------------------------------
-- DataSetLabelMe: a class to handle datasets from LabelMe.
-- 
-- Authors: BenCorda, ClemFarabet
--------------------------------------------------------------------------------
do
   local DataSetLabelMe = torch.class('DataSetLabelMe')

   local path_images = 'Images'
   local path_annotations = 'Annotations'
   local path_masks = 'Masks'

   function DataSetLabelMe:__init(args)
      -- parse args
      if not args.path then
         error('# ERROR <DataSetLabelMe:init()> please provide a path')
      end
      self.path = args.path
      self.nbClasses = args.nbClasses or 1
      self.classNames = args.classNames or {'no name'}
      self.rawSampleMaxSize = args.rawSampleMaxSize
      self.patchSize = args.patchSize or 64
      self.nbPatchPerSample = args.nbPatchPerSample or 100
      self.nbRawSamples = args.nbRawSamples
      self.verbose = args.verbose or false
      self.cacheFileName = args.cacheFile
      self.preloadSamples = args.preloadSamples or false
      self.sampling = args.samplingMode or 'random'
      self.infinite = args.infiniteSet or false
      self.classToSkip = args.classToSkip or 0

      -- fixed parameters
      self.colorMap = image.createColorMap(self.nbClasses)
      self.rawdata = {}
      self.currentIndex = -1
      --location of the patch in the img
      self.currentX = 0
      self.currentY = 0

      -- parse dir structure
      print('# loading LabelMe dataset from '..self.path)
      for folder in paths.files(paths.concat(self.path,path_images)) do
         if folder ~= '.' and folder ~= '..' then
            for file in paths.files(paths.concat(self.path,path_images,folder)) do
               if file ~= '.' and file ~= '..' then
                  local filepng = file:gsub('jpg$','png')
                  local filexml = file:gsub('jpg$','xml')
                  local imgf = paths.concat(self.path,path_images,folder,file)
                  local maskf = paths.concat(self.path,path_masks,folder,filepng)
                  local annotf = paths.concat(self.path,path_annotations,folder,filexml)
                  local size_x, size_y, size_c = image.getJPGsize(imgf)
                  table.insert(self.rawdata, {imgfile=imgf,
                                              maskfile=maskf,
                                              annotfile=annotf,
                                              size={size_x, size_y, size_c}})
               end
            end
         end
      end
      
      -- nb samples: user defined or max
      self.nbRawSamples = self.nbRawSamples or #self.rawdata

      -- extract some info (max sizes)
      self.maxX = self.rawdata[1].size[1]
      self.maxY = self.rawdata[1].size[2]
      for i = 2,self.nbRawSamples do
         if self.maxX < self.rawdata[i].size[1] then
            self.maxX = self.rawdata[i].size[1]
         end
         if self.maxY < self.rawdata[i].size[2] then
            self.maxY = self.rawdata[i].size[2]
         end
      end
      -- and nb of samples obtainable (this is overcomplete ;-)
      self.nbSamples = self.nbPatchPerSample * self.nbRawSamples

      -- max size ?
      local maxXY = math.max(self.maxX, self.maxY)
      if maxXY < self.rawSampleMaxSize then
         self.rawSampleMaxSize = maxXY
      end

      -- some info
      if self.verbose then
         print(self)
      end

      -- sampling mode
      if self.sampling == 'equal' then
         self:parseAllMasks()
      elseif self.sampling ~= 'random' then
         error('ERROR <DataSetLabelMe> unknown sampling mode')
      end

      -- preload ?
      if self.preloadSamples then
         self:preload()
      end
   end

   function DataSetLabelMe:size()
      return self.nbSamples
   end

   function DataSetLabelMe:__tostring__()
      str = '# DataSetLabelMe\n'
      str = str .. '  + path : '..self.path..'\n'
      if self.cacheFileName then
         str = str .. '  + cache files : [path]/'..self.cacheFileName..'-[tags|samples]\n'
      end
      str = str .. '  + nb samples : '..self.nbRawSamples..'\n'
      str = str .. '  + nb generated patches : '..self.nbSamples..'\n'
      if self.infinite then
         str = str .. '  + infinite set (actual nb of samples >> set:size())\n'
      end
      if self.rawSampleMaxSize then
         str = str .. '  + samples are resized to fit in a '
         str = str .. self.rawSampleMaxSize .. 'x' .. self.rawSampleMaxSize .. ' tensor'
         str = str .. ' [max raw size = ' .. self.maxX .. 'x' .. self.maxY .. ']\n'
      end
      str = str .. '  + patches size : ' .. self.patchSize .. 'x' .. self.patchSize .. '\n'
      if self.classToSkip ~= 0 then
         str = str .. '  + unused class : ' .. self.classNames[self.classToSkip] .. '\n'
      end
      str = str .. '  + sampling mode : ' .. self.sampling .. '\n'
      str = str .. '  + '..self.nbClasses..' categories : '
      for i = 1,#self.classNames-1 do
         str = str .. self.classNames[i] .. ' | '
      end
      str = str .. self.classNames[#self.classNames]
      return str
   end

   function DataSetLabelMe:__index__(key)
      if type(key)=='string' and key == 'last' then
         key = self.currentKey
      end
      if type(key)=='number' then
         if self.sampling == 'random' then
            local rawindex = key * self.nbRawSamples / self.nbSamples
            local index = math.ceil(rawindex)
            local subidx = math.floor((index - rawindex)*self.nbSamples/self.nbRawSamples)+1
            self:loadSample(index)
            local subx = math.floor(1 + ((self.currentSample:size(1)-self.patchSize-1)*random.uniform()))
            self.currentX = subx/self.currentSample:size(1)
            local suby = math.floor(1 + ((self.currentSample:size(2)-self.patchSize-1)*random.uniform()))
            self.currentY = suby/self.currentSample:size(2)
            -- generate patch
            local subtensor = self.currentSample:narrow(1,subx,self.patchSize):narrow(2,suby,self.patchSize)
            -- generate label vector for patch centre
            local vector = torch.Tensor(1,1,self.nbClasses):fill(-1)
            local ctr_x = math.floor(subx+self.patchSize/2)
            local ctr_y = math.floor(suby+self.patchSize/2)
            vector[1][1][ self.currentMask[ctr_x][ctr_y] ] = 1
            
            -- and optional string
            label = self.classNames[ self.currentMask[ctr_x][ctr_y] ]
            
            -- return sample+label
            self.currentKey = key
            return {subtensor, vector, label}, true
         elseif self.sampling == 'equal' then
            -- equally sample each category:
            local which_tag = ((key-1) % (self.nbClasses)) + 1
            while self.tags[which_tag].size == 0 or which_tag == self.classToSkip do
               -- no sample in that class, replacing with random patch
               which_tag = math.floor(random.uniform(1,self.nbClasses))
            end
            local nbSamplesPerClass = math.ceil(self.nbSamples / self.nbClasses)
            local tag_idx = math.floor((key*nbSamplesPerClass-1)/self.nbClasses) + 1
            tag_idx = ((tag_idx-1) % (self.tags[which_tag].size/3))*3 + 1

            -- generate patch
            self:loadSample(self.tags[which_tag].data[tag_idx+2])
            local ctr_x = self.tags[which_tag].data[tag_idx]
            local ctr_y = self.tags[which_tag].data[tag_idx+1] 
            local subx = math.floor(ctr_x - self.patchSize/2) + 1
            self.currentX = subx/self.currentSample:size(1)
            local suby = math.floor(ctr_y - self.patchSize/2) + 1
            self.currentY = suby/self.currentSample:size(1)
            local subtensor = self.currentSample:narrow(1,subx,self.patchSize):narrow(2,suby,self.patchSize)

            -- generate label vector for patch centre
            local vector = torch.Tensor(1,1,self.nbClasses):fill(-1)
            vector[1][1][which_tag] = 1

            -- and optional string
            label = self.classNames[which_tag]

            -- return sample+label
            self.currentKey = key
            return {subtensor, vector, label}, true
         end
      end
      return rawget(self,key)
   end

   function DataSetLabelMe:loadSample(index)
      if self.preloadedDone then
         if index ~= self.currentIndex then
            -- clean up
            self.currentSample = nil
            self.currentMask = nil
            collectgarbage()
            -- load new sample
            self.currentSample = torch.Tensor(self.preloaded.samples[index]:size())
            self.currentSample:copy(self.preloaded.samples[index]):mul(1/255)
            self.currentMask = torch.Tensor(self.preloaded.masks[index]:size())
            self.currentMask:copy(self.preloaded.masks[index])
            -- remember index
            self.currentIndex = index
         end
      elseif index ~= self.currentIndex then
         -- clean up
         self.currentSample = nil
         self.currentMask = nil
         collectgarbage()
         -- load image
         local img_loaded = image.load(self.rawdata[index].imgfile)
         local mask_loaded = image.load(self.rawdata[index].maskfile):select(3,1)
         if self.rawSampleMaxSize and (self.rawSampleMaxSize < img_loaded:size(1)
                                    or self.rawSampleMaxSize < img_loaded:size(2)) then
            local w,h
            if img_loaded:size(1) >= img_loaded:size(2) then
               w = self.rawSampleMaxSize
               h = math.floor((w*img_loaded:size(2))/img_loaded:size(1))
            else
               h = self.rawSampleMaxSize
               w = math.floor((h*img_loaded:size(1))/img_loaded:size(2))
            end
            self.currentSample = torch.Tensor(w,h,img_loaded:size(3))
            image.scale(img_loaded, self.currentSample, 'bilinear')
            self.currentMask = torch.Tensor(w,h)
            image.scale(mask_loaded, self.currentMask, 'simple')
         else
            self.currentSample = img_loaded
            self.currentMask = mask_loaded
         end
         -- process mask
         self.currentMask:mul(self.nbClasses-1):add(0.5):floor():add(1)
         self.currentIndex = index
      end
   end

   function DataSetLabelMe:preload(saveFile)
      -- if cache file exists, just retrieve images from it
      if self.cacheFileName 
         and paths.filep(paths.concat(self.path,self.cacheFileName..'-samples')) then
         print('# retrieving saved samples from :'
               .. paths.concat(self.path,self.cacheFileName..'-samples')
               .. ' [delete file to force new scan]')
         local file = torch.DiskFile(paths.concat(self.path,self.cacheFileName..'-samples'), 'r')
         file:binary()
         self.preloaded = file:readObject()
         file:close()
	 self.preloadedDone = true 
	 return
      end
      print('# preloading all images')
      self.preloaded = {samples={}, masks={}}
      for i = 1,self.nbRawSamples do
         toolBox.dispProgress(i,self.nbRawSamples)
         -- load samples, and store them in raw byte tensors (min memory footprint)
         self:loadSample(i)
         local rawTensor = torch.ByteTensor(self.currentSample:size())
         local rawMask = torch.ByteTensor(self.currentMask:size()):copy(self.currentMask)
         rawTensor:copy(self.currentSample:mul(255))
         -- insert them in our list
         table.insert(self.preloaded.samples, rawTensor)
         table.insert(self.preloaded.masks, rawMask)
      end
      self.preloadedDone = true
      -- optional cache file
      if saveFile then
         self.cacheFileName = saveFile
      end
      -- if cache file given, serialize list of tags to it
      if self.cacheFileName then
         print('# saving samples to cache file: ' 
               .. paths.concat(self.path,self.cacheFileName..'-samples'))
         local file = torch.DiskFile(paths.concat(self.path,self.cacheFileName..'-samples'), 'w')
         file:binary()
         file:writeObject(self.preloaded)
         file:close()
      end
   end

   function DataSetLabelMe:parseMask(existing_tags)
      local tags
      if not existing_tags then
         tags = {}
         local storage
         for i = 1,self.nbClasses do
            storage = torch.ShortStorage(self.rawSampleMaxSize*self.rawSampleMaxSize*3)
            tags[i] = {data=storage, size=0}
         end
      else
         tags = existing_tags
         -- make sure each tag list is large enough to hold the incoming data
         for i = 1,self.nbClasses do
            if ((tags[i].size + (self.rawSampleMaxSize*self.rawSampleMaxSize*3)) > 
             tags[i].data:size()) then
               tags[i].data:resize(tags[i].size+(self.rawSampleMaxSize*self.rawSampleMaxSize*3),true)
            end
         end
      end
      local mask = self.currentMask
      local x_start = math.ceil(self.patchSize/2)
      local x_end = mask:size(1) - math.ceil(self.patchSize/2)
      local y_start = math.ceil(self.patchSize/2)
      local y_end = mask:size(2) - math.ceil(self.patchSize/2)
      for x = x_start,x_end do
         for y = y_start,y_end do
            local size = tags[mask[x][y]].size
            tags[mask[x][y]].data[size+1] = x
            tags[mask[x][y]].data[size+2] = y
            tags[mask[x][y]].data[size+3] = self.currentIndex
            tags[mask[x][y]].size = size+3
         end
      end
      return tags
   end

   function DataSetLabelMe:parseAllMasks(saveFile)
      -- if cache file exists, just retrieve tags from it
      if self.cacheFileName and paths.filep(paths.concat(self.path,self.cacheFileName..'-tags')) then
         print('# retrieving saved tags from :' .. paths.concat(self.path,self.cacheFileName..'-tags')
            .. ' [delete file to force new scan]')
         local file = torch.DiskFile(paths.concat(self.path,self.cacheFileName..'-tags'), 'r')
         file:binary()
         self.tags = file:readObject()
         file:close()
         return
      end
      -- parse tags, long operation
      print('# parsing all masks to generate list of tags')
      print('# WARNING: this operation could allocate up to '.. 
            math.ceil(self.nbRawSamples*self.rawSampleMaxSize*self.rawSampleMaxSize*
             3*2/1024/1024)..'MB')
      self.tags = nil
      for i = 1,self.nbRawSamples do
         toolBox.dispProgress(i,self.nbRawSamples)
         self:loadSample(i)
         self.tags = self:parseMask(self.tags)
      end
      -- report
      print('# nb of patches extracted per category:')
      for i = 1,self.nbClasses do
         print('  ' .. i .. ' - ' .. self.tags[i].size / 3)
      end
      -- optional cache file
      if saveFile then
         self.cacheFileName = saveFile
      end
      -- if cache file exists, serialize list of tags to it
      if self.cacheFileName then
         print('# saving tags to cache file: ' .. paths.concat(self.path,self.cacheFileName..'-tags'))
         local file = torch.DiskFile(paths.concat(self.path,self.cacheFileName..'-tags'), 'w')
         file:binary()
         file:writeObject(self.tags)
         file:close()
      end
   end

   function DataSetLabelMe:exportIDX(samplefile, labelfile) 
      -- current limitation for IDX files
      local idxMaxSize = 2^31-1
      
      if samplefile then
         -- message
         print('# exporting data to '..samplefile..'-n|N.idx')

         -- check for global size
         local chanels = self.preloaded.samples[1]:size(3)
         local exportSize = self.rawSampleMaxSize^2 * chanels * self.nbRawSamples * 4
         local nbFiles = math.ceil(exportSize / idxMaxSize)
         local offset = 0
         
         for n = 1,nbFiles do
            local exported
            local nbSamplesPerFile = math.floor(idxMaxSize / (self.rawSampleMaxSize^2 * chanels * 4))
            if n == nbFiles then
               nbSamplesPerFile = self.nbRawSamples - ((nbFiles-1)*nbSamplesPerFile)
            end
            exported = torch.FloatTensor(self.rawSampleMaxSize, self.rawSampleMaxSize,
                                         chanels, nbSamplesPerFile)

            local filename = samplefile..'-'..string.format("%05d",n)..'|'..string.format("%05d",nbFiles)..'.idx'
            print('+ doing '..filename..' ('..exported:size(4)..' samples)')
            if not paths.filep(filename) then
               -- export samples
               for i = 1,exported:size(4) do
                  toolBox.dispProgress(i,exported:size(4))
                  local sample = self.preloaded.samples[offset+i]
                  local w = sample:size(1)
                  local h = sample:size(2)
                  exported:select(4,i):narrow(1,1,w):narrow(2,1,h):copy(sample)
               end
               offset = offset + exported:size(4)

               -- write file
               local file = torch.DiskFile(filename,'w')
               file:binary()
               file:writeInt(0x1e3d4c51) -- float type
               file:writeInt(4) -- nb dims
               file:writeInt(exported:size(4)) -- dim[0]
               file:writeInt(exported:size(3)) -- dim[1]
               file:writeInt(exported:size(2)) -- dim[2]
               file:writeInt(exported:size(1)) -- dim[3]
               file:writeFloat(exported:storage()) -- data
               file:close()
            end
         end
      end
      if labelfile then
         print('# exporting labels to '..labelfile..'-N.idx (N=0..'..tostring(#self.tags-1)..')')
         -- export each tag list in a separate file
         local tags = self.tags
	 local nbtags = #tags
         for i = 1,nbtags do
            local exportSize = tags[i].size
            toolBox.dispProgress(i,nbtags)
            if exportSize ~= 0 and not paths.filep(labelfile..'-'..tostring(i-1)..'.idx') then
               local dest = torch.ShortTensor(exportSize)
               local src = torch.ShortTensor():set(tags[i].data,1,exportSize)
               dest:copy(src)
               -- add 1 to the whole tensor, to go to 0-based indexing
               dest:apply(function (x) return x-1 end)
               -- write file
               local file = torch.DiskFile(labelfile..'-'..string.format("%05d",tostring(i-1))..'.idx', 'w')
               file:binary()
               file:writeInt(0x1e3d4c56) -- short type
               file:writeInt(2) -- nb dims
               file:writeInt(exportSize/3) -- dim[0]
               file:writeInt(3) -- dim[1]
               file:writeInt(1) -- unused dim
               file:writeShort(dest:storage()) -- data
               file:close()
               -- garbage cleaning
               dest = nil
               collectgarbage()
	    end
         end
      end
   end

   function DataSetLabelMe:display(args)
      -- parse args:
      local title = args.title or 'DataSetLabelMe'
      local min = args.min
      local max = args.max
      local nbSamples = args.nbSamples or 50
      local scale = args.scale
      local resX = args.resX or 1200
      local resY = args.resY or 800

      -- compute some geometry params
      local painter = qtwidget.newwindow(resX,resY,title)
      local step_x = 0
      local step_y = 0
      local sizeX = self.maxX
      local sizeY = self.maxY
      if not scale then
         scale = math.sqrt(resX*resY/ (sizeX*sizeY*nbSamples))
         local nbx = math.floor(resX/(scale*sizeX))
         scale = resX/(sizeX*nbx)
      end

      -- load the samples and display them
      local dispTensor = torch.Tensor(sizeX*scale,sizeY*scale,3)
      local dispMask = torch.Tensor(sizeX*scale,sizeY*scale)
      local displayer = Displayer()
      for i=1,nbSamples do
         toolBox.dispProgress(i,nbSamples)
         self:loadSample(i)
         image.scale(self.currentSample, dispTensor, 'simple')
         image.scale(self.currentMask, dispMask, 'simple')
         local displayed = image.mergeSegmentation(dispTensor, dispMask, self.colorMap)
         if (step_x > (resX-sizeX*scale)) then
            step_x = 0
            step_y = step_y +  sizeY*scale
            if (step_y > (resY-sizeY*scale)) then
               break
            end
         end
         displayer:show{painter=painter,
                        tensor=displayed,
                        min=min, max=max,
                        offset_x=step_x,
                        offset_y=step_y}
         step_x = step_x +  sizeX*scale
      end
   end
end
