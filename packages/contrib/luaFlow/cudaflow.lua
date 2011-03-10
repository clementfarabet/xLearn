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
--     cudaflow - a handler for our CUDA-enabled GPUs
--
-- history: 
--     September 12, 2010, 6:45PM
----------------------------------------------------------------------

cudaflow = {}

----------------------------------------------------------------------
-- cudaflow.ArrayT: an extension of luaflow.ArrayT for cudaflow
-- it provides the same functions, same API, but everything done
-- here runs on cudaflow rather than on the local machine
----------------------------------------------------------------------
cudaflow.ArrayT = {}
cudaflow.ArrayT.mt = {}

function cudaflow.ArrayT.new(...)
   -- init geometry
   local dims = {...}
   if type(dims[1]) == 'table' then
      dims = dims[1]
   end
   local array = {}
   array.type = 'cudaflow.ArrayT'
   array.data = {}
   array.data_start = 1
   array.dims = dims
   array.size = 1
   for i = 1,#dims do
      array.size = array.size * dims[i]
   end
   array.data_end = array.data_start + array.size - 1
   -- set metatable for easy operators
   setmetatable(array, cudaflow.ArrayT.mt)
   -- and set other operators
   for i,v in pairs(cudaflow.ArrayT) do
      array[i] = v
   end
   return array
end


----------------------------------------------------------------------
-- return cudaflow
----------------------------------------------------------------------
cudaflow.Array = cudaflow.ArrayT.new
return cudaflow
