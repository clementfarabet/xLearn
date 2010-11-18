
----------------------------------------------------------------------
--- Class: Linker
--
-- This class is used to manage and link the bytecode.
-- The bytecode contains processes and data:
-- (1) a process is an action recognized by the virtual machine
--     running on the dataflow computer
-- (2) data is used by processes
--
local Linker = torch.class('Linker')

function Linker:__init(args)
   -- args
   self.logfile = args.logfile or nil

   -- the bytecode array
   self.process = {}
   self.processp = 1
   self.datap = 1 -- unused

   -- initial offsets 
   self.start_process_x = 0 -- initial offset here!!!
   self.start_process_y = 0 -- initial offset here!!!

   self.start_text = (args.start_text or linker.offset_text) + 1

   -- only if we start NOT from page zero
   if(self.start_text ~= 1) then
      -- init with default code
      self.process = bootloader.content

 -- pad the array with instructions
 for i = #bootloader.content+1,self.start_text do
    self.process[i] = 0
 end
 self.processp = self.start_text

 -- calculate start_x and start_y for collision check
 self.start_process_x = 0 
 self.start_process_y = self.start_text / streamer.stride_b
   end

   self.counter_bytes = 0

end


function Linker:addInstruction(args)
   -- parse args
   local opcode = args.opcode or oFlower.op_nop
   local arg8_1 = args.arg8_1 or 0
   local arg8_2 = args.arg8_2 or 0
   local arg8_3 = args.arg8_3 or 0
   local arg32_1 = args.arg32_1 or 0

   -- serialize opcode + args
   self.process[self.processp] = math.floor(arg32_1/256^0) % 256; self.processp = self.processp+1
   self.process[self.processp] = math.floor(arg32_1/256^1) % 256; self.processp = self.processp+1
   self.process[self.processp] = math.floor(arg32_1/256^2) % 256; self.processp = self.processp+1
   self.process[self.processp] = math.floor(arg32_1/256^3) % 256; self.processp = self.processp+1
   self.process[self.processp] = arg8_3;		 self.processp = self.processp + 1
   self.process[self.processp] = arg8_2;		 self.processp = self.processp + 1
   self.process[self.processp] = arg8_1;		 self.processp = self.processp + 1
   self.process[self.processp] = opcode;		 self.processp = self.processp + 1
end

function Linker:addDataUINT32(uint32)
   self.process[self.processp] = math.floor(uint32/256^0) % 256; self.processp = self.processp + 1
   self.process[self.processp] = math.floor(uint32/256^1) % 256; self.processp = self.processp + 1
   self.process[self.processp] = math.floor(uint32/256^2) % 256; self.processp = self.processp + 1
   self.process[self.processp] = math.floor(uint32/256^3) % 256; self.processp = self.processp + 1
end


function Linker:addProcess(process, metaprocess)
   bytes_curr = (self.processp-1)
   bytes_coming = #process
   cur_page = math.floor(bytes_curr/oFlower.page_size_b)
   new_page = math.floor((bytes_curr + bytes_coming)/oFlower.page_size_b)
   self.logfile:write("***in Linker:addProcess\n")
   self.logfile:write(string.format("bytes currently: %d, bytes coming: %d\n",
				bytes_curr, bytes_coming))
   
   local c = toolBox.COLORS
   if bytes_coming > oFlower.page_size_b then
      print(c.Red .. 'WARNING' .. c.red .. ' inserting a process that is larger than a cache page')
      print('\tthis might result in unpredictable execution' .. c.none)
   end

   if (cur_page < new_page) then
      bytes_left = oFlower.page_size_b - (bytes_curr % oFlower.page_size_b)
      -- insert goto instruction to realign
      if (bytes_left > 0) then
	 self:addInstruction{opcode = oFlower.op_goto,
			     arg32_1 = (bytes_curr+bytes_left)/8}
      end
      bytes_left = oFlower.page_size_b - ((self.processp-1) % oFlower.page_size_b)
      self.logfile:write(string.format("bytes padding: %d\n",bytes_left))
      for i = 1,bytes_left do
         self.process[self.processp] = 0
         self.processp = self.processp + 1
      end
   end
   
   bytes_curr = self.processp
   for i = 1,#process do
      self.process[self.processp] = process[i]
      self.processp = self.processp + 1
   end

   -- resolve relative addresses (for gotos)
   local saved_pointer = self.processp
   for i=1,#process do
      if metaprocess[i] ~= nil then
	 self.processp = bytes_curr + i - 1 -- this address needs to be replaced
	 self:addDataUINT32( (bytes_curr + i - 2)/8 + metaprocess[i]) -- insert new address
      end
   end
   self.processp = saved_pointer
