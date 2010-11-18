------------------------------------------- Define legend object

local legend={}

function legend.new(self)
	local obj=gfx.newObject(self)
	obj.className='legend';
	obj.draw=legend.draw;
	obj.redraw=legend.redraw;
	obj:setg('position',2);  -- default position 'top-left'
	return obj;
end

function legend.draw(self)

	local titles=self:get('titles')
	if titles==nil then
		return; 
	end;

			
	local styles={}; -- get line styles from axes child objects
	for i=1,#self.parent.objects do
		local obj=self.parent.objects[i];
		if obj.getLegendStyle~=nil then
			local t=obj:getLegendStyle();
			for j=1,#t do styles[#styles+1]=t[j]; end
		end
	end		
	local items=math.min(#styles,#titles);

	if #styles==0 then return; end

	local cr=self:begin(); 
	cr:set_font_size(self:get('fontSize'))
	cr:set_line_width(self:get('borderLineWidth')/2);

	local maxLength=0;
	local maxHeight=0;

	for i=1,items do
		local title=titles[i];
		if title ~=nil then
  		 local szx=cr:get_text_width(title)
  	 	 local szy=cr:get_text_height(title)
		 if szx>maxLength then maxLength=szx; end;
		 if szy>maxHeight then maxHeight=szy; end;
		end
	end


	local startx=self:get('startX'); 
	local starty=self:get('startY');	
	local w=maxLength+30; 
	local h=maxHeight*(1+items*1.5);
	local sw=self:get('width')
	local sh=self:get('height')
	local bw=self:get('borderWidth')*1.25
	local bh=self:get('borderHeight')*1.25

	--positions:
	--1 = Upper right-hand corner (default)
	--2 = Upper left-hand corner
	--3 = Lower left-hand corner
	--4 = Lower right-hand corner

	local pos=self:get('position')
	if pos==1 then startx= startx+sw-bw-w; starty= starty+bh; end
	if pos==2 then startx= startx+bw;      starty= starty+bh; end
	if pos==3 then startx= startx+bw;      starty= starty+sh-bh-h; end
	if pos==4 then startx= startx+sw-bw-w; starty= starty+sh-bh-h; end

	cr:save();
	cr:reset_clip();

	if #self:get('paperColor') == 4 then
	  cr:set_source_rgba(unpack(self:get('paperColor')));
	else
	  cr:set_source_rgb(unpack(self:get('paperColor')));
	end
	cr:rectangle(startx,starty,w,h); cr:fill()
	cr:set_source_rgb(unpack(self:get('penColor')));
	cr:rectangle(startx,starty,w,h); cr:stroke()

	for i=1,items do
		cr:set_source_rgb(unpack(styles[i].penColor));
		cr:set_line_width(styles[i].lineWidth);
		local line=styles[i].lineStyle;
		if line~=nil then
		  if line==':' then cr:set_dash(4,0); end
		  self.primitive.line(self,
			  startx+5,i*maxHeight*1.5+starty,
			  startx+20,i*maxHeight*1.5+starty)
		  if line==':' then cr:set_not_dash();  end
		end
		local mark=styles[i].markerStyle;
		if mark~=nil then 
		 local f=self.primitive[mark];
		 f(self,startx+12.5,i*maxHeight*1.5+starty);
		end
		cr:set_source_rgb(unpack(self:get('penColor')))
		self.primitive.text(self, titles[i],
			  	       startx+25,i*maxHeight*1.5+starty+5,
				       self:get('fontSize'))
	end 
	cr:restore(); -- for reset clip
	self:theEnd()
end 

function legend.redraw(self)
	self.parent:redraw();
end


------------------------------------------- Extend gfx with legend

function gfx.legend(self,...)
	local redraw=true;

	local axes=self:getAxes(); -- get or create axes for this legend
	local legnd=axes:get('legend');
	if legnd==nil then
		redraw=false; -- only have to draw legend, not a redraw
		legnd=legend.new(axes);
		axes:setg('legend',legnd) -- add legend to axes
	end	


	if #arg>=2 and type(arg[1])=="number" and type(arg[2])=="number" then
		legnd:setg('startX',legnd.parent:get('startX')+arg[1]);
		table.remove(arg,1);
		legnd:setg('startY',legnd.parent:get('startY')+arg[1]);
		table.remove(arg,1);
		legnd:setg('position',0)
	else
		if #arg>0 and type(arg[1])=="number" then 
			legnd:setg('startX',legnd.parent:get('startX'));
			legnd:setg('startY',legnd.parent:get('startY'));
			legnd:setg('position',arg[1])
			table.remove(arg,1);
		end
	end 
   
	arg.n=#arg;
	if #arg>0 then
		legnd:setg('titles',arg); -- store titles
	end

	if redraw then
		self:redraw(); -- have to cls, and redraw (remove old legend)
	else
		legnd:draw(); -- only have to draw legend
	end
	return legnd;
end 


