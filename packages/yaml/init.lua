--==============================================================================
-- File: yaml wrapper
--
-- Description: A wrapper for using yaml parser
--              
--
-- Created: March 31, 2011, 2:26PM
--
-- Author: Benoit Corda -- corda@cs.nyu.edu
--==============================================================================

require 'XLearn'
require 'libyaml'

if not yaml then
   ------------------------------------------------------------
   -- Wrapper for yaml parse
   --
   -- To load: require 'yaml'
   --
   -- @release 2011 Benoit Corda
   ------------------------------------------------------------
   local libyaml = libyaml
   local paths = paths
   local io = io
   local print = print
   local assert = assert
   local xerror = xerror
   local toolBox = toolBox
   local torch = torch
   local string = string
   module('yaml')

   ------------------------------------------------------------
   -- Read a yaml file and return a table
   ------------------------------------------------------------
   loadfile = function(...)
                   local args, filename = toolBox.unpack(
                      {...},
                      'yaml.loadfile',
                      'load a file with yaml format and return a parsed table',
                      {arg='file', type='string', help='path of the file to be parsed'}
                   )
                   if not paths.filep(filename) then
                      xerror("<yaml.loadfile> " ..filename.. " doesn't exist")
                   end
                   local myf = io.open(filename)
                   local mystr = ''
                   for line in myf:lines() do
                      -- skip comments
                      if not string.find( line,"^%s*%%") then
                         mystr = mystr ..'\n'.. line
                      end
                   end
                   return libyaml.load(mystr)
              end

   testme = function()
               local yamlstr = [[
                     cars:
                        - &car1
                     make: Audi
                     model: S4
                        - &car2
                     make: VW
                     model: GTI
                     favorite: *car1
               ]]

               local result = libyaml.load(yamlstr)
               assert(result.favorite == result.cars[1])
               print('String was:[[',yamlstr,']]')
               print('parsed to:',result)
            end
end

return opencl
