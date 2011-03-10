--------------------------------------------------------------------------------
-- A collection of tools/helpers for daily problems.
-- 
-- Automatically loaded with XLearn.
--
-- @release 2010 Benoit Corda, Clement Farabet
--------------------------------------------------------------------------------
local _GLOB_ = _G
module('toolBox', package.seeall)

--------------------------------------------------------------------------------
-- Error messages
--------------------------------------------------------------------------------
NOT_IMPLEMENTED = "Not implemented yet"

--------------------------------------------------------------------------------
-- Loads QT environment
-- Has for side effect to set the variable toolBox.QT to true.
--------------------------------------------------------------------------------
qtloaded = false
function useQT()
   if (qtloaded == false) then
      _GLOB_.require 'qtuiloader'
      _GLOB_.require 'qtwidget'
      _GLOB_.require 'qt'
      qtloaded = true   
   end
end

--------------------------------------------------------------------------------
-- standard usage function: used to display automated help for functions
--
-- @param funcname     function name
-- @param description  description of the function
-- @param example      usage example
-- @param ...          [optional] arguments
--------------------------------------------------------------------------------
function usage(funcname, description, example, ...)
   local c = COLORS
   local str = c.magenta .. '\n'
   local str = str .. '++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n'
   str = str .. 'NAME:\n' .. funcname .. '\n'
   if description then
      str = str .. '\nDESC:\n' .. description .. '\n'
   end
   if example then
      str = str .. '\nEXAMPLE:\n' .. example .. '\n'
   end
   str = str .. '\nUSAGE:\n'

   -- named arguments:
   local args = {...}
   if args[1].arg then
      str = str .. funcname .. '{\n'
      for i,param in ipairs{...} do
         local key
         if param.req then
            key = '    ' .. param.arg .. ' = ' .. param.type
         else
            key = '    [' .. param.arg .. ' = ' .. param.type .. ']'
         end
         -- align:
         while key:len() < 40 do
            key = key .. ' '
         end
         str = str .. key .. '-- ' .. param.help 
         if param.default or param.default == false then
            str = str .. '  [default = ' .. tostring(param.default) .. ']'
         elseif param.defaulta then
            str = str .. '  [default == ' .. param.defaulta .. ']'
         end
         str = str.. '\n'
      end
      str = str .. '}\n'

   -- unnamed args:
   else
      str = str .. funcname .. '(\n'
      for i,param in ipairs{...} do
         local key
         if param.req then
            key = '    ' .. param.type
         else
            key = '    [' .. param.type .. ']'
         end
         -- align:
         while key:len() < 40 do
            key = key .. ' '
         end
         str = str .. key .. '-- ' .. param.help .. '\n'
      end
      str = str .. ')\n'
   end
   str = str .. '++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++'
   str = str .. c.none
   return str
end

--------------------------------------------------------------------------------
-- standard argument function: used to handle named arguments, and 
-- display automated help for functions
--------------------------------------------------------------------------------
function unpack(args, funcname, description, ...)
   -- look at def, autogen example
   local defs = {...}
   local example
   if #defs > 1 then
      example = funcname .. '{' .. defs[2].arg .. '=' .. defs[2].type .. ', '
                                .. defs[1].arg .. '=' .. defs[1].type .. '}\n'
      example = example .. funcname .. '(' .. defs[1].type .. ',' .. ' ...)'
   end

   -- generate usage string
   local usage = usage(funcname, description, example, ...)

   -- get args
   local iargs = {}
   if #args == 0 then error(usage)
   elseif #args == 1 and type(args[1]) == 'table' and #args[1] == 0 then
      -- named args
      iargs = args[1]
   else
      -- ordered args
      for i,a in ipairs(args) do
         iargs[defs[i].arg] = a
      end
   end

   -- check/set arguments
   local dargs = {}
   local c = COLORS
   for i,def in ipairs(defs) do
      -- is value requested ?
      if def.req and iargs[def.arg] == nil then
         print(c.Red .. 'missing argument: ' .. def.arg .. c.none)
         error(usage)
      end
      -- get value or default
      dargs[def.arg] = iargs[def.arg]
      if dargs[def.arg] == nil then
         dargs[def.arg] = def.default
      end
      if dargs[def.arg] == nil and def.defaulta then
         dargs[def.arg] = dargs[def.defaulta]
      end
      dargs[i] = dargs[def.arg]
   end

   -- return usage too
   dargs.usage = usage

   -- print doc ?
   if _PRINT_DOC_ then
      unpack_printdoc(dargs, funcname, description, ...)
   end

   -- return modified args
   return dargs,
   dargs[1], dargs[2], dargs[3], dargs[4], dargs[5], dargs[6], dargs[7], dargs[8], 
   dargs[9], dargs[10], dargs[11], dargs[12], dargs[13], dargs[14], dargs[15], dargs[16],
   dargs[17], dargs[18], dargs[19], dargs[20], dargs[21], dargs[22], dargs[23], dargs[24],
   dargs[25], dargs[26], dargs[27], dargs[28], dargs[29], dargs[30], dargs[31], dargs[32],
   dargs[33], dargs[34], dargs[35], dargs[36], dargs[37], dargs[38], dargs[39], dargs[40],
   dargs[41], dargs[42], dargs[43], dargs[44], dargs[45], dargs[46], dargs[47], dargs[48],
   dargs[49], dargs[50], dargs[51], dargs[52], dargs[53], dargs[54], dargs[55], dargs[56]
