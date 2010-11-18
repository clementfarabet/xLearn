require "torch"
require('liblcairo')

------------------------------------------------------------------

-- Cairo

local Cairo = torch.class('lcairo.Cairo')

function Cairo:__init(surface)
  self._cairo=lcairo.cairo_create(surface._surf)
  self._surf=surface.surf
  return self
end

-- NO ARGS
function Cairo:stroke()          lcairo.cairo_stroke(self._cairo)          end
function Cairo:show_page()       lcairo.cairo_show_page(self._cairo)       end
function Cairo:stroke()          lcairo.cairo_stroke(self._cairo)          end
function Cairo:stroke_preserve() lcairo.cairo_stroke_preserve(self._cairo) end
function Cairo:save()            lcairo.cairo_save(self._cairo)            end
function Cairo:restore()         lcairo.cairo_restore(self._cairo)         end
function Cairo:show_page()       lcairo.cairo_show_page(self._cairo)       end
function Cairo:fill()            lcairo.cairo_fill(self._cairo)            end
function Cairo:paint()            lcairo.cairo_paint(self._cairo)            end
function Cairo:fill_preserve()   lcairo.cairo_fill_preserve(self._cairo)   end
function Cairo:clip()            lcairo.cairo_clip(self._cairo)            end
function Cairo:reset_clip()      lcairo.cairo_reset_clip(self._cairo)      end
function Cairo:identity_matrix() lcairo.cairo_identity_matrix(self._cairo) end
-- 1 DOUBLE
function Cairo:set_line_width(x) lcairo.cairo_set_line_width(self._cairo,x) end
function Cairo:set_font_size(x)  lcairo.cairo_set_font_size(self._cairo,x)  end
function Cairo:rotate(x)         lcairo.cairo_rotate(self._cairo,x)  end
-- 2 DOUBLES
function Cairo:move_to(x,y)   lcairo.cairo_move_to(self._cairo,x,y)    end
function Cairo:line_to(x,y)   lcairo.cairo_line_to(self._cairo,x,y)    end
function Cairo:translate(x,y) lcairo.cairo_translate(self._cairo,x,y)  end
function Cairo:scale(x,y)     lcairo.cairo_scale(self._cairo,x,y)      end
-- 3 DOUBLES

function Cairo:set_source_rgb(r,g,b) lcairo.cairo_set_source_rgb(self._cairo,r,g,b)  end
-- 4 DOUBLES
function Cairo:set_source_rgba(r,g,b,a) lcairo.cairo_set_source_rgba(self._cairo,r,g,b,a)  end
function Cairo:rectangle(x,y,w,h) lcairo.cairo_rectangle(self._cairo,x,y,w,h)  end
-- 5 DOUBLES
function Cairo:arc(xc,yc,rad,ang1,ang2) lcairo.cairo_arc(self._cairo,xc,yc,rad,ang1,ang2)  end
-- 6 DOUBLES
function Cairo:curve_to(x1,y1,x2,y2,x3,y3) lcairo.cairo_curve_to(self._cairo,x1,y1,x2,y2,x3,y3)  end

-- other
function Cairo:show_text(x)  lcairo.cairo_show_text(self._cairo,x)  end

function Cairo:get_source() 
   local pat = lcairo.Pattern()
   pat._pattern = lcairo.cairo_get_source(self._cairo) 
   return pat
end

function Cairo:set_source(pattern) 
  lcairo.cairo_set_source(self._cairo,pattern._pattern) 
  return pattern
end

function Cairo:set_source_surface(s,x,y) 
  if x and y then
    lcairo.cairo_set_source_surface_xy(self._cairo,s._surf,x,y) 
  else
    lcairo.cairo_set_source_surface(self._cairo,s._surf) 
  end
end

--non standard...

function Cairo:set_dash(sep,offset) 
  lcairo.cairo_set_dash(self._cairo,sep,offset)  
end
function Cairo:set_not_dash() 
  lcairo.cairo_set_not_dash(self._cairo)  
end
function Cairo:get_text_width(txt)  
  return lcairo.cairo_get_text_width(self._cairo,txt) 
end
function Cairo:get_text_height(txt)  
  return lcairo.cairo_get_text_height(self._cairo,txt) 
end
function Cairo:get_matrix()
  return lcairo.cairo_get_matrix(self._cairo)
