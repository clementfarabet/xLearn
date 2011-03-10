-- 
-- Note: this bit of code is a simple wrapper around the eblearn/idx libraries
--
-- For now, it contains wrappers for:
--  + idx
--  + save_matrix
--  + load_matrix
--
-- Wrapper: Clement Farabet.
-- 

-- load C lib
require 'libleblearn'

if not leblearn then
   require 'XLearn'
   leblearn = {}

   leblearn.exportTensor
      = function(...)
           local args, tensor, path = toolBox.unpack(
              {...},
              'leblearn.exportTensor', 
              'exports a tensor to an IDX file',
              {arg='tensor', type='torch.Tensor', help='tensor of any type', req=true},
              {arg='path', type='string', help='path to IDX file (over-write)', req=true}
           )
           local idx = libleblearn.tensor2idx_double(tensor)
           local saved = libleblearn.save_double(path, idx)
           libleblearn.delete_double(idx)
           if not saved then 
              error('<leblearn.exportTensor> could not write idx to disk')
           end
        end

   leblearn.importTensor
      = function(...)
           local args, path = toolBox.unpack(
              {...},
              'leblearn.importTensor',
              'imports a tensor from an IDX file',
              {arg='path', type='string', help='path to IDX file (read-only)', req=true}
           )
           local loaded = libleblearn.load_double(path)
           local tensor = libleblearn.idx2tensor_double(loaded)
           libleblearn.delete_double(loaded)
           return tensor
        end

   leblearn.loadNetwork
      = function(...)
           local args, path = toolBox.unpack(
              {...},
              'leblearn.loadNetwork', 
              'exports a tensor to an IDX file',
              {arg='path', type='string', help='path to config file', req=true}
           )
           if path:find('.') == 1 or path:find('/') ~= 1 then
              path = paths.concat(paths.cwd(), path)
           end
           print('<leblearn.loadNetwork> loading from ' .. path)
           return libleblearn.load_network(path)
        end
end

return leblearn