end

--------------------------------------------------------------------------------
-- standard argument function for classes: used to handle named arguments, and 
-- display automated help for functions
-- auto inits the self with usage
--------------------------------------------------------------------------------
function unpack_class(object, args, funcname, description, ...)
   local dargs = unpack(args, funcname, description, ...)
   for k,v in pairs(dargs) do
      object[k] = v
   end
   for i = 1,#object do
      object[i] = nil
   end
end

--------------------------------------------------------------------------------
-- generates doc string from arguments (for luaDoc)
--------------------------------------------------------------------------------
function unpack_printdoc(dargs, funcname, description, ...)
   local sep = '------------------------------------------------------------'
   local comment = '-- '
   local line = '\n'
   local linep = '\n\n'
   -- begin
   local helpstr = sep .. line
   -- description
   helpstr = helpstr .. description .. linep
   -- usage
   helpstr = helpstr .. '@usage ' .. funcname .. '() -- prints online help' .. linep
   -- params
   for i,param in ipairs{...} do
      helpstr = helpstr .. '@param ' .. param.arg .. '  '
      helpstr = helpstr .. param.help
      if param.default then
         helpstr = helpstr .. ' [default = '
         if type(param.default) == 'table' then
            helpstr = helpstr .. '{' .. param.default[1] .. ', ...}'
         else
            helpstr = helpstr .. param.default
         end
         helpstr = helpstr .. ']'
      end
      if param.req then
         helpstr = helpstr .. ' [required]'
      end
      helpstr = helpstr .. ' [type = ' .. param.type .. ']'
      helpstr = helpstr .. line
   end
   -- end
   helpstr = helpstr .. sep
   print(helpstr)
end

--------------------------------------------------------------------------------
-- activates auto documentation string dump (for luaDoc)
--------------------------------------------------------------------------------
function activateDocDump()
   _PRINT_DOC_=true
end

--------------------------------------------------------------------------------
-- useful to fill a tensor from an IDX (lush) file:
--------------------------------------------------------------------------------
function fillTensor(tensor, charstorage, pointer)
   return libxlearn.fillTensor(tensor, charstorage, pointer)
end
function fillFloatTensor(tensor, charstorage, pointer)
   return libxlearn.fillFloatTensor(tensor, charstorage, pointer)
end
function fillByteTensor(tensor, charstorage, pointer)
   return libxlearn.fillByteTensor(tensor, charstorage, pointer)
end

function loadIDX(idxfile)
   local tensor = torch.Tensor()
   local binary = torch.ByteStorage(idxfile)
   local headerp = 1
   local swap = false
   -- helper function
   local function toint()
      local int = 0
      if swap then
         local shifter = 256*256*256
         for i = 1,4 do
            int = int + shifter*binary[headerp]
            shifter = shifter/256
            headerp = headerp+1
         end
      else
         local shifter = 1
         for i = 1,4 do
            int = int + shifter*binary[headerp]
            shifter = shifter*256
            headerp = headerp+1
         end
      end
      return int
   end
   -- get header:
   local magic = toint()
   if binary[1] == 0 then
      swap = true
      magic = binary[3]
   end
   local dims
   if swap then
      dims = binary[4]
   else
      dims = toint()
   end
   local tensorSize = torch.LongStorage(dims)
   for i = 1,dims do
      tensorSize[dims-i+1] = toint()
   end
   local result
   if magic == 507333713 or magic == 0x0D then
      result = torch.FloatTensor(tensorSize)
      fillFloatTensor(result, binary, headerp)
   elseif magic == 507333715 or magic == 0x0E then
      result = torch.Tensor(tensorSize)
      fillTensor(result, binary, headerp)
   elseif magic == 50855936 or magic == 0x08 or magic == 0x09 then
      result = torch.ByteTensor(tensorSize)
      fillByteTensor(result, binary, headerp)
   else
      error('# ERROR<loadIDX>: unknown magic number')
   end
   -- return double tensor
   local doubleT = torch.Tensor(tensorSize):copy(result)
   return doubleT
