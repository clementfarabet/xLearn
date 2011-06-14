--------------------------------------------------------------------------------
-- luaExt: extensions to Lua Core modules
-- 
-- @release 2010 Clement Farabet
--------------------------------------------------------------------------------

--------------------------------------------------------------------------------
-- returns the wall clock time as an accurate double
-- @return   real time (wall clock)
-- @see      os.clock() gives the CPU time
--------------------------------------------------------------------------------
os.realtime = libxlearn.getMicroTime

--------------------------------------------------------------------------------
-- computes log2 of a number
-- @param n real number
-- @return  log2(n)
--------------------------------------------------------------------------------
function math.log2(n)
   return math.log(n)/math.log(2)
end

--------------------------------------------------------------------------------
-- extends paths with a home variable
--------------------------------------------------------------------------------
paths.home = os.getenv('HOME')

--------------------------------------------------------------------------------
-- debugger helpers
--------------------------------------------------------------------------------
function __breakpoint__()
   if _DEBUG_ then pause()
   else xprint('only works if "debugger" is loaded (or luaD is used)','__breakpoint__')
   end
end