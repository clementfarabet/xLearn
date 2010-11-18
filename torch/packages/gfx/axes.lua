
local axes={}

function axes.new(self)
	local obj=self:newObject(); 
	obj.className="axes"
	obj.cls=axes.cls;
	obj.draw=axes.draw; 
	obj.redraw=axes.draw; 
	obj.getAxes=axes.getAxes;
	obj.drawAxes=axes.drawAxes;
	obj.recalcAxes=axes.recalcAxes;
	obj.calcTickMarks= axes.calcTickMarks;	
	obj.drawTickMarks=axes.drawTickMarks;
	obj.drawTitle=axes.drawTitle;
	obj.beginAxesScaling=axes.beginAxesScaling;
	obj.endAxesScaling=axes.endAxesScaling;
	obj.beginAxesClip=axes.beginAxesClip;
	obj.endAxesClip=axes.endAxesClip;

	obj:setg('axes',nil);
	obj:setg('axesStyle',"on"); 
	obj:setg('axesAutoRecalc',{true,true,true,true});
 	obj:setg('xlabel',nil)
	obj:setg('ylabel',nil);

	return obj
end

function axes.getAxes(self)
	return self;
end

function axes.cls(self,keep_history,type)

	local cr=self.windowHandle:begin();

	if self.cairoLevel==self.batchLevel and keep_history==nil then
		self.objects={}; -- clear history
		self:setg('axes',nil);
		self:setg('axesStyle',"on"); 
		self:setg('axesAutoRecalc',{true,true,true,true});
 		self:setg('xlabel',nil)
		self:setg('ylabel',nil);
		self:setg('legend',nil);
		self:redraw();
	 end
	
	local w=self:get('width');
	local h=self:get('height');
	local bw=self:get('borderWidth');
	local bh=self:get('borderHeight');

	if type=="outside" then
		self:beginAxesScaling("outside"); 
		cr:set_source_rgb(unpack(self:get('backgroundColor')));
		cr:rectangle(0,0,w,bh); cr:fill(); 
		cr:rectangle(0,h-bh,w,bh); cr:fill();
		cr:rectangle(0,0,bw,h); cr:fill(); 
		cr:rectangle(w-bw,0,bw,h); cr:fill();
	        self:endAxesScaling("outside");
	end

	if type=="inside" then 
		self:beginAxesScaling("inside");
		cr:set_source_rgb(unpack(self:get('paperColor')));
		cr:rectangle(0,0,w,h)
 		cr:fill()
		self:endAxesScaling("inside");
	end

	cr=self.windowHandle:theEnd();
end	


function axes.draw(self,type)
	self:begin();
	if type~="inside" then   -- render outside axes..
	   self:cls(true,"outside")
	   self:drawAxes("outside");
	end
	if type~="outside" then  -- render inside axes..
	   self:cls(true,"inside")
	   self:beginAxesScaling("inside");
	   self:beginAxesClip();
	   self:drawObjects(self);
	   self:endAxesClip();
	   self:endAxesScaling("inside");
	   self:drawAxes("inside");
	end
	self:theEnd()
end

function axes.beginAxesClip(self)
	-- clipping ...
	local cr=self.windowHandle.cairoHandle;
	cr:save()
	cr:rectangle(0,0,self:get('width'), self:get('height'))
	cr:clip() 
end

function axes.endAxesClip(self)
	local cr=self.windowHandle.cairoHandle;
	cr:reset_clip() 
	cr:restore()
end

function axes.recalcAxes(self,x,y)

	local redraw=true;
	local r=self:get('axesAutoRecalc');

	if r[1]==true or r[2]==true or r[3]==true or r[4]==true then 
	-- define axes
		if self:get('axes')==nil then
			self:setg('axes',{x:min(),x:max(),y:min(),y:max()});
			if x:var()<1e-15 or y:var()<1e-15 then
			  self:setg('axes',{x:min()-1,x:max()+1,
					    y:min()-1,y:max()+1});
			end
		else
			redraw=false;
			local t={x:min(),x:max(),y:min(),y:max()};
			local axes=self:get('axes')
			
			if r[1]==true and t[1]<axes[1] then 
				axes[1]=math.min(t[1],axes[1]); redraw=true; 
			end
			if r[2]==true and t[2]>axes[2] then 
				axes[2]=math.max(t[2],axes[2]); redraw=true; 
			end
			if r[3]==true and t[3]<axes[3] then 
				axes[3]=math.min(t[3],axes[3]); redraw=true; 
			end
			if r[4]==true and t[4]>axes[4] then 
				axes[4]=math.max(t[4],axes[4]); redraw=true; 
			end
		
			self:setg('axes',axes);
		end
	end

	return redraw;
