local SVM = torch.class('svm.SVM')

function SVM:__init(kernel)
   self.kernel = kernel
   self.svs = {}
   self.alpha = torch.Tensor()
   self.bias = 0
end

function SVM:write(file)
   file:writeObject(self.kernel)
   file:writeObject(self.svs)
   file:writeObject(self.alpha)
   file:writeDouble(self.bias)
end

function SVM:read(file)
   self.kernel = file:readObject()
   self.svs = file:readObject()
   self.alpha = file:readObject()
   self.bias = file:readDouble()
end

function SVM:train(...)
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

   local _options_ = {
      sumflag=true, 
      verbosity=true,
      epskt=true,
      epsgr=true,
      maxst=true,
      maxcachesize=true
   }


   options = options or {}
   
   local n
   if dataistensor then
      n = data:size(2)
   else
      n = data:size()
   end
   local solver = svm.QPSolver(n)
   if options.sumflag == false then
      self.bias = 0
      solver:sumflag(false)
   end
   for k,v in pairs(options) do
      if _options_[k] then
         solver[k](solver, v)
      else
         error('invalid option: ' .. k)
      end
   end
   
   local cmin = torch.Tensor(n)
   local cmax = torch.Tensor(n)
   if type(C) == 'number' then
      for i=1,n do
         if y[i] > 0 then
            cmin[i] = 0
            cmax[i] = C
         else
            cmin[i] = -C
            cmax[i] = 0
         end
      end
   else
      for i=1,n do
         if y[i] > 0 then
            cmin[i] = 0
            cmax[i] = C[i]
         else
            cmin[i] = -C[i]
            cmax[i] = 0
         end
      end
   end

   solver:cmin(cmin)
   solver:cmax(cmax)
   solver:b(y)

   if dataistensor then
      solver:run(data, self.kernel)
   else
      local kernel = self.kernel
      local function closure(i, j)
         return kernel:eval(data[i][1], data[j][1])
      end
      solver:run(closure)
   end

   if not options.sumflag then
      self.bias = (solver:gmin()+solver:gmax())/2
   end

   local alpha = solver:x()
   local nsv = 0
   local nsvbound = 0
   for i=1,n do
      if alpha[i] ~= 0 then
         nsv = nsv + 1
      end
      if y[i] > 0 and alpha[i] == cmax[i] then nsvbound = nsvbound + 1 end 
      if y[i] < 0 and alpha[i] == cmin[i] then nsvbound = nsvbound + 1 end     
   end

   print('[number of SVs = ' .. nsv .. ']') 
   print('[number of SVs at bound = ' .. nsvbound .. ']')
  
   for i=1,#self.svs do
      self.svs[i] = nil
   end
   self.alpha:resize(nsv)
   nsv = 0
   for i=1,n do
      if alpha[i] ~= 0 then
         local sv
         if dataistensor then
            sv = data:select(2, i)
            table.insert(self.svs, torch.Tensor(sv:size()):copy(sv))
         else
            sv = data[i][1]
            table.insert(self.svs, sv)
         end
         nsv = nsv + 1
         self.alpha[nsv] = alpha[i]
      end
   end
end