end


function Linker:dump(info, mem)
   -- parse argument
   if (info ~= nil) then
      -- get defaults if nil
      info.file = info.file or 'stdout'
      info.offsetData = info.offsetData or self.processp
      info.offsetProcess = info.offsetProcess or 0
      info.bigendian = info.bigendian or 0
      info.dumpHeader = info.dumpHeader or false
      info.writeArray = info.writeArray or false
   else
      -- Default params
      info = {file='stdout', 
	      offsetData=self.processp, offsetProcess=0, 
	      bigendian=0, dumpHeader=false, writeArray=false}
   end

   local out
   
   if(info.writeArray) then  -- we are writing arrays for C
      out = assert(io.open('boot_code', "w"))
      out:write('const uint8 bytecode_exampleInstructions[] = {');
   else -- writing bytecode to file or stdout
      -- select output
      if (info.file == 'stdout') then
         out = io.stdout
      else
         out = assert(io.open(info.file, "wb"))
      end
      -- print optional header
      if (info.dumpHeader) then
         out:write(info.offsetProcess, '\n') -- offset in mem
         out:write(info.offsetData - info.offsetProcess + (self.datap - 1)*4, '\n') -- size
         out:write((self.datap-1)*4, '\n') -- data size
      end
   end
   
   -- print all the instructions
   self:dump_instructions(info, out)
   
   if(info.writeArray == false)then
      -- and raw_data
      self:dump_RawData(info, out, mem)
      -- and data (images) for simulation)
      self:dump_ImageData(info, out, mem)
   end
   
   -- and close the file
   if(info.writeArray) then  -- we are writing arrays for C
      out:write('0};\n\n');
      assert(out:close())
   else -- writing bytecode to file or stdout
      if (info.file ~= 'stdout') then
         assert(out:close())
      end
   end
   
   -- check collisions:
   self:checkCollisions(info, mem)
end

