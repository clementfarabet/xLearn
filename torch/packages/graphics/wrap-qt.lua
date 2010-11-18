

require "qtwidget"
require "qttorch"

--------------------------------------
----------- MODULE GRAPHICS ----------
--------------------------------------


local qt = qt
local qtwidget = qtwidget
local torch = torch
local setmetatable = setmetatable

module('graphics')


local function declareLCairoFunctions(class)
   class.show_page = class.showpage
   class.save = class.gsave
   class.restore = class.grestore
   class.set_font_size = class.setfontsize
   class.set_line_width = class.setlinewidth
   class.move_to = class.moveto
   class.line_to = class.lineto
   class.set_source_rgb = class.setcolor
   class.set_source_rgba = class.setcolor
   class.curve_to = class.curveto
   class.set_source_surface = class.setpattern
   class.set_source_surface_xy = class.setpattern
   function class:show_text(s)
      self.port:show(s)
   end
   function class:get_width() 
      return self.width
   end
   function class:get_height() 
      return self.height
   end
   function class:clip()
      self.port:clip(true)
   end
   function class:stroke_preserve() 
      self.port:stroke(false) 
   end
   function class:fill_preserve() 
      self.port:fill(false) 
   end
   function class:reset_clip() 
      self.port:initclip() 
   end
   function class:identity_matrix() 
      self.port:initmatrix() 
   end
   function class:set_dash(s,o) 
      self.port:setdash({s,s},o) 
   end
   function class:set_not_dash() 
      self.port:setdash() 
   end
   function class:get_text_width(s) 
      return self.port:stringwidth(s) 
   end
   function class:get_text_height(s) 
      return self.port:stringrect(s):totable().height 
   end
   function class:write_to_png(s) 
      self.port:image():save(s,"png") 
   end
   function class:toTensor(t) 
      return self.port:image():toTensor(t) 
   end
   function class:begin()
      self.port:gbegin()
      self.port:gsave()
      self.port:setangleunit("Radians")
      return self
   end
   function class:theEnd()
      self.port:grestore()
      self.port:gend()
   end
end


-- windows

windowClass = {}
windowClass.__index = windowClass
setmetatable(windowClass, qtwidget.windowClass)
declareLCairoFunctions(windowClass)

function newwindow(...) 
   local r = qtwidget.newwindow(...)
   setmetatable(r, windowClass);
   return r;
end

-- images

imageClass = {}
imageClass.__index = imageClass
setmetatable(imageClass, qtwidget.imageClass)
declareLCairoFunctions(imageClass)

function newimage(...)
   local r = qtwidget.newimage(...)
   setmetatable(r, imageClass);
   return r;
end

-- printer

printerClass = {}
printerClass.__index = printerClass
setmetatable(printerClass, qtwidget.printerClass)
declareLCairoFunctions(printerClass)

function newprint(...)
   local r = qtwidget.newprint(...)
   setmetatable(r, printerClass);
   return r;
end

function newpdf(...)
   local r = qtwidget.newpdf(...)
   setmetatable(r, printerClass);
   return r;
end

function newps(...)
   local r = qtwidget.newps(...)
   setmetatable(r, printerClass);
   return r;
end


