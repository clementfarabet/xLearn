local oaaSVM = torch.class('svm.oaaSVM')

function oaaSVM:__init(kernel)
   self.kernel = kernel
   self.svms = {}
end

function oaaSVM:write(file)
   file:writeObject(self.kernel)
   file:writeObject(self.svms)
end

function oaaSVM:read(file)
   self.kernel = file:readObject()
   self.svms = file:readObject()
end

function oaaSVM:forward(input)
   local predcl, pred = nil, -math.huge
   local output = {}
   for cl,svm in pairs(self.svms) do
      local z = svm:forward(input)
      if z > pred then
         pred = z
         predcl = cl
      end
      output[cl] = z
   end
   return predcl, output
end

function oaaSVM:train(...)
   local data, y, C, options
   local dataistensor
   if torch.typename(arg[1]) == 'torch.Tensor' then
      dataistensor = true
      data = arg[1]
      y = arg[2]
      C = arg[3]
      options = arg[4]
   else
      dataistensor = false
      data = arg[1]
      y = torch.Tensor(data:size())
      for i=1,data:size() do
         y[i] = data[i][2]
      end
      C = arg[2]
      options = arg[3]
   end

   local classes = {}
   for i=1,y:size(1) do
      classes[y[i]] = true
   end

   local _y_ = torch.Tensor(y:size(1))
   local ncl = 0
   for cl,_ in pairs(classes) do
      ncl = ncl + 1
   end
   for cl,_ in pairs(classes) do
      print('[remaining classes to training: ' .. ncl .. ']')
      ncl = ncl - 1
      self.svms[cl] = svm.SVM(self.kernel)
      for i=1,y:size(1) do
         if y[i] == cl then
            _y_[i] = 1
         else
            _y_[i] = -1
         end
      end
      
      if dataistensor then
         self.svms[cl]:train(data, _y_, C, options)
      else
         local _data_ = {}
         function _data_:size()
            return data:size()
         end
         setmetatable(_data_, {__index = function(self, idx)
                                            local input = data[idx][1]
                                            local target = _y_[idx]
                                            return {input, target}
                                         end})
         self.svms[cl]:train(_data_, C, options)
      end
   end
end
