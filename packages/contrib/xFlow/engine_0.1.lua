--------------------------------------------------------------------------------
-- XFlow: a class to handle xFlow programs.
-- 
-- what?  xFlow programs can be used to represent algorithms that operate on
--        homogenous data structures.
--        It's particularly well suited to describe networks of transformations.
--
--        This class implements a simple xFlow parser, and a back-end to exec
--        an xFlow program using the Torch Environment.
--
-- version: 0.1
--------------------------------------------------------------------------------

require 'XLearn'

do
   local XFlow = torch.class('XFlow')

   
   ----------------------------------------------------------------------
   --- Buils the xFlow engine.
   -- @param verbose  be verbose during parsing
   --
   function XFlow:__init(args)
      self.verbose = false
      if (args) then
         self.verbose = args.verbose or false
      end
      self.parsed = nil
      self.version = nil
      self.stack = {}
      self.currentData = {}
      self.computer = nil

      -- messages
      self.MSG_PARSING = '# INFO: parsing xFlow'
      self.WARNING_UNBALANCED = "# xFlow WARNING: unbalanced s-exp"
      self.ERROR_IMPLEMENTED = '# ERROR: non implemented > '
      self.WARNING_IMPLEMENTED = '# WARNING: non implemented > '
      self.MSG_VERSION = '# INFO: xFlow version = '
      self.ERROR_MISSING_ARG = '# ERROR: missing argument > '
      self.ERROR_BADDATA = '# ERROR: bad data format > '
   end


   ----------------------------------------------------------------------   
   --- Parses an xFlow string.
   -- Returns a table that contains the whole hierarchy.
   -- @param flow   xFlow string.
   --
   function XFlow:parse(flow)
      local stack = {}
      local top = {}
      table.insert(stack, top)
      local ni,c,label,xarg,nilpattern,nolabel,comment
      local s = flow
      local i, j = 1, 1
      print(self.MSG_PARSING)
      while true do
         comment,nilpattern,nolabel,label,xarg,c = nil,nil,nil,nil,nil,nil

         -- find comments
         while true do
            ni,j,comment = string.find(s, "^%s*%;%;%s?([%s*%w.-]+)\n", i)
            if (comment ~= nil) then
               print(';; ' .. comment)
               i = j+1
            else
               break
            end
         end

         -- parse s-expr
         ni,j,nilpattern = string.find(s, "^%s*(%(%s*%))", i)
         if (nilpattern ~= nil) then
            label = 'nil'
            c = true
         else
            ni,j,label = string.find(s, "^%s*%(%s*([%w]+)", i)
            if (label == nil) then 
               ni,j,xarg = string.find(s, "^%s*([-%w.+]+)%s*%)", i)
               if (xarg == nil) then
                  ni,j,c = string.find(s, "^%s*(%))", i)
                  if (c == nil) then
                     ni,j,xarg = string.find(s, "^%s*([-%w.+]+)", i)
                  else
                     c = true
                  end
               else
                  c = true
               end
            end
         end

         -- end of s-exp
         if not ni then break end

         -- add symbols to stack
         if label ~= nil then
            top = {label=label, arg={}, nargs=0}
            table.insert(stack, top)   -- new level
            if (self.verbose) then
               print('new level: ' .. label)
            end
         elseif xarg ~= nil then   -- start tag
            stack[#stack].nargs = stack[#stack].nargs + 1
            stack[#stack].arg[stack[#stack].nargs] = xarg
            if (self.verbose) then
               print('appending arg: ' .. xarg)
            end
         end

         -- close
         if (c) then  -- end tag
            if (self.verbose) then
               print('close level: ' .. stack[#stack].label)
            end
            local toclose = table.remove(stack)  -- remove top
            top = stack[#stack]
            if #stack < 1 then
               error("nothing to close with "..label.." i="..i)
            end
            table.insert(top, toclose)
         end
         i = j+1
      end

      if #stack > 1 then
         print(self.WARNING_UNBALANCED)
      end

      -- save and return result
      self.parsed = stack[1]
      return self.parsed
   end


   ----------------------------------------------------------------------
   --- Implements an xFlow program within Torch framework
   -- Converts the given xFlow code to run natively within Torch.
   -- @param flow  an xFlow program (file).
   --
   function XFlow:implement(flow)
      -- open file
      local f = assert(io.open(flow, "r"))
      local s = f:read("*all")
      f:close()

      -- parse
      self:parse(s)

      -- extract code
      local flow = self.parsed
      if flow[1].label == 'xflow' then
         self:process_xflow(flow[1])
      else
         error(self.ERROR_IMPLEMENTED .. flow[1].label)
      end

      -- give stats/results
      print('# mapped xFlow code to torch.nn.module 100% OK')
      
      return self.computer
   end


   ----------------------------------------------------------------------   
   --- Generic function to parse an operation.
   -- @param flow        parsed flow program
   -- @param callback    callback table {name=callback, ...}
   --
   function XFlow:process_level(flow, callback)
      local idx = 1
      while true do
         if flow[idx] == nil then
            break
         end
         local callback = callback[flow[idx].label]
         if callback then
            callback(self, flow[idx])
         else 
            error(self.ERROR_IMPLEMENTED .. flow[idx].label)
         end
         idx = idx + 1
      end
   end


   ----------------------------------------------------------------------   
   --- All the functions process_*** can parse a list of type ***.
   -- For example, process_xflow parses (xflow ...)
   -- @param flow        parsed flow program
   --
   function XFlow:process_xflow(flow)
      -- parse args
      self:process_level(flow, {version=self.process_version, 
                                sequence=self.process_sequence,
                                network=self.process_network})
   end

   function XFlow:process_version(flow)
      if flow.arg[1] == nil then
         error(self.ERROR_MISSING_ARG .. flow.label)
      else
         self.version = flow.arg[1]
         print(self.MSG_VERSION .. self.version)
      end
   end

   function XFlow:process_sequence(flow)
      -- create sequential to hold computations
      self.computer = nn.Sequential()

      -- parse args
      self:process_level(flow, {input=self.process_input, 
                                convolution=self.process_convolution,
                                subsampling=self.process_subsampling,
                                transformation=self.process_transformation,
                                dotproduct=self.process_dotproduct})
   end

   function XFlow:process_network(flow)
      print(self.WARNING_IMPLEMENTED .. flow.label)
   end


   ----------------------------------------------------------------------   
   --- Data
   --
   function XFlow:process_input(flow)
      -- expected args
      self.stack[#self.stack+1] = {tensor=nil}

      -- process args
      self:process_level(flow, {tensor=self.process_tensor})
      
      -- get table or create default table
      if not self.stack[#self.stack].tensor then
         error(self.ERROR_MISSING_ARG .. flow.label)
      end

      self.currentData = self.stack[#self.stack].tensor

      -- clear stack
      self.stack[#self.stack] = nil
   end
   
   function XFlow:process_tensor(flow)
      if flow.nargs == 0 then
         error(self.ERROR_MISSING_ARG .. flow.label)
      end
      -- revert dimensions (Torch is like Fortran...)
      local dim = torch.LongStorage(flow.nargs)
      local size = 1
      for i = 1,flow.nargs do
         dim[i] = flow.arg[flow.nargs+1-i]
         size = size*dim[i]
      end

      -- build tensor
      local tensor = torch.Tensor(dim)

      -- fill tensor (use a linear tensor to make things way easier)
      if flow[1] and flow[1].label ~= 'nil' then
         local source = flow[1]
         if size ~= source.nargs then
            error(self.ERROR_BADDATA .. flow.label)
         end
         local target = torch.Tensor(tensor:storage(), 1, size)
         for i = 1,size do
            target[i] = source.arg[i]
         end
      end

      -- return on stack
      self.stack[#self.stack].tensor = tensor
   end


   ----------------------------------------------------------------------   
   --- Operators
   --
   function XFlow:process_convolution(flow)
      -- we expect these arguments:
      self.stack[#self.stack+1] = {kerx=3, kery=3,
                                   stridex=1, stridey=1,
                                   weight=nil,
                                   bias=nil,
                                   table=nil}

      -- parse args
      self:process_level(flow, {connex=self.process_connex, 
                                stride=self.process_stride,
                                weight=self.process_weight,
                                bias=self.process_bias})

      -- append convolver
      self.computer:add(nn.SpatialConvolutionTable(self.stack[#self.stack].table,
                                                   self.stack[#self.stack].kerx,
                                                   self.stack[#self.stack].kery,
                                                   self.stack[#self.stack].stridex,
                                                   self.stack[#self.stack].stridey))
      -- initialize content
      if self.stack[#self.stack].weight then
         self.computer.modules[#self.computer.modules].weight = self.stack[#self.stack].weight
      end
      if self.stack[#self.stack].bias then
         self.computer.modules[#self.computer.modules].bias = self.stack[#self.stack].bias
      end

      -- clear stack
      self.stack[#self.stack] = nil
   end

   function XFlow:process_subsampling(flow)
      -- we expect these arguments:
      self.stack[#self.stack+1] = {stridex=2, stridey=2,
                                   weight=nil,
                                   bias=nil}

      -- parse args
      self:process_level(flow, {stride=self.process_stride,
                                weight=self.process_weight,
                                bias=self.process_bias})

      -- append convolver
      self.computer:add(nn.SpatialSubSampling(self.computer:forward(self.currentData):size(3),
                                              self.stack[#self.stack].stridex,
                                              self.stack[#self.stack].stridey,
                                              self.stack[#self.stack].stridex,
                                              self.stack[#self.stack].stridey))
      -- initialize content
      if self.stack[#self.stack].weight then
         self.computer.modules[#self.computer.modules].weight = self.stack[#self.stack].weight
      end
      if self.stack[#self.stack].bias then
         self.computer.modules[#self.computer.modules].bias = self.stack[#self.stack].bias
      end
      
      -- clear stack
      self.stack[#self.stack] = nil
   end

   function XFlow:process_dotproduct(flow)
      -- we expect these arguments:
      self.stack[#self.stack+1] = {stridex=2, stridey=2,
                                   weight=nil,
                                   bias=nil}

      -- parse args
      self:process_level(flow, {weight=self.process_weight,
                                bias=self.process_bias})
      
      -- append convolver
      self.computer:add(nn.SpatialLinear(self.computer:forward(self.currentData):size(3),
                                         self.stack[#self.stack].bias:size(1)))
      -- initialize content
      if self.stack[#self.stack].weight then
         self.computer.modules[#self.computer.modules].weight = self.stack[#self.stack].weight
      end
      if self.stack[#self.stack].bias then
         self.computer.modules[#self.computer.modules].bias = self.stack[#self.stack].bias
      end
      
      -- clear stack
      self.stack[#self.stack] = nil
   end

   function XFlow:process_transformation(flow)
      -- parse args
      self:process_level(flow, {mult=self.process_mult,
                                tanh=self.process_tanh})
   end

   function XFlow:process_mult(flow)
      self.computer:add(nn.Mult(flow.arg[1]))
   end

   function XFlow:process_tanh(flow)
      self.computer:add(nn.Tanh())
   end


   ----------------------------------------------------------------------   
   --- Arguments
   --
   function XFlow:process_connex(flow)
      -- expected args
      self.stack[#self.stack+1] = {tensor=nil}

      -- process args
      self:process_level(flow, {tensor=self.process_tensor})
      
      -- get table or create default table
      if self.stack[#self.stack].tensor then
         self.stack[#self.stack-1].table = self.stack[#self.stack].tensor:t()
         self.stack[#self.stack-1].table:add(1) -- 1 based...
      else
         self.stack[#self.stack-1].table = nn.SpatialConvolutionTable:FullTable(1,1)
      end
      
      -- clear stack
      self.stack[#self.stack] = nil
   end

   function XFlow:process_weight(flow)
      -- expected args
      self.stack[#self.stack+1] = {tensor=nil}

      -- process args
      self:process_level(flow, {tensor=self.process_tensor})
      
      -- get table or create default table
      if not self.stack[#self.stack].tensor then
         error(self.ERROR_MISSING_ARG .. flow.label)
      end
      self.stack[#self.stack-1].weight = self.stack[#self.stack].tensor

      -- clear stack
      self.stack[#self.stack] = nil

      -- store kernel dims
      if self.stack[#self.stack].weight:nDimension() > 1 then
         self.stack[#self.stack].kerx = tonumber(self.stack[#self.stack].weight:size(1))
         self.stack[#self.stack].kery = tonumber(self.stack[#self.stack].weight:size(2))
      end
   end

   function XFlow:process_bias(flow)
      -- expected args
      self.stack[#self.stack+1] = {tensor=nil}

      -- process args
      self:process_level(flow, {tensor=self.process_tensor})
      
      -- get table or create default table
      if not self.stack[#self.stack].tensor then
         error(self.ERROR_MISSING_ARG .. flow.label)
      end
      self.stack[#self.stack-1].bias = self.stack[#self.stack].tensor

      -- clear stack
      self.stack[#self.stack] = nil
   end

   function XFlow:process_stride(flow)
      self.stack[#self.stack].stridex = tonumber(flow.arg[flow.nargs])
      self.stack[#self.stack].stridey = tonumber(flow.arg[flow.nargs-1])
      if not flow.arg[1] or not flow.arg[2] then
         error(self.ERROR_BADDATA .. flow.label)
      end
   end
end