end


function axes.beginAxesScaling(self,type)
	local cr=self.windowHandle:begin(); 
	cr:save();

	local w=self:get('borderWidth');
	local h=self:get('borderHeight');

	if type~="outside" then
 	 cr:translate(self:get('startX')+w,
		    self:get('startY')+h)
	 cr:scale((self:get('width')-w*2)/(self:get('width')),
		 (self:get('height')-h*2)/(self:get('height')))
	else
 	 cr:translate(self:get('startX'),
		    self:get('startY'));
	end
end


function axes.endAxesScaling(self,type)
	local cr=self.windowHandle.cairoHandle; 
	cr:restore();
	cr=self.windowHandle:theEnd();	
end


function axes.calcTickMarks(self,x1,x2)

	local szx= math.abs(x2-x1)

	local function calc_tic(multiple,size_scale)
	 local tic_gap=10^math.floor(math.log10(size_scale))/100; 
	 local sc=1;
	 while tic_gap<size_scale/10 do
		tic_gap=tic_gap*multiple; sc=sc*multiple;
	 end
	 if tic_gap<1e-12 then tic_gap=1e-12; end -- stop being equal to 0
 	 return tic_gap,sc
	end
 
	local tic_gap, sc;
	tic_gap,sc=calc_tic(10,szx);
	if  tic_gap*2>szx then -- do it all again with multiples of 5..
		tic_gap,sc=calc_tic(5,szx);
	end

	sc= 10^(math.floor(math.log10(sc)));
	local ticstart= math.ceil(x1*sc)/sc;
	ticstart= ticstart-math.floor((ticstart-x1)/tic_gap)*tic_gap; 
			-- go back a bit if necess.
	if x1<=0 and x2>=0 then
	  local t= math.floor(-x1/tic_gap);
	  ticstart= -t*tic_gap;
	end
	return ticstart,x2,tic_gap;
end


function axes.drawTickMarks(self,type)

	local cr=self.windowHandle.cairoHandle;
	local axes=self:get('axes')
	if axes==nil then axes={0,1,0,1}; end
	local width=(axes[2]-axes[1]);
	local height=(axes[4]-axes[3]);
	local xscale=self:get('width')/width;
	local yscale=self:get('height')/height;
	local szy= math.abs(axes[4]-axes[3])
	local tic_start,tic_end,tic_gap;

	tic_start,tic_end,tic_gap=self:calcTickMarks(axes[1],axes[2])

	local fontsz=self:get('fontSize');
	cr:set_font_size(fontsz)
	cr:set_line_width(self:get('borderLineWidth')/2);

	for i=tic_start,tic_end,tic_gap do
		local v=i;
		local x=(v-axes[1])*xscale;
		
		if v>=axes[1] then
		 if type~="outside" then
			 self.primitive.line(self,x,self:get('height'),
						  x,self:get('height')-8);
		 end
		 if type~="inside" then
		   	if math.abs(v)<1e-15 then v=0; end;
		   	local str=string.format('%0.4g',v);
			self.primitive.text(self,
					    str,x-cr:get_text_width(str)/2,
					    self:get('height')+1+
					    cr:get_text_height(str),fontsz);
		 end
		end
	end

	tic_start,tic_end,tic_gap=self:calcTickMarks(axes[3],axes[4])

	for i=tic_start,tic_end,tic_gap do
		local v=i; 
	 	local y=(v-axes[3])*yscale;

		if v>=axes[3] then
 		 if type~="outside" then

		 	self.primitive.line(self,
				0,self:get('height')-y,8,self:get('height')-y);
		 end
		 if type~="inside" then
		 	if math.abs(v)<1e-15 then v=0; end; 
		 	local str=string.format('%0.4g',v);
		 	self.primitive.text(self,str,-cr:get_text_width(str)-1,
						self:get('height')-y
						+cr:get_text_height(str)/2,
						fontsz);
		 end
		end
	end
end


