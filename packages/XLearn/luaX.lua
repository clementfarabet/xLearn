
-- load XLearn
require 'XLearn'

-- globals window ?
_globals_enabled_ = false

-- purge args
arg = nil

-- message
local msg = [[
luaFlow  Copyright (c) 2010-2011 New York University, Yale University
neuFlow  Copyright (c) 2009-2011 New York University, Yale University
XLearn   Copyright (c) 2009-2011 New York University, Yale University
Torch    Copyright (c) 2006-2008 NEC Laboratories America
Torch    Copyright (c) 2006 IDIAP Research Institute
Torch    Copyright (c) 2001-2004 IDIAP Research Institute]]

-- welcome
print(msg)

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
   _globalswinupdate_()
end

-- define a simple clear(var)
function clear(var)
   _G[var] = nil
   collectgarbage()
   _globalswinupdate_()
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

-- load global var window
if qt and qt.qConsole.captureOutput and _globals_enabled_ then
   toolBox.useQT()
   local pathui = paths.concat(paths.install_lua_path, 'XLearn/luaX.ui')
   function _globalswinupdate_ ()
      local savedgeom
      local savedtab
      if _globalswin_ then 
         savedgeom = _globalswin_.geometry
         savedtab = _globalswin_.tabw.currentIndex
         qt.qApp:writeSettings('_globalswin_',savedgeom)
         qt.qApp:writeSettings('_globalswin_tab_',_globalswin_.tabw.currentIndex)
      else
         savedgeom = qt.qApp:readSettings('_globalswin_')
         savedtab = qt.qApp:readSettings('_globalswin_tab_') or 0
         _globalswin_ = qtuiloader.load(pathui)
         _globalswin_.windowOpacity = 0.95
         _globalswin_:show()
      end
      if savedgeom then
         _globalswin_.geometry = savedgeom
         _globalswin_.tabw:setCurrentIndex(savedtab)
      end
      local user = {}
      local libs = {}
      local sys = {}
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
            user[k] = _G[k]
         else
            sys[k] = _G[k]
         end
      end

      -- LIBS
      local qttable = {}
      for i,v in ipairs(libs) do
         table.insert(qttable, qt.QString('1'))
         table.insert(qttable, qt.QString(v))
      end
      qt.qcall(_globalswin_.treemodules, _globalswin_.treemodules.clear, 
               _globalswin_.treemodules)
      qt.qcall(_globalswin_.treemodules, _globalswin_.treemodules.fromtable,
               _globalswin_.treemodules, qttable, 1)

      -- AVAILABLE
      qttable = {}
      for lib in paths.files(paths.concat(paths.install_share,'lua/5.1/')) do
         if lib ~= '.' and lib ~= '..' then
            local loaded = false
            for i,l in ipairs(libs) do
               if lib == l then 
                  loaded = true
                  break
               end
            end
            if not loaded then
               table.insert(qttable, qt.QString('1'))
               table.insert(qttable, qt.QString(lib))
            end
         end
      end
      qt.qcall(_globalswin_.treemodulesavail, _globalswin_.treemodulesavail.clear, 
               _globalswin_.treemodulesavail)
      qt.qcall(_globalswin_.treemodulesavail, _globalswin_.treemodulesavail.fromtable,
               _globalswin_.treemodulesavail, qttable, 1)

      -- USER VARS
      qttable = {}
      for k,v in pairs(user) do
         local mt = getmetatable(v) or {}
         table.insert(qttable, qt.QString('1'))
         table.insert(qttable, qt.QString(k))
         if type(v) == 'string' then
            local str = v:gsub('\n','\\n')
            table.insert(qttable, qt.QString(str))
            table.insert(qttable, qt.QString('string'))
         elseif type(v) == 'function' then
            table.insert(qttable, qt.QString('f()'))
            table.insert(qttable, qt.QString('function'))
         elseif type(v) == 'number' or type(v) == 'boolean' then
            table.insert(qttable, qt.QString(v))
            table.insert(qttable, qt.QString(type(v)))         
         elseif mt.__typename == 'torch.Tensor' then
            if v:nDimension() == 0 then
               table.insert(qttable, qt.QString('[]'))
            else
               local dims = '[' .. v:size(1)
               for i = 2,v:nDimension() do
                  dims = dims .. 'x' .. v:size(i)
               end
               dims = dims .. ']'
               table.insert(qttable, qt.QString(dims))
            end
            table.insert(qttable, qt.QString(mt.__typename))
         elseif mt.__typename then
            table.insert(qttable, qt.QString('<>'))
            table.insert(qttable, qt.QString(mt.__typename))
         elseif type(v) == 'table' then
            table.insert(qttable, qt.QString('{...}'))
            table.insert(qttable, qt.QString(type(v)))
         else
            table.insert(qttable, qt.QString('<>'))
            table.insert(qttable, qt.QString(type(v)))
         end
      end
      qt.qcall(_globalswin_.treeglobals, _globalswin_.treeglobals.clear, 
               _globalswin_.treeglobals)
      qt.qcall(_globalswin_.treeglobals, _globalswin_.treeglobals.fromtable,
               _globalswin_.treeglobals, qttable, 3)

      -- SYSTEM VARS
      qttable = _protectedvars_
      if not qttable then
         qttable = {}
         for k,v in pairs(sys) do
            local mt = getmetatable(v) or {}
            table.insert(qttable, qt.QString('1'))
            table.insert(qttable, qt.QString(k))
            if type(v) == 'string' then
               local str = v:gsub('\n','\\n')
               table.insert(qttable, qt.QString(str))
               table.insert(qttable, qt.QString('string'))
            elseif type(v) == 'function' then
               table.insert(qttable, qt.QString('f()'))
               table.insert(qttable, qt.QString('function'))
            elseif type(v) == 'number' or type(v) == 'boolean' then
               table.insert(qttable, qt.QString(v))
               table.insert(qttable, qt.QString(type(v)))
            elseif mt.__typename then
               table.insert(qttable, qt.QString('<>'))
               table.insert(qttable, qt.QString(mt.__typename))
            elseif type(v) == 'table' then
               table.insert(qttable, qt.QString('{...}'))
               table.insert(qttable, qt.QString(type(v)))
            else
               table.insert(qttable, qt.QString('<>'))
               table.insert(qttable, qt.QString(type(v)))
            end
         end
         _protectedvars_ = qttable
      end
      qt.qcall(_globalswin_.treesystem, _globalswin_.treesystem.clear, 
               _globalswin_.treesystem)
      qt.qcall(_globalswin_.treesystem, _globalswin_.treesystem.fromtable, 
               _globalswin_.treesystem, qttable, 3)
   end
