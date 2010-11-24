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
--     luaFlow - an efficient flow-graph extension for Lua
--
-- history: 
--     September 11, 2010, 3:50PM - creation
----------------------------------------------------------------------

if not luaFlowLoaded then
   require 'XLearn'
   luaFlow = {}

   -- meta data: a couple of global variables to manage a 
   -- pool of nodes
   luaFlow.pool = {}
   luaFlow.pool.nodes = {}
   luaFlow.pool.IDGEN = 1
   luaFlow.pool.register = function (obj)
                              if not obj.ID then
                                 -- give an ID to the object
                                 obj.ID = luaFlow.pool.IDGEN
                                 luaFlow.pool.IDGEN = luaFlow.pool.IDGEN + 1
                                 -- register the object with an inverted index
                                 luaFlow.pool.nodes[obj.ID] = obj
                              end
                           end
   luaFlow.pool.attach = function (parent,child)
                            if not parent.CID then parent.CID = {} end
                            table.insert(parent.CID, child.ID)
                            if not child.PID then child.PID = {} end
                            table.insert(child.PID, parent.ID)
                         end

   -- include sudmodules
   torch.include('luaFlow', 'Array.lua')
   torch.include('luaFlow', 'Node.lua')
   luaFlowLoaded = true
end

return luaFlow
