----------------------------------------------------------------------
--
-- Copyright (c) 2010 Clement Farabet
-- 
-- Permission is hereby granted, free of charge, to any person obtaining
-- a copy of this software and associated documentation files (the
-- "Software"), to deal in the Software without restriction, including
-- without limitation the rights to use, copy, modify, merge, publish,
-- distribute, sublicense, and/or sell copies of the Software, and to
-- permit persons to whom the Software is furnished to do so, subject to
-- the following conditions:
-- 
-- The above copyright notice and this permission notice shall be
-- included in all copies or substantial portions of the Software.
-- 
-- THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
-- EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
-- MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
-- NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
-- LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
-- OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
-- WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
-- 
----------------------------------------------------------------------
-- description:
--     inline - a package to dynamically build and run C/C++ from
--              within Lua.
--
-- history: 
--     March 27, 2011, 9:58PM - creation
----------------------------------------------------------------------

require 'paths'
module('inline', package.seeall)

----------------------------------------------------------------------
-- Internals
----------------------------------------------------------------------
verbose = false
loaded = {}

----------------------------------------------------------------------
-- Basic Templates
----------------------------------------------------------------------
_template_c_ = [[

static int #FUNCNAME#_l (lua_State *L) {
#BODY#
  return 0;
}

static const struct luaL_reg #FUNCNAME# [] = {
  {"#FUNCNAME#", #FUNCNAME#_l},
  {NULL, NULL}
};

int luaopen_lib#FUNCNAME# (lua_State *L) {
   luaL_openlib(L, "lib#FUNCNAME#", #FUNCNAME#, 0);
   return 1; 
}
]]
_template_cvirgin_ = _template_c_

----------------------------------------------------------------------
-- Make/Paths
----------------------------------------------------------------------
-- commands
_mkdir_ = 'mkdir -p '
_rmdir_ = 'rm -r '
_make_c_ = 'gcc -fpic -shared -I'..paths.install_include..' -o '
_make_libs_ = ' -L'..paths.install_lib..' -llua -lluaT -lTH '

-- paths
_make_path_ = paths.concat(os.getenv('HOME'), '.luamake')

----------------------------------------------------------------------
-- Headers
----------------------------------------------------------------------
validheaders = {}
function headers (...)
   for i = 1,select('#',...) do
      local header = select(i,...)
      if not validheaders[header] then
         _template_c_ = '#include <' .. header .. '>\n' .. _template_c_
      end
      validheaders[header] = true
   end
end
headers('stdlib.h','stdio.h','string.h','math.h','TH/TH.h','luaT.h')

----------------------------------------------------------------------
-- preamble
----------------------------------------------------------------------
function preamble (code_preamble)
   -- reset the template
   _template_c_ = '\n' .. code_preamble ..'\n'.._template_cvirgin_
   -- readd all the header
   for header,v in pairs(validheaders) do
      _template_c_ = '#include <' .. header .. '>\n' .. _template_c_
   end
end

----------------------------------------------------------------------
-- Compiler
----------------------------------------------------------------------
function load (code,exec)
   -- time
   local tt = os.realtime()

   -- get context
   local c = toolBox.COLORS
   local dbg = debug.getinfo(2 + (exec or 0))
   local source_path = dbg.source:gsub('@','')
   local source_line = dbg.currentline
   local iscompiled = false
   if source_path:find('/') ~= 1 then source_path = paths.concat(paths.cwd(),source_path) end
   local upath = source_path:gsub('%.lua','')..'_'..source_line
   local ref = upath
   local shell = false
   local modtime = toolBox.mtime(source_path)

   -- lib/src unique names
   if not paths.filep(source_path) then -- this call is from the lua shell
      ref = 'fromshell'
      shell = true
   end
   local funcname = paths.basename(upath)
   local filename = upath .. '.c'
   local libname = paths.concat(paths.dirname(upath), 'lib' .. paths.basename(upath) .. '.so')

   -- check existence of library
   local buildme = true
   if not shell then
      -- has file+line already been loaded ?
      local lib = loaded[ref]
      if lib and (modtime == lib.modtime) then
         -- file hasn't changed, just return local lib
         if verbose then
            print(c.green .. 'reusing preloaded code (originally compiled from '
                  .. source_path .. ' @ line ' .. source_line .. ')'
                  .. ' [in ' .. (os.realtime() - tt) .. ' seconds]' .. c.none)
         end
         return lib.f
      end
      -- or else, has file+line been built in a previous life ?
      local libmodtime = toolBox.mtime(_make_path_..libname)
      if libmodtime and libmodtime >= modtime then
         -- library was previously built, just reload it !
         if verbose then
            print(c.magenta .. 'reloading library (originally compiled from' 
                  .. source_path .. ' @ line ' .. source_line .. ')' .. c.none)
         end
         buildme = false
      end
   end

   -- if not found, build it
   if buildme then
      if verbose then
         print(c.red .. 'compiling inline code from ' .. source_path .. ' @ line ' .. source_line .. c.none)
      end

      -- parse given code
      local parsed = _template_c_:gsub('#FUNCNAME#',funcname)
      parsed = parsed:gsub('#BODY#', code)

      -- write file to disk
      toolBox.exec(_mkdir_ .. _make_path_..paths.dirname(filename))
      local f = io.open(_make_path_..filename, 'w')
      f:write(parsed)
      f:close()

      -- compile it
      local msgs = toolBox.exec('cd '.._make_path_..paths.dirname(filename)..'; ' 
                             .. _make_c_ .. paths.basename(libname) .. _make_libs_ .. paths.basename(filename))
      if msgs ~= '' then
         -- cleanup
         toolBox.exec(_rmdir_ .. _make_path_..filename)
         print(c.blue)
         local debug = ''
         local itr = 1
         local tmp = parsed:gsub('\n','\n: ')
         for line in tmp:gmatch("[^\r\n]+") do 
            if itr == 1 then line = ': ' .. line end
            debug = debug .. itr .. line .. '\n'
            itr = itr + 1
         end
         io.write(debug)
         print(c.red .. 'from GCC:')
         print(msgs .. c.none)
         xerror('could not compile given code', 'inline.load')
      end
   end

   -- load shared lib
   local saved_cpath = package.cpath
   package.cpath = _make_path_..paths.dirname(libname)..'/?.so'
   local loadedlib = require(paths.basename(libname):gsub('.so',''))
   package.cpath = saved_cpath

   -- register function for future use
   if not shell then
      loaded[ref] = {modtime=modtime, f=loadedlib[funcname]}
   end

   -- time
   if verbose then
      print(c.green .. '[in ' .. (os.realtime() - tt) .. ' seconds]' .. c.none)
   end

   -- return function
   return loadedlib[funcname]
end

----------------------------------------------------------------------
-- Executer
----------------------------------------------------------------------
function exec (code, ...)
   local func = load(code,1)
   return func(...)
end

----------------------------------------------------------------------
-- Flush all cached libraries
----------------------------------------------------------------------
function flush ()
   -- complete cleanup
   toolBox.exec(_rmdir_ .. _make_path_)
end

----------------------------------------------------------------------
-- Test me
----------------------------------------------------------------------
function testme ()
   local example = [[
         printf("Hello World\n");
   ]]
   local compiled = make.c(example)
   compiled()
end
