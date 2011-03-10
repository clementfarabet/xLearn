--------------------------------------------------------------------------------
-- xFlow.engine: a class to handle xFlow programs.
-- 
-- what?  xFlow programs can be used to represent algorithms that operate on
--        homogenous data structures.
--        It's particularly well suited to describe networks of transformations.
--
--        This class implements a simple xFlow parser, and a back-end to exec
--        an xFlow program using the Torch Environment.
--
-- version: 0.2
--------------------------------------------------------------------------------


----------------------------------------------------------------------
--- xFlow.engine
-- A class to parse and build xFlow programs
--
if not xFlow then xFlow = {} end
xFlow.engine = {}
local eng = xFlow.engine


----------------------------------------------------------------------
--- Inits the xFlow engine.
--
eng.verbose = false

-- messages
eng.MSG_PARSING = '# INFO: parsing xFlow'
eng.MSG_VERSION = '# INFO: xFlow version = '
eng.MSG_MAPPED = '# INFO: 100% mapped'

-- warnings
eng.ERROR_UNBALANCED = "# ERROR: unbalanced s-exp"
eng.WARNING_IMPLEMENTED = '# WARNING: non implemented > '

-- errors
eng.ERROR_IMPLEMENTED = '# ERROR: non implemented > '
eng.ERROR_MISSING_ARG = '# ERROR: missing argument > '
eng.ERROR_BADDATA = '# ERROR: bad data format > '
eng.ERROR_NOT_DECLARED = '# ERROR: not declared > '
eng.ERROR_UNKNOWN_TYPE = '# ERROR: unknown type > '


----------------------------------------------------------------------
--- Buils the xFlow engine.
-- @param verbose  be verbose during parsing
--
function xFlow.engine.reset()
   -- stack of defs:
   eng.defs = {}

   -- compiled engine.
   eng.computer = {nodes = {{inputs  = {}, outputs = {}}},
                    level = 1,
                    machine = {},
                    name = {},
                    update = {}}

   -- clear loaded libs
   for k,lib in pairs(xFlow.libs) do
      lib.loaded = false
   end
end