end


--------------------------------------------------------------------------------
-- split string based on pattern pat
--------------------------------------------------------------------------------
function split(str, pat)
   local t = {}  -- NOTE: use {n = 0} in Lua-5.0
   local fpat = "(.-)" .. pat
   local last_end = 1
   local s, e, cap = str:find(fpat, 1)
   while s do
      if s ~= 1 or cap ~= "" then
	 table.insert(t,cap)
      end
      last_end = e+1
      s, e, cap = str:find(fpat, last_end)
   end
   if last_end <= #str then
      cap = str:sub(last_end)
      table.insert(t, cap)
   end
   return t
end

--------------------------------------------------------------------------------
-- check if a tensor has NaN
--------------------------------------------------------------------------------
function hasNaN(tensor)
   local input = torch.Tensor(tensor:nElement()):copy(tensor)
   local idx = torch.Tensor(tensor:nDimension())
   local i,j,left
   local hasNaN = false
   for i=1,input:nElement() do 
      if isNaN(input[i]) then
         hasNaN = true
         left = input:nElement()
         for j=1,tensor:nDimension() do 
            idx[j] = math.floor(left/tensor:size(j))
            left = left % idx[j]
         end
         print('Nan found at',idx)
         return hasNan
      end
   end
   return hasNaN
end

--------------------------------------------------------------------------------
-- check if a number is NaN
--------------------------------------------------------------------------------
function isNaN(number)
   -- We rely on the property that NaN is the only value that doesn't equal itself.
   return (number ~= number)
end

--------------------------------------------------------------------------------
-- simple sleep function
-- @param seconds time in seconds
--------------------------------------------------------------------------------
function sleep(seconds)
   time = os.realtime()
   while os.realtime()-time < seconds do libxlearn.usleep(1000) end
end

--------------------------------------------------------------------------------
-- returns the size of a file
--------------------------------------------------------------------------------
function fsize(file)
   local current = file:seek()      -- get current position
   local size = file:seek("end")    -- get file size
   file:seek("set", current)        -- restore position
   return size
end

--------------------------------------------------------------------------------   
-- display a progress bar 
--------------------------------------------------------------------------------
do
   local barDone = true
   local previous = -1

   function dispProgress(current, goal)
      local barLength = 77

      -- Compute percentage
      local percent = math.floor(((current) * barLength) / goal)

      -- start new bar
      if (barDone and current == 1) then
         barDone = false
         previous = -1
      end

      --if (percent ~= previous and not barDone) then
      if (not barDone) then
         previous = percent
         -- print bar
         io.write(' [')
         for i=1,barLength do
            if (i < percent) then io.write('=')
            elseif (i == percent) then io.write('>')
            else io.write('.') end
         end
         io.write(']')
         -- go back to center of bar, and print progress
         for i=1,47 do io.write('\b') end
         io.write(' ', current, '/', goal, ' ')
         -- reset for next bar
          io.write('\r')
         if (percent == barLength) then
            barDone = true
            io.write('\n')
         end
         -- flush
         io.flush()
      end
   end
