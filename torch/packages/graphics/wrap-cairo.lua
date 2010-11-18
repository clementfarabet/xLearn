

--------------------------------------
-------------- LCAIRO ----------------
--------------------------------------

require "torch"
require "lcairo"




--------------------------------------
----------- MODULE GRAPHICS ----------
--------------------------------------


local G = _G
local ipairs = ipairs
local lcairo = lcairo
local pairs = pairs
local setmetatable = setmetatable
local string = string
local table = table
local torch = torch
local type = type

module('graphics')




-- helper

local function declareRelayFunctions(toclass,fromclass)
   for name,func in pairs(fromclass) do
      if (type(func) == 'function' and not string.match(name, "^_"))
      then
         toclass[name] = function(self,...) 
                            return self.object[name](self.object,...) 
                         end
      end
   end
end

-- windows

windowList = {}
windowClass = {}
windowClass.__index = windowClass
declareRelayFunctions(windowClass, torch.getmetatable('lcairo.Surface'))
declareRelayFunctions(windowClass, torch.getmetatable('lcairo.PixelSurface'))
declareRelayFunctions(windowClass, torch.getmetatable('lcairo.Window'))
windowClass.resize = nil


function readline_callback()
   for i,window in ipairs(windowList) do
      local w,h;
      while true do
         local msg = window.object:messages_next()
         local id = string.match(msg,"(%S+)")
         if id=="null" then 
            break 
         end
         if id=="resize" then
            id,w,h=string.match(msg,"(%S+)%s+(%S+)%s+(%S+)")
         end
      end
      if (w and h) then
         window.width = w
         window.height = h
         window.object:resize(w,h)
         if (type(window.resizeCallback) == 'function') then
            window.resizeCallback(w,h)
         end
      end
   end
end

function windowClass:onResize(f)
   if (f == nil) then
      self.resizeCallback = nil;
   else
      self.resizeCallback = f;
   end
end

function windowClass:valid()
   if (self.object:valid() == 1) then return 1 end
   return false
end

function windowClass:begin()
   return lcairo.Cairo(self.object)
end

function windowClass:theEnd()
   self.object:refresh()
end

function windowClass:close()
   for i=#windowList,1,-1 do
      if (windowList[i] == self) then
         table.remove(windowList, i)
      end
   end
   G.collectgarbage()
end

function windowClass:refresh()
   self.object:refresh()
end

function newwindow(w,h,title)
   local self = {}
   setmetatable(self, windowClass)
   self.width = w;
   self.height = h;
   self.object = lcairo.Window(w,h,title)
   self._surf = self.object._surf
   windowList[#windowList+1] = self
   if (G.readline_callback == nil) then
      G.readline_callback=readline_callback
   end
   return self
end



-- images

imageClass = {}
imageClass.__index = imageClass
declareRelayFunctions(imageClass, torch.getmetatable('lcairo.Surface'))
declareRelayFunctions(imageClass, torch.getmetatable('lcairo.PixelSurface'))
declareRelayFunctions(imageClass, torch.getmetatable('lcairo.Image'))

function imageClass:begin()
   return lcairo.Cairo(self.object)
end

function imageClass:theEnd()
end

function imageClass:refresh()
end

function newimage(...)
   local self = {}
   setmetatable(self, imageClass)
   local firstarg = ...
   if (torch.typename(firstarg) == "torch.Tensor") then
      self.object = lcairo.Image(firstarg:size(1), firstarg:size(2))
      self.object:fromTensor(firstarg)
   else
      self.object = lcairo.Image(...)
   end
   self.width = self.object.width;
   self.height = self.object.height;
   self._surf = self.object._surf
   return self
end


-- pdf

pdfClass = {}
pdfClass.__index = pdfClass
declareRelayFunctions(pdfClass, torch.getmetatable('lcairo.Surface'))
declareRelayFunctions(pdfClass, torch.getmetatable('lcairo.PDF'))

function pdfClass:begin()
   return lcairo.Cairo(self.object)
end

function pdfClass:theEnd()
end

function pdfClass:refresh()
end

function newpdf(w,h,fname)
   local self = {}
   setmetatable(self, pdfClass)
   self.object = lcairo.PDF(fname,w,h)
   self.width = self.object.width or w;
   self.height = self.object.height or h;
   self._surf = self.object._surf
   return self
end


-- ps

psClass = {}
psClass.__index = psClass
declareRelayFunctions(psClass, torch.getmetatable('lcairo.Surface'))
declareRelayFunctions(psClass, torch.getmetatable('lcairo.PS'))

function psClass:begin()
   return lcairo.Cairo(self.object)
end

function psClass:theEnd()
end

function psClass:refresh()
end

function newps(w,h,fname)
   local self = {}
   setmetatable(self, psClass)
   self.object = lcairo.PS(fname,w,h)
   self.width = self.object.width or w;
   self.height = self.object.height or h;
   self._surf = self.object._surf
   return self
end