else
   function _globalswinupdate_ ()
   end
end

-- remember startup variables + XLearn variables
_G._protect_ = {'_protect_','_globalswin_','_protectedvars_'}
for k,v in pairs(_G) do
   table.insert(_G._protect_, k)
end

-- setup a hook to autorefresh tree's content
qt.connect(qt.qEngine,'evalDone()',_globalswinupdate_)

-- preload some useful variables
lena = image.lena()
graylena = image.rgb2y(image.lena())

-- refresh globals again
_globalswinupdate_()

-- create hooks
if qt and qt.qConsole.captureOutput and _globals_enabled_ then
   -- setup hooks to dump the content of selected variables
   local hook_modules 
      = function ()
           local sel = _globalswin_.treemodules:selected()
           sel = sel:totable()
           local var = sel[1]:tostring()
           print('module ' .. var .. '\'s content:')
           print(_G[var])
           print('')
        end
   local hook_avail
      = function ()
           local sel = _globalswin_.treemodulesavail:selected()
           sel = sel:totable()
           local var = sel[1]:tostring()
           print('loading module ' .. var .. ':')
           print(xrequire(var))
           _globalswinupdate_()
        end
   local hook_globals 
      = function ()
           local sel = _globalswin_.treeglobals:selected()
           sel = sel:totable()
           local var = sel[1]:tostring()
           io.write(var .. ' = ')
           print(_G[var])
           print('')
        end
   qt.connect(_globalswin_.treemodules,
              'itemPressed(QTreeWidgetItem*,int)',
              hook_modules)
   qt.connect(_globalswin_.treemodulesavail,
              'itemPressed(QTreeWidgetItem*,int)',
              hook_avail)
   qt.connect(_globalswin_.treeglobals,
              'itemPressed(QTreeWidgetItem*,int)',
              hook_globals)

   -- setup hooks on buttons
   local hook_display
      = function () 
           local sel = _globalswin_.treeglobals:selected()
           sel = sel:totable()
           if sel[1] then
              local var = sel[1]:tostring()
              local g = _G[var]
              local mt = getmetatable(g) or {}
              if mt.__typename == 'torch.Tensor' then
                 if g:nDimension() == 2 or g:nDimension() == 3 or g:nDimension() == 4 then
                    image.display(g)
                 end
              elseif type(g) == 'table' and g.__show then
                 g:__show()
              end
           end
        end
   local hook_clear = 
      function () 
         local sel = _globalswin_.treeglobals:selected()
         sel = sel:totable()
         if sel[1] then
            local var = sel[1]:tostring()
            clear(var)
         end
      end
   local hook_help =
      function ()
         local sel = _globalswin_.treemodules:selected()
         sel = sel:totable()
         if sel[1] then
            local package = sel[1]:tostring()
            local path = paths.concat(paths.install_share, 'torch/html', package, 'index.html')
            if paths.filep(path) then
               qtide.browser(path)
            end
         end
      end
   qt.connect(qt.QtLuaListener(_globalswin_.tabw.display),
              'sigMousePress(int,int,QByteArray,QByteArray,QByteArray)', 
              hook_display)
   qt.connect(qt.QtLuaListener(_globalswin_.tabw.clear), 
              'sigMousePress(int,int,QByteArray,QByteArray,QByteArray)', 
              hook_clear)
   qt.connect(qt.QtLuaListener(_globalswin_.tabw.clearall), 
              'sigMousePress(int,int,QByteArray,QByteArray,QByteArray)', 
              clearall)
   qt.connect(qt.QtLuaListener(_globalswin_.tabw.help), 
              'sigMousePress(int,int,QByteArray,QByteArray,QByteArray)', 
              hook_help)
end
