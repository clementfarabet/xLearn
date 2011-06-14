-- 
-- Note: this bit of code is a simple wrapper around the matlab
-- library (load/save functions for now)
--
-- Wrapper: Clement Farabet.
-- 

-- load C lib
require 'libmatlab'

------------------------------------------------------------
-- helps
local help = {
---
load = [[Loads a .mat file into a Lua table. 
Each mex Array is converted into a torch.Tensor.
A table with all the loaded variables is returned:
  {varname1 = var1, varname2 = var2, ... } ]]
,
save = [[Exports variables to a .mat file.
Supported now:
  > tensor1 = torch.Tensor(...)
  > tensor2 = torch.Tensor(...)
  > tensor3 = torch.Tensor(...)
  > matlab.save('output.mat', tensor1)
  > -- OR
  > list = {myvar = tensor1, othervar = tensor2, thisvar = tensor3}
  > matlab.save('output.mat', list) ]]
}

------------------------------------------------------------
-- create package
require 'XLearn'
matlab = {}

-- load
matlab.load = function(path)
                 if not path then
                    xerror('please provide a path','matlab.load',help.load)
                 end
                 return libmatlab.load(path)
              end

-- save
matlab.save = function(path,vars)
                 if not path then
                    xerror('please provide a path','matlab.save',help.save)
                 end
                 if type(vars) == 'userdata' and torch.typename(vars) == 'torch.Tensor' then
                    local tensor = torch.Tensor():resizeAs(vars):copy(vars)
                    libmatlab.saveTensor(path,tensor)

                 elseif type(vars) == 'table' then
                    for i,v in ipairs(vars) do
                       if v then xerror('can only export table of named variables, e.g. {x=..., y=...}') end
                    end
                    for _,v in pairs(vars) do
                       if type(v) ~= 'userdata' or torch.typename(v) ~= 'torch.Tensor' then 
                          xerror('can only export table of torch.Tensors')
                       end
                    end
                    libmatlab.saveTable(path,vars)

                 else
                    xerror('cannot export given variables','matlab.save',help.save)
                 end
              end

-- return package
return matlab