function Linker:checkCollisions(info, mem)
   -- processes are 1 byte long, numbers are 2 (streamer.word_b) bytes long
   
   offset_bytes_process = self.start_process_y * streamer.stride_b
                        + self.start_process_x
   --+ self.start_process_x * streamer.word_b

   offset_bytes_rawData = mem.start_raw_data_y * streamer.stride_b
			  + mem.start_raw_data_x * streamer.word_b

   offset_bytes_data = mem.start_data_y * streamer.stride_b
		       + mem.start_data_x * streamer.word_b

   offset_bytes_buffer = mem.start_buff_y * streamer.stride_b
			 + mem.start_buff_x * streamer.word_b

   size_raw_data = (mem.raw_data_offset_y - mem.start_raw_data_y) * streamer.stride_b
		 + (mem.raw_data_offset_x - mem.start_raw_data_x - mem.last_align) * streamer.word_b

   size_data =	(mem.data_offset_y - mem.start_data_y) * streamer.stride_b

   if (mem.data_offset_x ~= 0) then -- if we did not just step a new line
      -- take into account all the lines we wrote (the last entry's hight is enough)
      -- if not all the lines are filled till the end we are counting more than we should here,
      -- but for checking collision it's OK
      size_data = size_data + mem.data[mem.datap - 1].h * streamer.stride_b
   end
   
   size_buff =  (mem.buff_offset_y - mem.start_buff_y) * streamer.stride_b
   
   if (mem.buff_offset_x ~= 0) then -- if we did not just step a new line
      -- take into account all the lines we wrote (the last entry's hight is enough)
      -- if not all the lines are filled till the end we are counting more than we should here,
      -- but for checking collision it's OK
      size_buff = size_buff + mem.buff[mem.buffp - 1].h * streamer.stride_b
   end

   local c = toolBox.COLORS
   print("++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++")
   print(c.Cyan .. '-openFlow-' .. c.Magenta .. ' Compilation Report ' .. 
         c.none ..'[' .. info.file .. "]\n")
   print(string.format("    bytecode segment: start = %10d, size = %10d, end = %10d",
		       offset_bytes_process, 
		       (self.processp-offset_bytes_process-1), 
		       offset_bytes_process+(self.processp-offset_bytes_process)-1))
   if ((self.processp-offset_bytes_process) + offset_bytes_process > offset_bytes_rawData) then
      print(c.Red .. 'ERROR' .. c.red .. ' segments overlap' .. c.none)
   end
   print(string.format("kernels data segment: start = %10d, size = %10d, end = %10d",
		       offset_bytes_rawData, 
		       size_raw_data, 
		       offset_bytes_rawData+size_raw_data))
   if (offset_bytes_rawData+size_raw_data > offset_bytes_data) then
      print(c.Red .. 'ERROR' .. c.red .. ' segments overlap' .. c.none)
   end
   print(string.format("  image data segment: start = %10d, size = %10d, end = %10d",
		       offset_bytes_data,
		       size_data, 
		       offset_bytes_data+size_data))
   if (offset_bytes_data+size_data > offset_bytes_buffer) then
      print(c.Red .. 'ERROR' .. c.red .. ' segments overlap' .. c.none)
   end
   print(string.format("        heap segment: start = %10d, size = %10d, end = %10d",
		       offset_bytes_buffer, size_buff, memory.size_b))

   print(string.format("                                  the binary file size should be = %10d",
		       self.counter_bytes))
   print("++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++")
end

function Linker:dump_instructions(info, out)
   for i=1,(self.processp-1) do
      if (info.writeArray) then
	 out:write(string.format('0x%02X, ', self.process[i]))
      else
	 --print(string.format('0x%02X, ', self.process[i]))
	 -- 0 needs to be handled separately... isn't that crazy ???
	 if self.process[i] == 0 then
	    out:write('\0')
	    --print(string.format("in dump_inst,self.process[i]: %c", 0))
	 else
	    --print(string.format("in dump_inst,self.process[i]: %c", self.process[i]))
	    out:write(string.format("%c", self.process[i]))
	 end
	 self.counter_bytes = self.counter_bytes + 1
      end
   end
end

function Linker:dump_RawData(info, out, mem)
   

   -- pad initial offset for raw data
   self.logfile:write("Kernels:\n")
   offset_bytes =  mem.start_raw_data_y * streamer.stride_b
		 + mem.start_raw_data_x * streamer.word_b
   for p=out:seek(),offset_bytes-1 do
      out:write('\0')
      self.counter_bytes = self.counter_bytes + 1
   end
  
   for i=1,(mem.raw_datap-1) do
      mem_entry = mem.raw_data[i]
      self.logfile:write(string.format("#%d, offset_x = %d, offset_y = %d\n",
                                       i,mem_entry.x,mem_entry.y))
      -- set offset in file
      offset_bytes = mem_entry.y * streamer.stride_b + mem_entry.x * streamer.word_b
      -- pad alignment 
      for p=out:seek(),offset_bytes-1 do
         out:write('\0')
	 self.counter_bytes = self.counter_bytes + 1
      end
      if (mem_entry.bias ~= nil) then
         self.logfile:write("Bias:\n")
	 for b = 1,mem_entry.bias:size(1) do
            dataTwos = math.floor(mem_entry.bias[b] * num.one + 0.5)
            dataTwos = bit.band(dataTwos, num.mask)
            for j=0,(num.size_b - 1) do
               -- get char from short
               if (info.bigendian == 1) then
                  tempchar = math.floor(dataTwos / (256^((num.size_b - 1)-j))) % 256
               else
                  tempchar = math.floor(dataTwos / (256^j)) % 256
               end
               -- then dump the char
               if tempchar == 0 then
                  out:write('\0')
               else
                  out:write(string.format("%c", tempchar))
               end
	       self.counter_bytes = self.counter_bytes + 1
            end
            -- print the kernel to logFile:
            self.logfile:write(string.format("%d ", mem_entry.bias[b]))
	 end
         self.logfile:write(string.format("\n"))
      end
      
      self.logfile:write("Kernel:\n")
      for r=1,mem_entry.data:size(1) do
         for c=1,mem_entry.data:size(2) do
            dataTwos = math.floor(mem_entry.data[r][c] * num.one + 0.5)
            dataTwos = bit.band(dataTwos, num.mask)
            for j=0,(num.size_b - 1) do
               -- get char from short
               if (info.bigendian == 1) then
                  tempchar = math.floor(dataTwos / (256^((num.size_b - 1)-j))) % 256
               else
                  tempchar = math.floor(dataTwos / (256^j)) % 256
               end
               -- then dump the char
               if tempchar == 0 then
                  out:write('\0')
               else
                  out:write(string.format("%c", tempchar))
               end
	       self.counter_bytes = self.counter_bytes + 1
            end
            -- print the kernel to logFile:
            self.logfile:write(string.format("%d ", mem_entry.data[r][c]))
	 end
         self.logfile:write(string.format("\n"))
      end
   end
end
 
function Linker:dump_ImageData(info, out, mem)
   if (mem.data[1] == nil) then
      return
   end
   -- pad initial offset for raw data
   offset_bytes =  mem.start_data_y*streamer.stride_b + mem.start_data_x*streamer.word_b
   for p=out:seek(),offset_bytes-1 do
      out:write('\0')
   end
   mem_entry = mem.data[1]
   self.logfile:write(string.format("Writing images from offset: %d\n", 
				    mem.start_data_y*streamer.stride_w
                                       + mem.start_data_x))
   for r=1,mem_entry.h do
      for i=1,(mem.datap-1) do
         mem_entry = mem.data[i]
         offset_bytes = (mem_entry.y + r - 1)*streamer.stride_b + mem_entry.x*streamer.word_b
         for p=out:seek(),offset_bytes-1 do
            out:write('\0')
         end
         for c=1, mem_entry.w do
            dataTwos = math.floor(mem_entry.data[c][r] * num.one + 0.5)
            dataTwos = bit.band(dataTwos, num.mask)
            self.logfile:write(string.format("%d ",dataTwos))--mem_entry.data[r][c]))
            for j=0,(num.size_b - 1) do
               -- get char from short
               if (info.bigendian == 1) then
                  tempchar = math.floor(dataTwos / (256^((num.size_b - 1)-j))) % 256
               else
                  tempchar = math.floor(dataTwos / (256^j)) % 256
               end
               -- then dump the char
               if tempchar == 0 then
                  out:write('\0')
               else
                  out:write(string.format("%c", tempchar))
               end
            end
         end -- column
         self.logfile:write(string.format("\t"))
      end -- entry
      self.logfile:write(string.format("\n"))
   end -- row
