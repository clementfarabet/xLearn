-- print mode:
torch.printStyle = 'memory'

-- max size (to avoid lenghty printouts)
torch.printMax = 30

-- tostring() functions for Tensor and Storage
local function Storage__printformat(self)
   local intMode = true
   local type = torch.typename(self)
   if type == 'torch.FloatStorage' or type == 'torch.DoubleStorage' then
      for i=1,self:size() do
         if self[i] ~= math.ceil(self[i]) then
            intMode = false
            break
         end
      end
   end
   local tensor = torch.Tensor(torch.DoubleStorage(self:size()):copy(self), 1, self:size()):abs()
   local expMin = tensor:min()
   if expMin ~= 0 then
      expMin = math.floor(math.log10(expMin)) + 1
   end
   local expMax = tensor:max()
   if expMax ~= 0 then
      expMax = math.floor(math.log10(expMax)) + 1
   end

   local format
   local scale
   local sz
   if intMode then
      if expMax > 9 then
         format = "%11.4e"
         sz = 11
      else
         format = "%SZd"
         sz = expMax + 1
      end
   else
      if expMax-expMin > 4 then
         format = "%SZ.4e"
         sz = 11
         if math.abs(expMax) > 99 or math.abs(expMin) > 99 then
            sz = sz + 1
         end
      else
         if expMax > 5 or expMax < 0 then
            format = "%SZ.4f"
            sz = 7
            scale = math.pow(10, expMax-1)
         else
            format = "%SZ.4f"
            if expMax == 0 then
               sz = 7
            else
               sz = expMax+6
            end
         end
      end
   end
   format = string.gsub(format, 'SZ', sz)
   if scale == 1 then
      scale = nil
   end
   return format, scale, sz
end

local function Storage__tostring(self)
   local str = '\n'
   local format,scale = Storage__printformat(self)
   if scale then
      str = str .. string.format('%g', scale) .. ' *\n'
      for i = 1,self:size() do
         str = str .. string.format(format, self[i]/scale) .. '\n'
      end
   else
      for i = 1,self:size() do
         str = str .. string.format(format, self[i]) .. '\n'
      end
   end
   str = str .. '[' .. torch.typename(self) .. ' of size ' .. self:size() .. ']\n'
   return str
end

rawset(torch.getmetatable('torch.ByteStorage'), '__tostring__', Storage__tostring)
rawset(torch.getmetatable('torch.CharStorage'), '__tostring__', Storage__tostring)
rawset(torch.getmetatable('torch.ShortStorage'), '__tostring__', Storage__tostring)
rawset(torch.getmetatable('torch.IntStorage'), '__tostring__', Storage__tostring)
rawset(torch.getmetatable('torch.LongStorage'), '__tostring__', Storage__tostring)
rawset(torch.getmetatable('torch.FloatStorage'), '__tostring__', Storage__tostring)
rawset(torch.getmetatable('torch.DoubleStorage'), '__tostring__', Storage__tostring)

local function Tensor__printMatrix(self, indent)
   local format,scale,sz = Storage__printformat(self:storage())
   if format:sub(2,4) == 'nan' then format = '%f' end
   local str = '' 
   if scale then
      str = string.format('%g', scale) .. ' *\n'
   else
      scale = 1
   end
   indent = indent or ''
   str = str .. indent .. '['
   if torch.printStyle == 'memory' then
      for j = 1,self:size(2) do
         str = str .. '['
         for i = 1,self:size(1) do
            str = str .. string.format(format, self:select(2,j)[i]/scale)
            if i == torch.printMax then
               str = str .. ' ...'
               break
            end
            if i ~= self:size(1) then
               str = str .. ' '
            end
         end
         if j == torch.printMax then
            str = str .. ']\n' .. indent .. ' ... ]\n'
            break
         end
         if j == self:size(2) then
            str = str .. ']]\n'
         else
            str = str .. ']\n ' .. indent
         end
      end
   elseif torch.printStyle == 'matrix' then
      for j = 1,self:size(1) do
         str = str .. '['
         for i = 1,self:size(2) do
            str = str .. string.format(format, self:select(1,j)[i]/scale)
            if i == torch.printMax then
               str = str .. ' ...'
               break
            end
            if i ~= self:size(2) then
               str = str .. ' '
            end
         end
         if j == torch.printMax then
            str = str .. ']\n' .. indent .. ' ... ]\n'
            break
         end
         if j == self:size(1) then
            str = str .. ']]\n'
         else
            str = str .. ']\n ' .. indent
         end
      end
   else
      str = '[...]\n'
   end
   return str
end

local function Tensor__printTensor(self)
   local counter = torch.LongStorage(self:nDimension())
   local str = ''
   local finished
   counter:fill(1)
   counter[3] = 0
   if torch.printStyle ~= 'matrix' and torch.printStyle ~= 'memory' then
      str = 'torch.printStyle should be one of: matrix | memory \n'
   end
   while true do
      for i=3,self:nDimension() do
         counter[i] = counter[i] + 1
         if counter[i] > self:size(i) then
            if i == self:nDimension() then
               finished = true
               break
            end
            counter[i] = 1
         else
            break
         end
      end
      if finished then
         break
      end
      if str ~= '' then
         str = str .. '\n'
      end
      str = str .. '(:,:'
      local tensor = self
      for i=3,self:nDimension() do
         tensor = tensor:select(3, counter[i])
         str = str .. ',' .. counter[i]
      end
      str = str .. ') = \n'
      str = str .. Tensor__printMatrix(tensor, ' ')
   end
   return str
end

local function Tensor__tostring(self)
   local str = '\n'
   if self:nDimension() == 0 then
      str = str .. '[' .. torch.typename(self) .. ' with no dimension]\n'
   else
      local tensor = torch.Tensor():resize(self:size()):copy(self)
      if tensor:nDimension() == 1 then
         local format,scale,sz = Storage__printformat(tensor:storage())
         if scale then
            str = str .. string.format('%g', scale) .. ' *\n'
            for i = 1,tensor:size(1) do
               str = str .. string.format(format, tensor[i]/scale) .. '\n'
            end
         else
            for i = 1,tensor:size(1) do
               str = str .. string.format(format, tensor[i]) .. '\n'
            end
         end
         str = str .. '[' .. torch.typename(self) .. ' of dimension ' .. tensor:size(1) .. ']\n'
      elseif tensor:nDimension() == 2 then
         str = str .. Tensor__printMatrix(tensor)
         str = str .. '[' .. torch.typename(self) .. ' of dimension ' .. tensor:size(1) .. 'x' .. tensor:size(2) .. ']\n'
      else
         str = str .. Tensor__printTensor(tensor)
         str = str .. '[' .. torch.typename(self) .. ' of dimension '
         for i=1,tensor:nDimension() do
            str = str .. tensor:size(i) 
            if i ~= tensor:nDimension() then
               str = str .. 'x'
            end
         end
         str = str .. ']\n'
      end
   end
   return str
end
rawset(torch.getmetatable('torch.ByteTensor'), '__tostring__', Tensor__tostring)
rawset(torch.getmetatable('torch.CharTensor'), '__tostring__', Tensor__tostring)
rawset(torch.getmetatable('torch.ShortTensor'), '__tostring__', Tensor__tostring)
rawset(torch.getmetatable('torch.IntTensor'), '__tostring__', Tensor__tostring)
rawset(torch.getmetatable('torch.LongTensor'), '__tostring__', Tensor__tostring)
rawset(torch.getmetatable('torch.FloatTensor'), '__tostring__', Tensor__tostring)
rawset(torch.getmetatable('torch.Tensor'), '__tostring__', Tensor__tostring)
