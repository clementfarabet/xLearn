
--[[--------------------------------------------------------------------------

    This file is part of lunit 0.5.

    For Details about lunit look at: http://www.mroth.net/lunit/

    Author: Michael Roth <mroth@nessie.de>

    Copyright (c) 2004, 2006-2009 Michael Roth <mroth@nessie.de>

    Permission is hereby granted, free of charge, to any person 
    obtaining a copy of this software and associated documentation
    files (the "Software"), to deal in the Software without restriction,
    including without limitation the rights to use, copy, modify, merge,
    publish, distribute, sublicense, and/or sell copies of the Software,
    and to permit persons to whom the Software is furnished to do so,
    subject to the following conditions:

    The above copyright notice and this permission notice shall be 
    included in all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
    EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
    MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
    IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
    CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
    TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
    SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

--]]--------------------------------------------------------------------------




    local orig_assert     = assert

    local pairs           = pairs
    local ipairs          = ipairs
    local next            = next
    local type            = type
    local error           = error
    local tostring        = tostring

    local string_sub      = string.sub
    local string_format   = string.format


    module("lunit", package.seeall)     -- FIXME: Remove package.seeall

    local lunit = _M

    local __failure__ = {}    -- Type tag for failed assertions

    local typenames = { "nil", "boolean", "number", "string", "table", "function", "thread", "userdata" }



    local traceback_hide      -- Traceback function which hides lunit internals
    local mypcall             -- Protected call to a function with own traceback
    do
       local _tb_hide = setmetatable( {}, {__mode="k"} )

       function traceback_hide(func)
          _tb_hide[func] = true
       end

       local function my_traceback(errobj)
          if is_table(errobj) and errobj.type == __failure__ then
             local info = debug.getinfo(5, "Sl")   -- FIXME: Hardcoded integers are bad...
             errobj.where = string_format( "%s:%d", info.short_src, info.currentline)
          else
             errobj = { msg = tostring(errobj) }
             errobj.tb = {}
             local i = 2
             while true do
                local info = debug.getinfo(i, "Snlf")
                if not is_table(info) then
                   break
                end
                if not _tb_hide[info.func] then
                   local line = {}       -- Ripped from ldblib.c...
                   line[#line+1] = string_format("%s:", info.short_src)
                   if info.currentline > 0 then
                      line[#line+1] = string_format("%d:", info.currentline)
                   end
                   if info.namewhat ~= "" then
                      line[#line+1] = string_format(" in function '%s'", info.name)
                   else
                      if info.what == "main" then
                         line[#line+1] = " in main chunk"
                      elseif info.what == "C" or info.what == "tail" then
                         line[#line+1] = " ?"
                      else
                         line[#line+1] = string_format(" in function <%s:%d>", info.short_src, info.linedefined)
                      end
                   end
                   errobj.tb[#errobj.tb+1] = table.concat(line)
                end
                i = i + 1
             end
          end
          return errobj
       end

       function mypcall(func)
          orig_assert( is_function(func) )
          local ok, errobj = xpcall(func, my_traceback)
          if not ok then
             return errobj
          end
       end
       traceback_hide(mypcall)
    end


    -- Type check functions

    for _, typename in ipairs(typenames) do
       lunit["is_"..typename] = function(x)
                                   return type(x) == typename
                                end
    end

    local is_nil      = is_nil
    local is_boolean  = is_boolean
    local is_number   = is_number
    local is_string   = is_string
    local is_table    = is_table
    local is_function = is_function
    local is_thread   = is_thread
    local is_userdata = is_userdata


    local function failure(name, usermsg, defaultmsg, ...)
       local errobj = {
          type    = __failure__,
          name    = name,
          msg     = string_format(defaultmsg,...),
          usermsg = usermsg
       }
       error(errobj, 0)
    end
    traceback_hide( failure )


    local function format_arg(arg)
       local argtype = type(arg)
       if argtype == "string" then
          return "'"..arg.."'"
       elseif argtype == "number" or argtype == "boolean" or argtype == "nil" then
          return tostring(arg)
       else
          return "["..tostring(arg).."]"
       end
    end


    function fail(msg)
       stats.assertions = stats.assertions + 1
       failure( "fail", msg, "failure" )
    end
    traceback_hide( fail )


    function assert(assertion, msg)
       stats.assertions = stats.assertions + 1
       if not assertion then
          failure( "assert", msg, "assertion failed" )
       end
       return assertion
    end
    traceback_hide( assert )


    function assert_true(actual, msg)
       stats.assertions = stats.assertions + 1
       local actualtype = type(actual)
       if actualtype ~= "boolean" then
          failure( "assert_true", msg, "true expected but was a "..actualtype )
       end
       if actual ~= true then
          failure( "assert_true", msg, "true expected but was false" )
       end
       return actual
    end
    traceback_hide( assert_true )


    function assert_false(actual, msg)
       stats.assertions = stats.assertions + 1
       local actualtype = type(actual)
       if actualtype ~= "boolean" then
          failure( "assert_false", msg, "false expected but was a "..actualtype )
       end
       if actual ~= false then
          failure( "assert_false", msg, "false expected but was true" )
       end
       return actual
    end
    traceback_hide( assert_false )


    function assert_equal(expected, actual, msg)
       stats.assertions = stats.assertions + 1
       if expected ~= actual then
          failure( "assert_equal", msg, "expected %s but was %s", format_arg(expected), format_arg(actual) )
       end
       return actual
    end
    traceback_hide( assert_equal )

    function assert_almost_equal(expected, actual, decimal, msg)
       stats.assertions = stats.assertions + 1
       diff = math.abs(expected - actual)
       if diff > 10^-decimal then
          failure( "assert_almost_equal", msg, "expected < %s but was %s", format_arg(10^-decimal), format_arg(actual) )
       end
       return actual
    end
    traceback_hide( assert_almost_equal )


    function assert_not_equal(unexpected, actual, msg)
       stats.assertions = stats.assertions + 1
       if unexpected == actual then
          failure( "assert_not_equal", msg, "%s not expected but was one", format_arg(unexpected) )
       end
       return actual
    end
    traceback_hide( assert_not_equal )


    function assert_match(pattern, actual, msg)
       stats.assertions = stats.assertions + 1
       local patterntype = type(pattern)
       if patterntype ~= "string" then
          failure( "assert_match", msg, "expected the pattern as a string but was a "..patterntype )
       end
       local actualtype = type(actual)
       if actualtype ~= "string" then
          failure( "assert_match", msg, "expected a string to match pattern '%s' but was a %s", pattern, actualtype )
       end
       if not string.find(actual, pattern) then
          failure( "assert_match", msg, "expected '%s' to match pattern '%s' but doesn't", actual, pattern )
       end
       return actual
    end
    traceback_hide( assert_match )


    function assert_not_match(pattern, actual, msg)
       stats.assertions = stats.assertions + 1
       local patterntype = type(pattern)
       if patterntype ~= "string" then
          failure( "assert_not_match", msg, "expected the pattern as a string but was a "..patterntype )
       end
       local actualtype = type(actual)
       if actualtype ~= "string" then
          failure( "assert_not_match", msg, "expected a string to not match pattern '%s' but was a %s", pattern, actualtype )
       end
       if string.find(actual, pattern) then
          failure( "assert_not_match", msg, "expected '%s' to not match pattern '%s' but it does", actual, pattern )
       end
       return actual
    end
    traceback_hide( assert_not_match )


    function assert_error(msg, func)
       stats.assertions = stats.assertions + 1
       if func == nil then
          func, msg = msg, nil
       end
       local functype = type(func)
       if functype ~= "function" then
          failure( "assert_error", msg, "expected a function as last argument but was a "..functype )
       end
       local ok, errmsg = pcall(func)
       if ok then
          failure( "assert_error", msg, "error expected but no error occurred" )
       end
    end
    traceback_hide( assert_error )


    function assert_error_match(msg, pattern, func)
       stats.assertions = stats.assertions + 1
       if func == nil then
          msg, pattern, func = nil, msg, pattern
       end
       local patterntype = type(pattern)
       if patterntype ~= "string" then
          failure( "assert_error_match", msg, "expected the pattern as a string but was a "..patterntype )
       end
       local functype = type(func)
       if functype ~= "function" then
          failure( "assert_error_match", msg, "expected a function as last argument but was a "..functype )
       end
       local ok, errmsg = pcall(func)
       if ok then
          failure( "assert_error_match", msg, "error expected but no error occurred" )
       end
       local errmsgtype = type(errmsg)
       if errmsgtype ~= "string" then
          failure( "assert_error_match", msg, "error as string expected but was a "..errmsgtype )
       end
       if not string.find(errmsg, pattern) then
          failure( "assert_error_match", msg, "expected error '%s' to match pattern '%s' but doesn't", errmsg, pattern )
       end
    end
    traceback_hide( assert_error_match )


    function assert_pass(msg, func)
       stats.assertions = stats.assertions + 1
       if func == nil then
          func, msg = msg, nil
       end
       local functype = type(func)
       if functype ~= "function" then
          failure( "assert_pass", msg, "expected a function as last argument but was a %s", functype )
       end
       local ok, errmsg = pcall(func)
       if not ok then
          failure( "assert_pass", msg, "no error expected but error was: '%s'", errmsg )
       end
    end
    traceback_hide( assert_pass )


    -- lunit.assert_typename functions

    for _, typename in ipairs(typenames) do
       local assert_typename = "assert_"..typename
       lunit[assert_typename] = function(actual, msg)
                                   stats.assertions = stats.assertions + 1
                                   local actualtype = type(actual)
                                   if actualtype ~= typename then
                                      failure( assert_typename, msg, typename.." expected but was a "..actualtype )
                                   end
                                   return actual
                                end
       traceback_hide( lunit[assert_typename] )
    end


    -- lunit.assert_not_typename functions

    for _, typename in ipairs(typenames) do
       local assert_not_typename = "assert_not_"..typename
       lunit[assert_not_typename] = function(actual, msg)
                                       stats.assertions = stats.assertions + 1
                                       if type(actual) == typename then
                                          failure( assert_not_typename, msg, typename.." not expected but was one" )
                                       end
                                    end
       traceback_hide( lunit[assert_not_typename] )
    end


    function lunit.clearstats()
       stats = {
          assertions  = 0;
          passed      = 0;
          failed      = 0;
          errors      = 0;
       }
    end


    local report, reporterrobj
    do
       local testrunner

       function lunit.setrunner(newrunner)
          if not ( is_table(newrunner) or is_nil(newrunner) ) then
             return error("lunit.setrunner: Invalid argument", 0)
          end
          local oldrunner = testrunner
          testrunner = newrunner
          return oldrunner
       end

       function lunit.loadrunner(name)
          if not is_string(name) then
             return error("lunit.loadrunner: Invalid argument", 0)
          end
          local ok, runner = pcall( require, name )
          if not ok then
             return error("lunit.loadrunner: Can't load test runner: "..runner, 0)
          end
          return setrunner(runner)
       end

       function report(event, ...)
          local f = testrunner and testrunner[event]
          if is_function(f) then
             pcall(f, ...)
          end
       end

       function reporterrobj(context, tcname, testname, errobj)
          local fullname = tcname .. "." .. testname
          if context == "setup" then
             fullname = fullname .. ":" .. setupname(tcname, testname)
          elseif context == "teardown" then
             fullname = fullname .. ":" .. teardownname(tcname, testname)
          end
          if errobj.type == __failure__ then
             stats.failed = stats.failed + 1
             report("fail", fullname, errobj.where, errobj.msg, errobj.usermsg)
          else
             stats.errors = stats.errors + 1
             report("err", fullname, errobj.msg, errobj.tb)
          end
       end
    end



    local function key_iter(t, k)
       return (next(t,k))
    end


    local testcase
    do
       -- Array with all registered testcases
       local _testcases = {}

       -- Marks a module as a testcase.
       -- Applied over a module from module("xyz", lunit.testcase).
       function lunit.testcase(m)
          orig_assert( is_table(m) )
          --orig_assert( m._M == m )
          orig_assert( is_string(m._NAME) )
          --orig_assert( is_string(m._PACKAGE) )

          -- Register the module as a testcase
          _testcases[m._NAME] = m

          -- Import lunit, fail, assert* and is_* function to the module/testcase
          m.lunit = lunit
          m.fail = lunit.fail
          for funcname, func in pairs(lunit) do
             if "assert" == string_sub(funcname, 1, 6) or "is_" == string_sub(funcname, 1, 3) then
                m[funcname] = func
             end
          end
       end

       -- Iterator (testcasename) over all Testcases
       function lunit.testcases()
          -- Make a copy of testcases to prevent confusing the iterator when
          -- new testcase are defined
          local _testcases2 = {}
          for k,v in pairs(_testcases) do
             _testcases2[k] = true
          end
          return key_iter, _testcases2, nil
       end

       function testcase(tcname)
          return _testcases[tcname]
       end
    end


    do
       -- Finds a function in a testcase case insensitive
       local function findfuncname(tcname, name)
          for key, value in pairs(testcase(tcname)) do
             if is_string(key) and is_function(value) and string.lower(key) == name then
                return key
             end
          end
       end

       function lunit.setupname(tcname)
          return findfuncname(tcname, "setup")
       end

       function lunit.teardownname(tcname)
          return findfuncname(tcname, "teardown")
       end

       -- Iterator over all test names in a testcase.
       -- Have to collect the names first in case one of the test
       -- functions creates a new global and throws off the iteration.
       function lunit.tests(tcname)
          local testnames = {}
          for key, value in pairs(testcase(tcname)) do
             if is_string(key) and is_function(value) then
                local lfn = string.lower(key)
                if string.sub(lfn, 1, 4) == "test" or string.sub(lfn, -4) == "test" then
                   testnames[key] = true
                end
             end
          end
          return key_iter, testnames, nil
       end
    end




    function lunit.runtest(tcname, testname)
       orig_assert( is_string(tcname) )
       orig_assert( is_string(testname) )

       local function callit(context, func)
          if func then
             local err = mypcall(func)
             if err then
                reporterrobj(context, tcname, testname, err)
                return false
             end
          end
          return true
       end
       traceback_hide(callit)

       report("run", tcname, testname)

       local tc          = testcase(tcname)
       local setup       = tc[setupname(tcname)]
       local test        = tc[testname]
       local teardown    = tc[teardownname(tcname)]

       local setup_ok    =              callit( "setup", setup )
       local test_ok     = setup_ok and callit( "test", test )
       local teardown_ok = setup_ok and callit( "teardown", teardown )

       if setup_ok and test_ok and teardown_ok then
          stats.passed = stats.passed + 1
          report("pass", tcname, testname)
       end
    end
    traceback_hide(runtest)



    function lunit.run()
       clearstats()
       report("begin")
       for testcasename in lunit.testcases() do
          -- Run tests in the testcases
          for testname in lunit.tests(testcasename) do
             runtest(testcasename, testname)
          end
       end
       report("done")
       return stats
    end
    traceback_hide(run)


    function lunit.loadonly()
       clearstats()
       report("begin")
       report("done")
       return stats
    end









    local lunitpat2luapat
    do 
       local conv = {
          ["^"] = "%^",
          ["$"] = "%$",
          ["("] = "%(",
          [")"] = "%)",
          ["%"] = "%%",
          ["."] = "%.",
          ["["] = "%[",
          ["]"] = "%]",
          ["+"] = "%+",
          ["-"] = "%-",
          ["?"] = ".",
          ["*"] = ".*"
       }
       function lunitpat2luapat(str)
          return "^" .. string.gsub(str, "%W", conv) .. "$"
       end
    end



    local function in_patternmap(map, name)
       if map[name] == true then
          return true
       else
          for _, pat in ipairs(map) do
             if string.find(name, pat) then
                return true
             end
          end
       end
       return false
    end








    -- Called from 'lunit' shell script.

    function main(argv)
       argv = argv or {}

       -- FIXME: Error handling and error messages aren't nice.

       local function checkarg(optname, arg)
          if not is_string(arg) then
             return error("lunit.main: option "..optname..": argument missing.", 0)
          end
       end

       local function loadtestcase(filename)
          if not is_string(filename) then
             return error("lunit.main: invalid argument")
          end
          local chunk, err = loadfile(filename)
          if err then
             return error(err)
          else
             chunk()
          end
       end

       local testpatterns = nil
       local doloadonly = false
       local runner = nil

       local i = 0
       while i < #argv do
          i = i + 1
          local arg = argv[i]
          if arg == "--loadonly" then
             doloadonly = true
          elseif arg == "--runner" or arg == "-r" then
             local optname = arg; i = i + 1; arg = argv[i]
             checkarg(optname, arg)
             runner = arg
          elseif arg == "--test" or arg == "-t" then
             local optname = arg; i = i + 1; arg = argv[i]
             checkarg(optname, arg)
             testpatterns = testpatterns or {}
             testpatterns[#testpatterns+1] = arg
          elseif arg == "--" then
             while i < #argv do
                i = i + 1; arg = argv[i]
                loadtestcase(arg)
             end
          else
             loadtestcase(arg)
          end
       end

       loadrunner(runner or "lunit-console")

       if doloadonly then
          return loadonly()
       else
          return run(testpatterns)
       end
    end

    clearstats()