end
do
   local barDone = true
   local previous = -1

   function ncurseProgressBar(current, goal, yy)
      local winHeight,winWidth = libxlearn.ncurseGetDims()
      local x = 0
      local y = yy or (winHeight-1)
      local barLength = winWidth-4
      local function ncurseGoto(to)
         x=to
      end
      local function ncurseOut(str)
         libxlearn.ncursePrint(str,y,x)
         x=x+string.len(str)
      end
      -- Compute percentage
      local percent = math.floor(((current) * barLength) / goal)

      -- start new bar
      if (barDone and current == 1) then
         barDone = false
         previous = -1
      end

      --if (percent ~= previous and not barDone) then
      if (not barDone) then
         previous = percent
         -- print bar
         ncurseOut(' [')
         for i=1,barLength do
            if (i < percent) then ncurseOut('=')
            elseif (i == percent) then ncurseOut('>')
            else ncurseOut('.') end
         end
         ncurseOut(']')
         -- go back to center of bar, and print progress
         local prog = ' '..current..'/'..goal..' '
         ncurseGoto((winWidth-prog:len())/2)
         ncurseOut(prog)
         -- reset for next bar
         ncurseGoto(0)
         ncurseOut(' ')
         if (percent == barLength) then
            barDone = true
         end
      end
      libxlearn.ncurseRefresh()
   end
   
   function ncurseTest()
      libxlearn.ncurseStart()
      maxy, maxx = libxlearn.ncurseGetDims()
      local message = 'nCurse windowing test'
      libxlearn.ncursePrint(message,0,(maxx-message:len())/2,3,0)
      libxlearn.ncursePrint('testing progress bar',6,0)
      for i=1,10000 do
         ncurseProgressBar(i,10000)
      end
      libxlearn.ncursePrint('type in something',10,0)
      local typed = libxlearn.ncurseGetChar()
      libxlearn.ncursePrint('you typed: '..typed,12,0)
      libxlearn.ncursePrint('type smthg to exit',20,0)
      libxlearn.ncurseGetChar() 
      libxlearn.ncurseEnd()
   end
end

--------------------------------------------------------------------------------
-- test if a file exist 
--------------------------------------------------------------------------------
function fileExists(filename)
   local file = io.open(filename)
   if file then
      io.close(file)
      return true
   else
      return false
   end
end

--------------------------------------------------------------------------------
-- test if a path is a directory
--------------------------------------------------------------------------------
function isDir(path)
   if require 'posix' then error('could not load posix module') end

   local file = io.open(path)
   -- check the path exist
   if file then
      io.close(file)
      -- check it's a directory
      if posix.stat(path).type == 'directory' then
         return true
      end
   end
   return false
end

--------------------------------------------------------------------------------  
-- prints the meta table of a torch class
--------------------------------------------------------------------------------
function printMetaTable(class) 
   for k,v in pairs(torch.getmetatable(class)) do 
      print(k,v) 
   end
end


function date()
   local time = os.date("*t", os.time())
   local string = time.year..'-'..time.month..'-'..time.day
                   ..'_'..time.hour..':'..time.min..':'..time.sec
   return string
end

--------------------------------------------------------------------------------
-- returns true/false depending on the just-in-time compiler status
--------------------------------------------------------------------------------
function isJIT(...) 
   if arg then 
      print('# JIT OFF [regular Lua]') 
      return false 
   else print('# JIT ON [using LuaJIT]') 
      return true 
   end 
end

--------------------------------------------------------------------------------
-- returns true if package is installed
--------------------------------------------------------------------------------
function installed(package) 
   return paths.dirp(paths.concat(paths.install_lua_path, package))
end
function require(package) 
   if installed(package) then
      return _GLOB_.require(package)
   else
      print('warning: <' .. package .. '> could not be loaded (is it installed?)')
      return false
   end
end

--------------------------------------------------------------------------------
-- prints an error with nice formatting. If domain is provided, it is used as
-- following: <domain> msg
--------------------------------------------------------------------------------
function error(message, domain, usage) 
   if domain then
      message = '<' .. domain .. '> ' .. message
   else
      message = 'ERROR: ' .. message
   end
   local c = COLORS
   local col_msg = c.Red .. message .. c.none
   if usage then
      print(col_msg)
      _GLOB_.error(usage)
   else
      _GLOB_.error(col_msg)
   end
end

--------------------------------------------------------------------------------
-- execute an OS command, but retrieves the result in a string
-- side effect: creates a file in /tmp/
--
-- @param cmd shell command to execute
-- @return  output from command
--------------------------------------------------------------------------------
function exec(cmd)
   local tmpfile = '/tmp/lua.os.execute.out'
   os.execute(cmd .. ' 1>'.. tmpfile..' 2>' .. tmpfile)
   local file = io.open(tmpfile)
   local str = file:read('*all')
   file:close()
   os.execute('rm ' .. tmpfile)
   return str
end

--------------------------------------------------------------------------------
-- returns the name of the OS in use
-- warning, this method is extremely dumb, and should be replaced by something
-- more reliable
--------------------------------------------------------------------------------
function getOS() 
   if paths.dirp('C:\\') then
      return 'windows'
   else
      local os = exec('uname -a')
      if os:find('Linux') then
         return 'linux'
      elseif os:find('Darwin') then
         return 'macos'
      else
         return '?'
      end
   end