end
function Cairo:set_matrix(xx,yx,xy,yy,x0,y0)
  return lcairo.cairo_set_matrix(self._cairo,xx,yx,xy,yy,x0,y0)
end

------------------------------------------------------------------

-- Pattern 

local Pattern = torch.class("lcairo.Pattern")
function Pattern:__init()
end
function Pattern:__gc()
  print("garbage pattern")
end

function Pattern:set_filter(f)
  lcairo.pattern_set_filter(self._pattern, f)
end


------------------------------------------------------------------

-- Surfaces

local Surface = torch.class("lcairo.Surface")
function Surface:__gc()
  print("garbage surface")
end

--

local PDF = torch.class('lcairo.PDF','lcairo.Surface')
function PDF:__init(filename, w, h)
 self._surf = lcairo.create_pdf_surface(filename,w,h)
 self.width =w
 self.height=h
end
function PDF:close()
 lcairo.surface_destroy(self._surf)
 self._surf=nil
end

local PS = torch.class('lcairo.PS','lcairo.Surface')
function PS:__init(filename, w, h)
 self._surf = lcairo.create_ps_surface(filename,w,h)
 self.width =w
 self.height=h
end
function PS:close()
 lcairo.surface_destroy(self._surf)
 self._surf=nil
end

--

local PixelSurface = torch.class("lcairo.PixelSurface",'lcairo.Surface')
function PixelSurface:__init()
end
function PixelSurface:write_to_png(filename)
 lcairo.surface_write_to_png(self._surf,filename)
end
function PixelSurface:fromTensor(x)
 if self.width ~= x:size(1) or self.height ~= x:size(2) then
   print("Surface width and height must match!")
   return;
 end
 lcairo.THTensor_to_surface(x,self._surfdata)
end
function PixelSurface:toTensor(x)
 if self.width ~= x:size(1) or self.height ~= x:size(2) then
   print("Surface width and height must match!")
   return;
 end
 lcairo.surface_to_THTensor(self._surfdata,x)
end

local Image = torch.class('lcairo.Image','lcairo.PixelSurface')
function Image:__init(w,h) -- w may be png filename

  local imagetmp

  if type(w)=="string" then
   imagetmp={}
   imagetmp._image=lcairo.create_image_from_png(w)
   if not imagetmp._image then
    return nil
   end
   imagetmp._surf =lcairo.image_get_surface(imagetmp._image);
   w =lcairo.image_surface_get_width (imagetmp._surf)
   h =lcairo.image_surface_get_height(imagetmp._surf)
  end

  self.width =w
  self.height=h
  self._image   =lcairo.create_image(w,h)
  self._surf    =lcairo.image_get_surface(self._image);
  self._surfdata=lcairo.image_get_surface_data(self._image);
   
  -- we copy the image to a new surface to get the 
  -- data memory accessible
  if imagetmp then
   local cr=lcairo.Cairo(self)
   cr:set_source_surface(imagetmp)
   cr:rectangle(0,0,w,h)
--   pat=cr:get_source()
--   pat:set_filter(1)
   cr:fill(cr)
  end

  -- garbage_collect?

  return self
end
function Image:refresh()
end
function Image:valid()
  return true
end
 
--

local Window = torch.class('lcairo.Window','lcairo.PixelSurface')
function Window:__init(w,h,title)
  if not w then w=300 end
  if not h then h=200 end
  self.width =w
  self.height=h


  ---------------------------------------------------
  -- try this, and pass to window for create
  -- make sure lock is possible

 -- dothisnext

  self.image    =lcairo.create_image(w,h)
  self._surf    =lcairo.image_get_surface(self.image);
  self._surfdata=lcairo.image_get_surface_data(self.image);

  self._window=lcairo.create_window(w,h,title)
  lcairo.window_set_surface(self._window,self._surf)

  return self
end
function Window:__gc()
 print("window gc")
end

function Window:refresh()
  lcairo.window_refresh( self._window ) 
end
function Window:valid()
  return lcairo.window_valid( self._window ) 
end
function Window:messages_next()
  return lcairo.window_messages_next( self._window ) 
end
function Window:resize(w,h)
  self.image    =lcairo.create_image(w,h)
  self._surf    =lcairo.image_get_surface(self.image);
  self._surfdata=lcairo.image_get_surface_data(self.image);
  lcairo.window_set_surface(self._window,self._surf)
end
