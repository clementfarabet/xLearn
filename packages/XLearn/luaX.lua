
-- load XLearn
require 'XLearn'

-- message
print("luaFlow  Copyright (c) 2010 New York University, Yale University\n"..
      "neuFlow  Copyright (c) 2009-2010 New York University, Yale University\n"..
      "XLearn   Copyright (c) 2009-2010 New York University, Yale University\n"..
      "Torch    Copyright (c) 2006-2008 NEC Laboratories America\n"..
      "Torch    Copyright (c) 2006 IDIAP Research Institute\n"..
      "Torch    Copyright (c) 2001-2004 IDIAP Research Institute")

-- define a global clearall()
function clearall()
   for k,v in pairs(_G) do
      local protected = false
      local lib = false
      for i,p in ipairs(_protect_) do
	 if k == p then protected = true end
      end
      for p in pairs(package.loaded) do
	 if k == p then lib = true end
      end
      if not protected then
	 _G[k] = nil
         if lib then package.loaded[k] = nil end
      end
   end
   collectgarbage()
end

-- define a simple clear(var)
function clear(var)
   _G[var] = nil
   collectgarbage()
end

-- print global variables
function who()
   local user = {}
   local libs = {}
   for k,v in pairs(_G) do
      local protected = false
      local lib = false
      for i,p in ipairs(_protect_) do
	 if k == p then protected = true end
      end
      for p in pairs(package.loaded) do
	 if k == p and p ~= '_G' then lib = true end
      end
      if lib then
         table.insert(libs, k)
      elseif not protected then
	 user[k] =  _G[k]
      end
   end
   print('')
   print('Global Libs:')
   print(libs)
   print('')
   print('Global Vars:')
   print(user)
   print('')
end

-- remember startup variables + XLearn variables
_G._protect_ = {'_protect_'}
for k,v in pairs(_G) do
   table.insert(_G._protect_, k)
end
