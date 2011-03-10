--------------------------------------------------------------------------------
-- xPrompt: just a bunch of funtions to enhance the prompt.
-- 
-- Most important > provides pretty() or p() to print lists !!!
--------------------------------------------------------------------------------

------------------------------------------------------------
-- better prompt
--
local c = toolBox.COLORS
-- CANT USE color because it fucks up the history ... WHY ???
--_PROMPT = c.Red .. 'xLua' .. c.none .. ' > '
--_PROMPT2 = c.Green .. ' ...' .. c.none .. ' > '
_PROMPT = 'xLua > '
_PROMPT2 = ' ... > '
c = nil

------------------------------------------------------------
-- overwrite print function to expand tables
--
lua_print = print
print = function(obj,...)
          if type(obj) == 'table' then
              local mt = getmetatable(obj)
              if mt and mt.__tostring__ then
                 io.write(mt.__tostring__(obj))
              else
                 local tos = tostring(obj)
                 local obj_w_usage = false
                 if tos and not string.find(tos,'table: ') then
                    if obj.usage then
                       io.write(obj.usage)
                       io.write('\n\nFIELDS:\n')
                       obj_w_usage = true
                    else
                       io.write(tos .. ':\n')
                    end
                 end
                 io.write('{')
                 local tab = ''
                 local idx = 1
                 for k,v in pairs(obj) do
                    if idx > 1 then io.write(',\n') end
                    if type(v) == 'userdata' then
                       io.write(tab .. '[' .. k .. ']' .. ' = <userdata>')
                    else
                       local tostr = tostring(v):gsub('\n','\\n')
                       if #tostr>40 then
                          local tostrshort = tostr:sub(1,40) .. toolBox.COLORS.none
                          io.write(tab .. '[' .. k .. ']' .. ' = ' .. tostrshort .. ' ... ')
                       else
                          io.write(tab .. '[' .. k .. ']' .. ' = ' .. tostr)
                       end
                    end
                    tab = ' '
                    idx = idx + 1
                 end
                 io.write('}')
                 if obj_w_usage then
                    io.write('')                    
                 end
              end
           else 
              io.write(tostring(obj))
           end
           if select('#',...) > 0 then
              io.write('    ')
              print(...)
           else
              io.write('\n')
           end
        end

------------------------------------------------------------
-- the following defines a pretty print for tables
--
local print_handlers = {}
local jstack = {}

local pretty_print_limit = 40
local max_depth = 7
local table_clever = true
local inputnb = 0
local verbose = false
local strict = false

local function join(tbl,delim,limit,depth)
   if not limit then limit = pretty_print_limit end
   if not depth then depth = max_depth end
   local n = #tbl
   local res = ''
   local k = 0
   -- very important to avoid disgracing ourselves with circular referencs...
   if #jstack > depth then
      return "..."
   end
   for i,t in ipairs(jstack) do
      if tbl == t then
         return "<self>"
      end
   end
   table.insert(jstack,tbl)
   -- this is a hack to work out if a table is 'list-like' or 'map-like'
   -- you can switch it off with ilua.table_options {clever = false}
   local is_list
   if table_clever then
      local index1 = n > 0 and tbl[1]
      local index2 = n > 1 and tbl[2]
      is_list = index1 and index2
   end
   if is_list then
      for i,v in ipairs(tbl) do
         res = res..delim..val2str(v)
         k = k + 1
         if k > limit then
            res = res.." ... "
            break
         end
      end
   else
      for key,v in pairs(tbl) do
         if type(key) == 'number' then
            key = '['..tostring(key)..']'
         else
            key = tostring(key)
         end
         res = res..delim..key..' = '..val2str(v)
         k = k + 1
         if k > limit then
            res = res.." ... "
            break
         end            
      end
   end
   table.remove(jstack)
   return string.sub(res,2)
end

function val2str(val)
   local tp = type(val)
   if print_handlers[tp] then
      local s = print_handlers[tp](val)
      return s or '?'
   end
   if tp == 'function' then
      return tostring(val)
   elseif tp == 'table' then
      if val.__tostring  then
         return tostring(val)
      else
         return '{'..join(val,', ')..'}'
      end
   elseif tp == 'string' then
      return "'"..val.."'"
   elseif tp == 'number' then
      -- we try only to apply floating-point precision for numbers deemed to be floating-point,
      -- unless the 3rd arg to precision() is true.
      if num_prec and (num_all or math.floor(val) ~= val) then
         return num_prec:format(val)
      else
         return tostring(val)
      end
   else
      return tostring(val)
   end
end

local function oprint(...)
    if savef then
        savef:write(table.concat({...},' '),'\n')
    end
    print(...)
end

function pretty(...)
   local arg = {...}
   for i,val in ipairs(arg) do
      oprint(val2str(val))
   end
   _G['_'] = arg[1]
end

p=pretty
