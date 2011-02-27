require("torch")
require("graphics")

-- helper function....
function table.print(t)
  if type(t)=="table" then
	for i,k in pairs(t) do
		print(i,k);
	end
  else
	print(t)
  end
end

gfx={}

-- load local drawing packages..

dofile(torch.packageLuaPath('gfx') .. '/io.lua')
dofile(torch.packageLuaPath('gfx') .. '/draw.lua')
dofile(torch.packageLuaPath('gfx') .. '/axes.lua')
dofile(torch.packageLuaPath('gfx') .. '/legend.lua')
dofile(torch.packageLuaPath('gfx') .. '/subplot.lua')
dofile(torch.packageLuaPath('gfx') .. '/plot.lua')
dofile(torch.packageLuaPath('gfx') .. '/hist.lua')



-------------------------------------------------


local function gfx_window_index(table,key)
	return gfx[key];
end


function gfx.default_params(self)
 local w=self.globals.width;
 local h=self.globals.height;
 local x=self.globals.startX;
 local y=self.globals.startY;

 -- if the currentAxes belongs to this object then delete links in the main window
 if self.globals.currentAxesHandle==self.windowHandle.globals.currentAxesHandle then
	self.windowHandle.globals.currentAxesHandle=nil;
 end
 self.globals.currentAxesHandle=nil;

 self.globals={};
 self.globals.width=w;
 self.globals.height=h;
 self.globals.startX=x;
 self.globals.startY=y;
 self.globals.currentFigureHandle=nil;  -- handle on last plotting child object 

end 


local function default_globals()
 local globals={};
 globals.backgroundColor={.85,.85,.85};
 globals.paperColor={1,1,1};
 globals.penColor= {0,0,0};
 globals.fontSize= 15;
 globals.lineWidth= 2;
 globals.markerStyle=nil; 
 globals.markerText='';
 globals.markerSize=2;
 globals.lineStyle=nil;  
 globals.startX=0;
 globals.startY=0;
 globals.width=500;
 globals.height=400;
 globals.borderWidth=50
 globals.borderHeight=40
 globals.borderLineWidth=4
 return globals;
end

function gfx.Window(x,y,str)

 local w={};
 w.className='window'
 w.globalDefaults=default_globals(); 
 w.globals={};
 local m={}; setmetatable(w,m);
 m.__index = gfx_window_index;

 -- check parameters --..
 w:setg('width',w.globalDefaults['width']);
 w:setg('height',w.globalDefaults['height']);
 w:setg('startX',w.globalDefaults['startX']);
 w:setg('startY',w.globalDefaults['startY']);
 if x~=nil then w:setg('width',x);  end;
 if y~=nil then w:setg('height',y); end;
 if str==nil then str="Torch"; end;
 
 w.lcairo_object=graphics.newwindow(w:get('width'),w:get('height'),str);
 w.lcairo_object:onResize(function(nw,nh) 
                             w:setPosition(0,0,math.max(200,nw),math.max(200,nh),false)
                             w:redraw() end)
 w.lcairo_write_object=w.lcairo_object;
 w.objects={}; 
 w.cairoHandle=nil;
 w.cairoLevel=0;
 w.batchLevel=0;
 w.windowHandle=w;
 w:cls(true);
 return w;
end
gfx.New=  gfx.Window; 


function gfx.ImageSurface(x,y,str)

 local w={};
 w.className='imageSurface'
 w.globalDefaults=default_globals(); 
 local m={}; setmetatable(w,m);
 m.__index = gfx_window_index;

 -- check parameters --..
 w:setg('width',w.globalDefaults['width']);
 w:setg('height',w.globalDefaults['height']);
 w:setg('startX',w.globalDefaults['startX']);
 w:setg('startY',w.globalDefaults['startY']);
 if x~=nil then w:setg('width',x);  end;
 if y~=nil then w:setg('height',y); end;

 w.lcairo_object=graphics.newimage(w:get('width'),w:get('height'));
 w.lcairo_write_object=w.lcairo_object;
 w.objects={}; 
 w.cairoHandle=nil;
 w.cairoLevel=0;
 w.batchLevel=0;
 w.windowHandle=w;
 w:cls(true);
 return w;
end


function gfx.begin(self) 
   local cr = self.windowHandle.cairoHandle;
   if cr == nil then
      self.windowHandle.cairoLevel=0;
      cr = self.windowHandle.lcairo_write_object:begin();  
      self.windowHandle.cairoHandle = cr;
   else 
      self.windowHandle.cairoLevel = self.windowHandle.cairoLevel+1; 
   end
   cr:set_source_rgb(unpack(self:get('penColor')))
   cr:set_line_width(self:get('lineWidth'))
   return cr;
end

function gfx.theEnd(self)
   if self.windowHandle.cairoLevel == 0 then
      self.windowHandle.cairoHandle=nil;
      self.windowHandle.lcairo_write_object:theEnd()
   else
      self.windowHandle.cairoLevel=self.windowHandle.cairoLevel-1;
   end