function axes.drawTitle(self)   -- draw title + xlabel/ylabel on window
 local cr=self.windowHandle.cairoHandle;
 --cr:save();
 --cr:translate(self:get('startX'),self:get('startY'));

 local str=self:get('titleString');
 if str~=nil then
  cr:set_font_size(self:get('fontSize'))
  local szx=cr:get_text_width(str);
  local szy=cr:get_text_height(str);
  cr:save();
  cr:move_to(self:get('width')/2-szx/2,math.max(self:get('borderHeight')*0.7,szy*1.2))
  cr:set_source_rgb(unpack(self:get('penColor')))
  cr:show_text(str)
  cr:restore();
 end
 if self:get('axesStyle')~="off" then
  local str= self:get('xlabel')
  if str~=nil then  
   cr:set_font_size(self:get('fontSize'))
   local szx=cr:get_text_width(str);
   local szy=cr:get_text_height(str);
   cr:save();
   cr:move_to(self:get('width')/2-szx/2,self:get('height')-0.8*self:get('borderHeight')+2*szy)
   cr:set_source_rgb(unpack(self:get('penColor')))
   cr:show_text(str)
   cr:restore();
  end
  local str=self:get('ylabel');
  if str~=nil then  
   cr:set_font_size(self:get('fontSize'))
   local szx=cr:get_text_width(str);
   local szy=cr:get_text_height(str);
   cr:save();
   cr:move_to(math.max(szy+1,self:get('borderWidth')*0.5-szy), self:get('height')/2+szx/2)
   cr:rotate(-math.pi/2);
   cr:set_source_rgb(unpack(self:get('penColor')))
   cr:show_text(str)
   cr:restore();
  end
 end

 --cr:restore();
end



function axes.drawAxes(self,type)

	-- draw axes --..
	-- and set clipping boundaries --..
	
	local cr=self.windowHandle:begin();
	
	local w=self:get('width');
	local h=self:get('height');
	local bw=self:get('borderWidth');
	local bh=self:get('borderHeight');

	if type=="outside" then
		self:beginAxesScaling("outside");
		self:drawTitle()
		self:endAxesScaling("outside");
		self:beginAxesScaling("inside");
		cr:set_source_rgb(unpack(self:get('penColor')))
		if self:get('axesStyle')~="off" then
		 	self:drawTickMarks("outside")
		end -- end draw of axes
		self:endAxesScaling("inside");
	end

	if type=="inside" then
		self:beginAxesScaling("inside");
		self:beginAxesClip();
		cr:set_source_rgb(unpack(self:get('penColor')))
		if self:get('axesStyle')~="off" then
		 	self:drawTickMarks("inside")
		end -- end draw of axes
		cr:set_line_width(self:get('borderLineWidth'));
		cr:rectangle(0,0,w,h); cr:stroke()
		self:endAxesClip();
		self:endAxesScaling("inside");
	end 

	if type=="inside" then
		local legend=self:get('legend');
		if legend~=nil then legend:draw(); end
	end

	cr=self.windowHandle:theEnd();
end



------------------------------------------ Extend gfx..


function gfx.axis(self,x1,x2,y1,y2)

	local axes=self:getAxes();
	if(x1=='off') then
		axes:setg('axesStyle',"off")
		axes:redraw(); 
		return axes
	end
	if(x1=='on') then
		axes:setg('axesStyle',"on")
		axes:redraw();
		return axes
	end

	if x1==nil and x2==nil and y1==nil and y2==nil then 
		axes:redraw(); return axes;
 	end

	-- can supply some of the values as nil, they do not change..
	local a=axes:get('axes')
	local r=axes:get('axesAutoRecalc')
	if a==nil then 	a={0,1,0,1}; end
	if x1==nil then x1=a[1]; else r[1]=false; end
	if x2==nil then x2=a[2]; else r[2]=false; end
	if y1==nil then y1=a[3]; else r[3]=false; end
	if y2==nil then y2=a[4]; else r[4]=false; end

	if x2>x1 and y2>y1 then
		axes:setg('axes',{x1,x2,y1,y2});
		axes:setg('axesAutoRecalc',r); 
		axes:redraw(); 
	else
		error("gfx: axes not correctly specified")
	end
	return axes;
end 

function gfx.getAxes(self)

	local axesHandle;

	if self:get('currentAxesHandle')==nil or
	   self:get('currentAxesHandle')==false
						then -- if no plot obj yet
		local object=self:get('currentFigureHandle')
		if object==nil then object=self; end
		axesHandle=axes.new(object); 		-- create new axes
		object:addObject(axesHandle); 		-- add axes to window
		object:setg('currentFigureHandle',self);-- set as current obj
		object:setg('currentAxesHandle',axesHandle) 
		self:setg('currentFigureHandle',self);
		self:setg('currentAxesHandle',axesHandle)
		object.windowHandle:setg('currentAxesHandle',axesHandle)
		object.windowHandle:setg('currentFigureHandle',object)
	else
		axesHandle=self:get('currentAxesHandle')
	end
	return axesHandle;
end


function gfx.title(self,str)
	local axes=self:getAxes();
	axes:setg('titleString',str)
	axes:redraw("outside");
end

function gfx.xlabel(self,str)
	local axes=self:getAxes();
	axes:setg('xlabel',str);
	axes:redraw("outside");
end


function gfx.ylabel(self,str)
	local axes=self:getAxes();
	axes:setg('ylabel',str);
	axes:redraw("outside");
end



