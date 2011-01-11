#!/usr/bin/env qlua

------------------------------------------------------------
-- tests a few segmentation algorithms available
-- in the repo
--

require 'mstsegm'
mstsegm.testme()

require 'powerwatersegm'
powerwatersegm.testme()

print([[

To know more about these algos, start a Lua shell, and try:
> require 'mstsegm'
> img = image.load('/path/to/image.jpg')
> mstsegm.infer()
... prints some online help on the function ...
> result = mstsegm.infer{image=img} 
> image.display(result)
> require 'powerwatersegm'
> powerwatersegm.infer() 
]])
