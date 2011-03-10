--------------------------------------------------------------------------------
-- table: extends 'table' package with a bunch of useful functions
-- 
-- Authors: Clement Farabet
--------------------------------------------------------------------------------
do
   ----------------------------------------------------------------------
   --- Returns a table of a given size
   -- @param   size   size
   -- @param   fill   init with content
   --
   function table.new(size,fill)
      -- args
      local fill = fill or 0

      -- create table
      local new = {}
      for i = 1,size do
         new[i] = fill
      end
      return new
   end

   ----------------------------------------------------------------------
   --- Returns a table of a given size, by calling a function
   -- @param   size   size
   -- @param   func   init content by calling function
   -- @param   ...    any params to be passed to the function
   --
   function table.generate(size,func,...)
      -- create table
      local new = {}
      for i = 1,size do
         new[i] = func(...)
      end
      return new
   end
end
