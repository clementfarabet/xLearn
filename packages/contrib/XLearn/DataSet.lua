--------------------------------------------------------------------------------
-- DataSet: a class to handle datasets.
-- 
-- Authors: BenCorda, ClemFarabet
--------------------------------------------------------------------------------

--- creates a class "DataSet"
do
   local DataSet = torch.class('DataSet')

   --- the initializer
   function DataSet:__init(args)
      self.resized = false
      self.nbSamples = 0
      self.cacheFileName = args.cacheFile
      if paths.filep(self.cacheFileName) then
         self:open(self.cacheFileName)
      elseif args and args.dataSetFolder then
         self:load(args)
      end
   end

   function DataSet:size() 
      return self.nbSamples 
   end

   function DataSet:load(...)
      local args = {...}
      -- parse args      
      local dataSetFolder = args.dataSetFolder or error('please provide a path to dataset')
      local nbSamplesRequired = args.nbSamplesRequired or 'all'
      local chanels = args.chanels or 1
      self.cacheFileName = args.cacheFile or self.cacheFileName

      -- Clear current dataset
      self:emptySet()

      -- Then try to find if cache file exists
      -- the base name of this file can be provided by useCacheFile()
      -- and the suffixe is the nb of samples needed, 'all' if not specified
      local fileName
      local datasetLoadedFromFile = false
      if (self.cacheFileName ~= nil) then
         fileName = self.cacheFileName .. '-' .. nbSamplesRequired
         if toolBox.fileExists(fileName) then
            -- File found
            print('# Loading samples from cached file ' .. fileName)
            f = torch.DiskFile(fileName, 'rw')
            self:read(f)
            f.close(f)
            datasetLoadedFromFile = true
         end
      end

      -- If dataset couldn't be loaded from cache, load it
      if (datasetLoadedFromFile == false) then
         self:append(args)
         -- if cache name given, create it now
         if (fileName ~= nil) then
            print('# Dumping dataset to cache file ' .. fileName .. ' for fast retrieval')
            f = torch.DiskFile(fileName, 'rw')
            self:write(f)
            f.close(f)
         end
      end
   end

   function DataSet:emptySet(dataSetFolder)
      for i = 1,table.getn(self) do
         self[i] = nil
      end
      self.nbSamples = 0
   end

   function DataSet:apply(toapply)
      print('# Applying function to dataset')
      for i=1,self.nbSamples do
         toolBox.dispProgress(i, self.nbSamples)
         self[i][1] = toapply(self[i][1])
      end
   end
   
   function DataSet:cropAndResize(side)
      for i=1,self.nbSamples do
         local newSample = torch.Tensor(side, side, 1)
         local initSide = math.min(self[i][1]:size()[1], self[i][1]:size()[2])
         local x1 = math.floor((self[i][1]:size()[1] - initSide) / 2)
         local y1 = math.floor((self[i][1]:size()[2] - initSide) / 2)
         local x2 = x1 + initSide
         local y2 = y1 + initSide
         image.crop(self[i][1],newSample,x1,y1,x2,y2)
         self[i][1] = newSample
      end
   end

   function DataSet:add(args)
      local input = args.input
      local output = args.output
      self.nbSamples = self.nbSamples + 1
      self[self.nbSamples] = {input, output}
   end

   function DataSet:append(...)
      -- parse args
      local args, dataSetFolder, chanels, nbSamplesRequired, useLabelPiped, useDirAsLabel, nbLabels    
         = toolBox.unpack({...},
                          'DataSet:append', 'append a folder to the dataset object',
                          {arg='dataSetFolder', type='string', help='size of history buffer (e.g. past frames kept)', req=true},
                          {arg='chanels', type='number', help='number of chanels for the image to load', default=3},
                          {arg='nbSamplesRequired', type='number', help='max number of samples to load'},
                          {arg='useLabelPiped', type='boolean', help='flag to use the filename as output value',default=false},
                          {arg='useDirAsLabel', type='boolean', help='flag to use the directory as label',default=true},
                          {arg='nbLabels', type='boolean', help='how many classes (goes with useDirAsLabel)', default=1})
      -- data size:


      local files = paths.dir(args.dataSetFolder)
      table.sort(files)
      print('# Loading samples from ' .. args.dataSetFolder .. '/')

      

      -- nb of samples to load:
      local toLoad = table.getn(files)
      if (nbSamplesRequired ~= nil and nbSamplesRequired ~= 'all') then
         toLoad = math.min(toLoad, nbSamplesRequired)
      end
      local loaded = 0
      local dir = toolBox.split(dataSetFolder,"/")
      -- take last element
      dir = dir[#dir]+0

      for k,file in pairs(files) do
         local input
         local rawOutput

         -- disp progress
         toolBox.dispProgress(k, toLoad)

         if (string.find(file,'.png')) then
            -- load the PNG into a new Tensor
            pathToPng = paths.concat(dataSetFolder, file)
            input = image.loadPNG(pathToPng,chanels)

            -- parse the file name and set the ouput from it
            rawOutput = toolBox.split(string.gsub(file, ".png", ""),'|')

         elseif (string.find(file,'.p[pgn]m')) then
            -- load the PPM into a new Tensor
            pathToPpm = paths.concat(dataSetFolder, file)
            input = image.loadPPM(pathToPpm,chanels)

            -- parse the file name and set the ouput from it
            rawOutput = toolBox.split(string.gsub(file, ".p[pgn]m", ""),'|')
         end
         
         -- if image loaded then add into the set
         if (useLabelPiped and input and rawOutput) then
            table.remove(rawOutput,1) --remove file ID
            -- put input in 3D tensor
            input:resize(input:size()[1], input:size()[2], chanels)
            -- and generate output
            local output = torch.Tensor(table.getn(rawOutput), 1)
            for i,v in ipairs(rawOutput) do
               output[i][1]=v
            end

            -- add input/output in the set
            self.nbSamples = self.nbSamples + 1
            self[self.nbSamples] = {input, output}

            loaded = loaded + 1
            if (loaded == toLoad) then
               break
            end
         elseif (useDirAsLabel and input) then
            if(nbLabels < dir) then
               error("the nbLabels is smaller than the directory id")               
            end
            local output = torch.Tensor(nbLabels,1):fill(0)
            output[dir][1]=1
            
            -- add input/output in the set
            self.nbSamples = self.nbSamples + 1
            self[self.nbSamples] = {input, output}
            loaded = loaded + 1
            if (loaded == toLoad) then
               break
            end
         end
--          else
--             print("input",input,file)
--             error("Unknow method for label, either set 'useLabelPiped' or 'useDirAsLabel'")
--          end

      end
   end

   function DataSet:appendDataSet(dataset)
      print("# Merging dataset of size = "..dataset:size()..
                               " into dataset of size = "..self:size())
      for i = 1,dataset:size() do
         self.nbSamples = self.nbSamples + 1
         self[self.nbSamples] = {}
         self[self.nbSamples][1] = torch.Tensor(dataset[i][1]):copy(dataset[i][1])
         if (dataset[i][2] ~= nil) then
            self[self.nbSamples][2] = torch.Tensor(dataset[i][2]):copy(dataset[i][2])
         end
      end
   end

   function DataSet:popSubset(args)
      -- parse args
      local nElement = args.nElement
      local ratio = args.ratio or 0.1
      local subset = args.outputSet or DataSet()

      -- get nb of samples to pop
      local start_index
      if (nElement ~= nil) then
         start_index = self:size() - nElement + 1
      else
         start_index = math.floor((1-ratio)*self:size()) + 1
      end
      
      -- info
      print('# Popping ' .. self:size() - start_index + 1 .. ' samples dataset')
      
      -- extract samples
      for i = self:size(), start_index, -1 do
         subset.nbSamples = subset.nbSamples + 1
         subset[subset.nbSamples] = {}
         subset[subset.nbSamples][1] = torch.Tensor(self[i][1]):copy(self[i][1])
         subset[subset.nbSamples][2] = torch.Tensor(self[i][2]):copy(self[i][2])
         self[i] = nil
         self.nbSamples = self.nbSamples - 1
      end
      
      -- return network
      return subset
   end

   function DataSet:resize(w,h)
      error( toolBox.NOT_IMPLEMENTED )
      self.resized = true
   end

   function DataSet:shuffle()
      if (self.nbSamples == 0) then
         print('Warning, trying to shuffle empty Dataset, no effect...')
         return
      end
      local n = self.nbSamples
      
      while n > 2 do
         local k = math.random(n)
         -- swap elements
         self[n], self[k] = self[k], self[n]
         n = n - 1
      end
   end

   function DataSet:display(args) -- opt args : scale, nbSamples
      local args = args or {}
      -- arg list:
      local min, max, nbSamples, scale, w
      local title = args.title or args.legend or 'dataset'
      local resX = 800
      local resY = 600
      min = args.min
      max = args.max
      nbSamples = args.nbSamples or self.nbSamples
      scale = args.scale
      title = args.title or title
      w = self.window or gfx.Window(resX, resY, title)
      resX = args.resX or resX
      resY = args.resY or resY

      local step_x = 0
      local step_y = 0
      self.window = w

      if (scale == nil) then
         --get the best scale to feet all data
         local sizeX = self[1][1]:size()[1] 
         local sizeY = self[1][1]:size()[2]
         scale = math.sqrt(resX*resY/ (sizeX*sizeY*nbSamples)) 
      end

      for i=1,nbSamples do
         local idx = i
         if args.nbSamples then
            idx = math.random(1,self.nbSamples)
         end
         if (step_x >= resX) then
            step_x = 0
            step_y = step_y + self[idx][1]:size()[2]*scale
            if (step_y >= resY) then
               break
            end
         end
         local tmp  = image.scaleForDisplay{tensor=self[idx][1], min=min, max=max}
         w:blit(tmp, scale, step_x, step_y, title)
         step_x = step_x + self[idx][1]:size()[1]*scale
      end
   end
   
   function DataSet:__tostring__()
      str = 'DataSet\n'
      str = str..'nb elements : '..self.nbSamples
      if self[1] then 
         local inputSize = self[1][1]:size()
         local outputSize = self[1][2]:size()
         str = str..'\ninput '..inputSize[1]..'x'..inputSize[2]
         if (self[1][2]:nDimension() > 1) then
            str = str..'\noutput '..outputSize[1]..'x'..outputSize[2]
         end
      end
      return str
   end

   function DataSet:useCacheFile(fileName)
      self.cacheFileName = fileName
   end


   function DataSet:save(fileName)
      local fileName = fileName or self.fileName
      self.fileName = fileName
      print('# Saving DataSet to:',fileName)
      local file = torch.DiskFile(fileName, 'w')
      self:write(file)
      file:close()
   end

   function DataSet:open(fileName)
      local fileName = fileName or self.fileName
      self.fileName = fileName
      print('# Loading DataSet from File:',fileName)
      local file = torch.DiskFile(fileName, 'r')
      self:read(file)
      file:close()
      print('# '..self.nbSamples..' samples loaded')
   end

   function DataSet:write(file)
      file:writeBool(self.resized)
      file:writeInt(self.nbSamples)
      -- write all the samples
      for i = 1,self.nbSamples do
         file:writeObject(self[i])
      end
   end

   function DataSet:read(file)
      self.resized = file:readBool()
      self.nbSamples = file:readInt()
      -- read all the samples
      for i = 1,self.nbSamples do
         self[i] = file:readObject()
      end
   end

end

-- dofile('dataSet.lua')
-- a = DataSet()
-- a:load('set1')