end 

-- iain: to deprecate
function gfx.batchBegin(self)
 if self.batchLevel==0 then gfx.begin(self); end
 self.batchLevel=1;
end
function gfx.batchEnd(self)
 if self.batchLevel==1 then gfx.theEnd(self); end
 self.batchLevel=0;
end
function gfx.valid(self)
 return self.lcairo_object:valid()
end


function gfx.cls(self, keep_history)

 local cr=self:begin(); 
 cr:set_source_rgb(unpack(self:get('backgroundColor')));
 cr:rectangle(self:get('startX'),self:get('startY'),
	      self:get('width'), self:get('height'))
 cr:fill()
 cr:set_source_rgb(unpack(self:get('penColor')))
 if self.cairoLevel==self.batchLevel and keep_history==nil then
	self.objects={}; -- clear history
	self:default_params();
 end
 self:theEnd();
end


function gfx.set(self,variableName,value)

	if variableName==nil then -- print possible variables instead
		table.print(self.globals)
		return
	end

	self:setg(variableName,value)

	local f={};
	function f.recursiveset(s,name,val) 
		local o=s.objects;
		if s==nil or o==nil then return; end
		for i=1,#o do
			if o[i].globals~=nil and o[i].globals[name]~=nil 
		   	and o[i].globals[name]~=val then
				o[i].globals[name]=val;
			end
			f.recursiveset(o[i],name,val)
		end
	end
	f.recursiveset(self,variableName,value)

	--self:redraw(); -- redraw after set
end 

function gfx.setPosition(self,x,y,w,h,redraw,cnt)
	if redraw==nil then redraw=true; end
	if cnt==nil then cnt=0; end
	local oldx=self:get('startX');
	local oldy=self:get('startY');
	local oldw=self:get('width');
	local oldh=self:get('height');
	local scx= w/oldw;
	local scy= h/oldh;
	local trx= x-oldx;
	local try= y-oldy;
	if self.globals~=nil then
	 if self.globals.startX~=nil then self:setg('startX',x) end	
	 if self.globals.startY~=nil then self:setg('startY',y) end	
	 if self.globals.width~=nil  then self:setg('width',w) end	
	 if self.globals.height~=nil then self:setg('height',h) end	
	end
	local o=self.objects;
	if o==nil then return; end
	for i=1,#o do
		local ox=o[i]:get('startX')
		local oy=o[i]:get('startY')
		local ow=o[i]:get('width')
		local oh=o[i]:get('height')
		o[i]:setPosition((ox+trx)*scx,(oy+try)*scy,(ow+trx)*scx,(oh+try)*scy,
			redraw,cnt+1)
	end
	if cnt==0 and redraw then
		if self.parent~=nil then
			self.parent:redraw();
		else
			self:redraw();
		end
	end
end 

function gfx.setg(self,variableName,value)
	if self.globals==nil then self.globals={}; end
	self.globals[variableName]=value;
end 
  
function gfx.get(self,variableName)
	if self.globals~=nil and self.globals[variableName] ~= nil then
		return self.globals[variableName];
	else
		if self.parent ~= nil then 
			return self.parent:get(variableName,value)
		else
			return self.globalDefaults[variableName];
		end
	end
end


function gfx.remove(self)
	for i=1,#self.parent.objects do 
		if self==self.parent.objects[i] then
			table.remove(self.parent.objects,i);
		end
	end
	-- clean up current handles..
	if self.windowHandle.globals.currentAxesHandle==self then 
		self.windowHandle.globals.currentAxesHandle=nil; 
	end
	if self.windowHandle.globals.currentFigureHandle==self then 
		self.windowHandle.globals.currentFigureHandle=nil; 
	end
	if self.parent.globals.currentAxesHandle==self then 
		self.parent.globals.currentAxesHandle=nil; 
	end
	if self.parent.globals.currentFigureHandle==self then 
		self.parent.globals.currentFigureHandle=nil; 
	end
	self.parent:redraw();
end

function gfx.addObject(self,object)
	local h=self.objects; 
	h[#h+1]=object;
	return h[#h]; -- return handle to table
end

function gfx.newObject(self)
	local obj={}
	obj.objects={}; 
	obj.parent=self;
	obj.windowHandle=self.windowHandle;
	obj.globals={}
	local m={}; setmetatable(obj,m);
 	m.__index = gfx_window_index;
	return obj; 
end


function gfx.newPrimitiveObject(self,fn,...)
	local obj=self.newObject(self);
        local arg = {...}
	arg.n=nil;
	function obj.draw(slf)
		local f=fn;   
		local p=arg;	
		local cr=slf:begin()
		cr:save();
		cr:translate(self:get('startX'),self:get('startY'))
		f(slf,unpack(p));
		cr:restore()
		slf:theEnd()
	end;
	function obj.redraw(slf)
		slf.parent:redraw();
	end
	return obj;
end

	
