--------------------------------------------------------------------------------
-- xPrompt: just a bunch of funtions to enhance the prompt.
-- 
-- Most important > provides pretty() or p() to print lists !!!
--------------------------------------------------------------------------------

-- better prompt
local c = toolBox.COLORS
-- CANT USE color because it fucks up the history ... WHY ???
--_PROMPT = c.Red .. 'xLua' .. c.none .. ' > '
--_PROMPT2 = c.Green .. ' ...' .. c.none .. ' > '
_PROMPT = 'xLua > '
_PROMPT2 = ' ... > '
c = nil

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