end
OS = getOS()

--------------------------------------------------------------------------------
-- get modify time of file
--------------------------------------------------------------------------------
function ftime(file)
   if OS == 'macos' then
      return tonumber(exec('stat -f "%m" ' .. file))
   elseif OS == 'linux' then
      return tonumber(exec('stat -c "%Z" ' .. file))
   end
end

--------------------------------------------------------------------------------
-- test if a file exist 
--------------------------------------------------------------------------------
function parseXML(xmlString)
   if not require 'xmlreader' then error('could not load xmlreader') end
   
   local r = assert(xmlreader.from_string(xmlString))

   while (r:read()) do
      local leadingws = ('    '):rep(r:depth())
      if (r:node_type() == 'element') then
         io.write(('%s%s:'):format(leadingws, r:name()))
         while (r:move_to_next_attribute()) do
            io.write((' %s=%q'):format(r:name(), r:value()))
         end
         io.write('\n')
      end
   end
end

--------------------------------------------------------------------------------
-- test if a file exist 
--------------------------------------------------------------------------------
do 
   local listDir = torch.class('listDir')
   function listDir:__init(flags)
      if flags then
         self.flags = flags
      else
         self.flags = ''
      end
   end
   function listDir:__tostring__()
      local res = exec('ls '..self.flags)
      return '\n' .. res
   end
end

--------------------------------------------------------------------------------
-- this table contains colors for nice shell printing
--------------------------------------------------------------------------------
COLORS = {none = '\27[0m',
          black = '\27[0;30m',
          red = '\27[0;31m',
          green = '\27[0;32m',
          yellow = '\27[0;33m',
          blue = '\27[0;34m',
          magenta = '\27[0;35m',
          cyan = '\27[0;36m',
          white = '\27[0;37m',
          Black = '\27[1;30m',
          Red = '\27[1;31m',
          Green = '\27[1;32m',
          Yellow = '\27[1;33m',
          Blue = '\27[1;34m',
          Magenta = '\27[1;35m',
          Cyan = '\27[1;36m',
          White = '\27[1;37m',
          _black = '\27[40m',
          _red = '\27[41m',
          _green = '\27[42m',
          _yellow = '\27[43m',
          _blue = '\27[44m',
          _magenta = '\27[45m',
          _cyan = '\27[46m',
          _white = '\27[47m'}

--------------------------------------------------------------------------------  
-- return new tensor of the same size but with only -1 or 1 or 0 vals
-- based on the sign
--------------------------------------------------------------------------------
function sign(tensor)
   result = torch.Tensor(tensor:size()):copy(tensor)
   result:apply(function(x)
                  val = (x>0 and 1) or (x<0 and -1) or 0
                  return val
                end) -- compute the sign
   return result
end

--------------------------------------------------------------------------------
-- return table dataTable from file filename
--------------------------------------------------------------------------------
function readTable(filename)
    file = torch.DiskFile(filename, 'r')
    local data = file:readObject()
    file:close()
    return data
end

--------------------------------------------------------------------------------
-- return a random int number according to uniform distribution
-- min and max are included in the possible range
--------------------------------------------------------------------------------
function randInt(min,max)
   local amplitude = max-min+1
   local res = math.floor(amplitude*random.uniform()+min)
   return res
end

function combinations(n,p)
   if p>n then error("can't choose more than n elements") end
   local function factorial(n)
      return n == 0 and 1 or n * factorial(n - 1)
   end
   local size = factorial(n)/(factorial(p)*factorial(n-p))
   local buffer = torch.Tensor(size,p):fill(0)
   local idx = 1
   -- add a combination in buffer
   local function bufferize(set)
      local id = 1
      for k,v in pairs(set) do
         buffer[idx][id] = v
         id = id+1
      end
      idx = idx + 1
   end
   local function copytable(tab)
      local c = { }
      for j,x in ipairs(tab) do c[j] = x end
      return c
   end
   -- recursive call to do them all
   local function buildcombi(elmts,combi,p)
      if p == 0 then bufferize(combi) return end
      for i=1,#elmts do
         val = table.remove(elmts)
         -- add value for the current combination
         table.insert(combi,val)
         -- recursive call to finish the combination
         buildcombi(copytable(elmts),combi,p-1)
         -- now remove val
         table.remove(combi)
      end
   end
   -- now time for calls
   local values = {}
   for i = n,1,-1 do table.insert(values,i) end
   buildcombi(values,{},p)
   return buffer
end