end

------------ these functions are for testing --------------------------------------

function Linker:dump_instructions_log(out)
   -- set initial offset for instructions in file
   --out:seek("set", 1024*2)
   out:write(string.format("****************instructions****************"))
   for i=1,(self.processp-1) do
      out:write(string.format('_%d', self.process[i])) 
   end
end

function Linker:dump_data_log(mem, out)
   -- set initial offset for raw data in file
   offset_bytes =  mem.start_raw_data_y * streamer.stride_b
		 + mem.start_raw_data_x * streamer.word_b
   out:seek("set",offset_bytes)
   --out:write(string.format("****************raw_data****************"))
   for i=1,(mem.raw_datap-1) do
      mem_entry = mem.raw_data[i]
      -- set offset in file
      offset_bytes = mem_entry.y * streamer.stride_b + mem_entry.x * streamer.word_b
      out:seek("set", offset_bytes)
      print('mem_entry.y = ',mem_entry.y ,'mem_entry.x = ',mem_entry.x)
      print('for mem_entry #',i,' the offset is = ', offset_bytes)
      if (mem_entry.bias ~= nil) then
	 for b = 1,mem_entry.bias:size(1) do
	    out:write(string.format("_%d", mem_entry.bias[b]))
	 end
      end
      for r=1,mem_entry.data:size(1) do
         for c=1, mem_entry.data:size(2) do
            out:write(string.format('_%d', mem_entry.data[r][c]))
         end
      end
   end
   
   -- set initial offset for data in file
   -- out:seek("set", ( mem.start_data_y - 1)*streamer.stride_b + mem.start_data_x)
--    out:write(string.format("****************data****************"))
--    for i=1,(mem.datap-1) do
--       mem_entry = mem.data[i]
--       for r=1,mem_entry.data:size(1) do
--          -- set offset in file
--          offset_bytes = (mem_entry.y + r - 1)*streamer.stride_b + mem_entry.x*streamer.word_b
--          out:seek("set", offset_bytes)
--          print('mem_entry.y = ',mem_entry.y ,'mem_entry.x = ',mem_entry.x)
--          print('for mem_entry #',i,'row #', r,' the offset is = ', offset_bytes)
--          for c=1, mem_entry.data:size(2) do
--             out:write(string.format('_%d', mem_entry.data[r][c]/100))
--          end
--       end
--    end	    
   
end