----------------------------------------------------------------------   
--- Parses an xFlow string.
-- Returns a table that contains the whole hierarchy.
-- @param flow   xFlow string.
--
function xFlow.engine.parse(flow)
   -- reset first
   eng.reset()

   local stack = {}
   local top = {}
   table.insert(stack, top)
   local ni,c,open,xarg,nilpattern,nolabel,comment,stringp
   local s = flow
   local i, j = 1, 1
   print(eng.MSG_PARSING)
   while true do
      comment,nilpattern,nolabel,open,xarg,stringp,c = nil,nil,nil,nil,nil,nil,nil

      -- find comments
      while true do
         ni,j,comment = string.find(s, "^%s*;;", i)
         if (ni ~= nil) then
            i = j+1
            ni,j,comment = string.find(s, "\n", i)
            i = j+1
         else
            break
         end
      end

      -- parse s-expr
      ni,j,open = string.find(s, "^%s*%(%s*()", i) -- (
      if (open == nil) then 
         ni,j,xarg = string.find(s, '^%s*"([-%w.+]+)"', i) -- string
         if ni == nil then
            ni,j,xarg = string.find(s, "^%s*([-%w.+*/]+)", i) -- param
            if xarg == nil then
               ni,j,xarg = string.find(s, "^%s*(?)", i) -- special param: ?
            end
            if xarg == nil then
               ni,j,xarg = string.find(s, "^%s*(%.%.%.)", i) -- ...
            end
         else
            stringp = true
         end
         if xarg == nil then
            ni,j,c = string.find(s, "^%s*(%))", i) -- )
            if c ~= nil then c = true end
         end
      end

      -- end of s-exp
      if not ni then break end

      -- add symbols to stack
      if open ~= nil then
         top = {}  -- arg={}, nargs=0
         table.insert(stack, top)   -- new level
         if (eng.verbose) then
            print('new level> ')
         end
      elseif xarg ~= nil then   -- start tag
         --stack[#stack].nargs = stack[#stack].nargs + 1
         stack[#stack][#stack[#stack]+1] = xarg
         if (eng.verbose) then
            print('appending arg: ' .. xarg)
         end
         if stringp then end -- treat string differently ?
      end

      -- close
      if (c) then  -- end tag
         if (eng.verbose) then
            print('close level< ')
         end
         local toclose = table.remove(stack)  -- remove top
         top = stack[#stack]
         if #stack < 1 then
            error("nothing to close i="..i)
         end
         table.insert(top, toclose)
      end
      i = j+1
   end

   if #stack > 1 then
      error(eng.ERROR_UNBALANCED)
   end

   -- save and return result
   return stack[1]
end


----------------------------------------------------------------------
--- Serializes an xFlow program: luaFlow -> xFlow
-- Converts a luaFlow node into an xFlow file
-- @param file    xFlow output path (file)
--
function xFlow.engine.serialize(flow, file)
   print(eng.WARNING_IMPLEMENTED .. 'xFlow serializer not available yet')
end


----------------------------------------------------------------------
--- Implements an xFlow program: xFlow -> luaFlow
-- Converts the given xFlow code to run natively the luaFlow framework
-- @param file  an xFlow program (file)
--
function xFlow.engine.implement(file)
   -- open file
   local f = assert(io.open(file, "r"))
   local s = f:read("*all")
   f:close()

   -- implement string
   return eng.implement_str(s)
end


----------------------------------------------------------------------
--- Implements an xFlow program within Torch framework
-- Converts the given xFlow code to run natively within Torch.
-- @param file  an xFlow program (file)
--
function xFlow.engine.implement_str(s)
   -- (0) parse
   local flow = eng.parse(s)

   -- (1) implement
   eng.implement_level(flow)

   -- (2) reorder/reorganize
   eng.finalize_computer()

   -- give stats/results
   print(eng.MSG_MAPPED)

   return eng.computer
end


----------------------------------------------------------------------
--- Implements a level of an xFlow program
-- @param flow   a parsed flow
--
function xFlow.engine.implement_level(flow)
   -- (1) create new level on stack
   eng.defs[#eng.defs+1] = {}
   if eng.defs[#eng.defs-1] then
      eng.defs[#eng.defs]['version'] = eng.defs[#eng.defs-1]['version']
   end
   eng.defs[#eng.defs]['compute-node'] = {}
   eng.defs[#eng.defs]['data-node'] = {}
   eng.defs[#eng.defs]['input-node'] = {}
   eng.defs[#eng.defs]['output-node'] = {}

   -- (2) define one level == a def is a pointer to a parsed node
   eng.process_level(flow, 
                      {['require']       =  eng.process_require,
                       ['version']       =  eng.process_version,
                       ['compute-node']  =  eng.process_node,
                       ['data-node']     =  eng.process_node,
                       ['input-node']    =  eng.process_node,
                       ['output-node']   =  eng.process_node },
                      true)

   -- (3) implement symbols in level
   local dummy = function () end
   eng.process_level(flow,
                      {['require']       =  dummy,
                       ['version']       =  dummy,
                       ['compute-node']  =  dummy,
                       ['data-node']     =  dummy,
                       ['input-node']    =  dummy,
                       ['output-node']   =  dummy},
                      false)
end


----------------------------------------------------------------------   
--- Processes a level: extracts all defs and adds them to the defs 
-- stack. Creates a new level on the defs stack.
-- @param flow        parsed flow program
-- @param callback    callback table {name=callback, ...}
-- @param ignoreVars  ignore variables (only process definitions)
-- @param start       offset in flow
--
function xFlow.engine.process_level(flow, callback, ignoreVars, start)
   local idx = start or 1
   local ignoreVars = ignoreVars or false
   while true do
      if flow[idx] == nil then
         break
      end
      local call = callback[flow[idx][1]]
      if call then
         call(flow[idx])
      elseif not ignoreVars then
         eng.process_definition(flow[idx])
      end
      idx = idx + 1
   end
end


----------------------------------------------------------------------   
--- Process definitions, implement them
--
function xFlow.engine.get_node(flow)
   -- retrieve definition
   local def = nil
   local type = nil
   for lvl = #eng.defs,1,-1 do
      -- variable can be one of [compute|data|input|output]-node
      def = eng.defs[lvl]['compute-node'][flow[1]]
      if def then
         type = 'compute'
         break
      end
      def = eng.defs[lvl]['data-node'][flow]
      if def then
         type = 'data'
         break
      end
      def = eng.defs[lvl]['input-node'][flow]
      if def then
         type = 'input'
         break
      end
      def = eng.defs[lvl]['output-node'][flow]
      if def then
         type = 'output'
         break
      end
   end

   -- found ?
   if not def and flow[1] then
      error(eng.ERROR_NOT_DECLARED .. flow[1])
   elseif not def then
      error(eng.ERROR_NOT_DECLARED .. flow)
   end

   return def,type
end


----------------------------------------------------------------------   
--- Process definitions, implement them
--
function xFlow.engine.process_definition(flow)
   -- (0) retrieve definition
   local def,type_ = eng.get_node(flow)

   -- (1) process node depending on type
   if type_ == 'compute' then
      -- (2) create new level in computer:
      eng.computer.level = eng.computer.level + 1
      --if not eng.computer.nodes[eng.computer.level] then
      eng.computer.nodes[eng.computer.level] = {inputs = {}, outputs = {}}
      --end

      -- aliases:
      local parent_inputs = eng.computer.nodes[eng.computer.level-1].inputs
      local parent_outputs = eng.computer.nodes[eng.computer.level-1].outputs
      local child_inputs = eng.computer.nodes[eng.computer.level].inputs
      local child_outputs = eng.computer.nodes[eng.computer.level].outputs

      -- (3) connect its I/Os
      for i = 2,#flow do
         if type(flow[i][2]) == 'table' then
            -- nameless data node
            child_inputs[flow[i][1]] = eng.tensor_new(flow[i][2])
         else
            -- get type of node:
            local _,node_type_ = eng.get_node(flow[i][2])
            -- create parent input if necessary:
            if node_type_ == 'input' and not parent_inputs[flow[i][2]] then
               parent_inputs[flow[i][2]] = eng.process_definition(flow[i][2])
            end
            -- create parent output if necessary:
            if node_type_ == 'output' and not parent_outputs[flow[i][2]] then
               parent_outputs[flow[i][2]] = eng.process_definition(flow[i][2])
            end
            -- create data node in both inputs and outputs:
            if node_type_ == 'data' and not parent_outputs[flow[i][2]] then
               parent_outputs[flow[i][2]] = eng.process_definition(flow[i][2])
               parent_inputs[flow[i][2]] = parent_outputs[flow[i][2]]
            end
            -- connect child inputs to parent:
            child_inputs[flow[i][1]] = parent_inputs[flow[i][2]]
            -- connect child outputs to parent:
            child_outputs[flow[i][1]] = parent_outputs[flow[i][2]]
         end
      end

      -- (4) try to find a library match of each output node
      local found = false
      for i = 2,#flow do
         local outputname = flow[i][1]
         local output = child_outputs[outputname]
         -- try to see if code exists for this output (if yes, then it comes from a library)
         if def[outputname] then
            local node = def[outputname](child_inputs, output)
            eng.computer.machine[#eng.computer.name+1] = node
            eng.computer.update[#eng.computer.update+1] = node.update
            eng.computer.name[#eng.computer.name+1] = node.name
            found = true
         end
      end

      -- (5) if no output nodes were found in libraries, try to find it in the locals
      if not found then
         eng.implement_level(def)
      end

      -- (6) decrement stack level
      eng.computer.level = eng.computer.level - 1
      return
   end

   if type_ == 'data' or type_ == 'input' or type_ == 'output' then
      return eng.tensor_new(def[1])
   end
end


----------------------------------------------------------------------   
--- Generic header functions
--
function xFlow.engine.process_require(flow)
   if not flow[2] then
      error(eng.ERROR_MISSING_ARG .. flow[1])
   elseif flow[2] == 'lib-math' and not xFlow.libs.math.loaded then
      xFlow.libs.math.load(eng.defs[#eng.defs])
   elseif flow[2] == 'lib-neural' and not xFlow.libs.neural.loaded then
      xFlow.libs.neural.load(eng.defs[#eng.defs])
   end
end

function xFlow.engine.process_version(flow)
   if not flow[2] then
      error(eng.ERROR_MISSING_ARG .. flow[1])
   else
      eng.defs[#eng.defs]['version'] = flow[2]
      print(eng.MSG_VERSION .. eng.defs[#eng.defs]['version'])
   end
end


----------------------------------------------------------------------   
--- Defines Node:
-- simply insert the flow definition in the defs stack
--
function xFlow.engine.process_node(flow)
   if flow[2] == nil then
      error(eng.ERROR_MISSING_ARG .. flow[1])
   else
      -- create new definition for node (flow[1] is the type of node,
      -- and flow[2] is its name)
      eng.defs[#eng.defs][flow[1]][flow[2]] = {}
      -- add content:
      for i = 3,#flow do
         eng.defs[#eng.defs][flow[1]][flow[2]][i-2] = flow[i]
      end
   end
end


----------------------------------------------------------------------   
--- Creates a tensor, from a flow def
-- @param flow  flow definition of the tensor
--
function xFlow.engine.tensor_new(flow)
   -- a tensor
   eng.temp_var = torch.Tensor()
   -- type
   if flow[1] == 'scalar' then
      eng.temp_var:resize(1)
      eng.process_level(flow, {['value'] = eng.tensor_init, ['type'] = eng.tensor_type}, true, 2)
   elseif flow[1] == 'tensor' then
      eng.process_level(flow, {['dim'] = eng.tensor_resize}, true, 2)
      eng.process_level(flow, {['value'] = eng.tensor_init, ['type'] = eng.tensor_type}, true, 2)
   else
      error(eng.ERROR_UNKNOWN_TYPE .. flow[1])
   end
   return eng.temp_var
end


----------------------------------------------------------------------   
--- functions to resize, init and change type of tensor
-- @param flow  flow definition of the tensor
--
function xFlow.engine.tensor_init(flow)
   local linear_storage = eng.temp_var:storage()
   local size = linear_storage:size()
   if size ~= #flow-1 then
      error(eng.ERROR_BADDATA .. 'tensor size is '..size..', given data is '..#flow-1)
   end
   for i = 1,size do
      linear_storage[i] = flow[1+i]
   end
end

function xFlow.engine.tensor_type(flow)
   if not flow[2] then
      error(eng.ERROR_MISSING_ARG .. flow[1])
   end
   if flow[2] ~= 'float' and flow[2] ~= 'integer' then
      error(eng.ERROR_UNKNOWN_TYPE .. flow[2])
   end
end

function xFlow.engine.tensor_resize(flow)
   -- get dimensions
   local dim = torch.LongStorage(#flow-1)
   for i = 1,#flow-1 do
      if flow[#flow+1-i] == '?' then
         dim[i] = 1
      else
         dim[i] = tonumber(flow[#flow+1-i])
      end
   end
   eng.temp_var:resize(dim)
end



----------------------------------------------------------------------   
--- reorders computations, and does some postprocessing operations
-- on the computer
-- @param none
--
function xFlow.engine.finalize_computer()

   -- TODO: reorder update list in case of parallel flow:
   -- eng.computer.update[]

   local nodes = eng.computer.nodes
   local update = eng.computer.update
   local name = eng.computer.name
   local machine = eng.computer.machine

   -- provides easy update function
   eng.computer.update = function (eng,args) 
                             for i=1,#update do update[i](args) end 
                          end

   -- a debug function
   eng.computer.__tostring = function (eng,tab) 
                                local str = ''
                                local tab = tab or ''
                                for i=1,#name do
                                   str = str .. tab .. '- <' .. name[i]..'>'
                                   if i ~= #name then str = str .. '\n' end
                                end
                                return str
                             end

   -- metatable
   setmetatable(eng.computer, {__tostring=eng.computer.__tostring})

   -- create links to inputs/outputs
   eng.computer.input = {}
   eng.computer.output = {}
   for i,v in pairs(nodes[1].inputs) do
      if i == 'update' then
         eng.computer['update_'] = v
         table.insert(eng.computer.input,v)
      else
         eng.computer[i] = v
         table.insert(eng.computer.input,v)
      end
   end
   for i,v in pairs(nodes[1].outputs) do
      if i == 'update' then
         eng.computer['update_'] = v
         table.insert(eng.computer.output,v)
      else
         eng.computer[i] = v
         table.insert(eng.computer.output,v)
      end
   end

   -- done
   return eng.computer
end
