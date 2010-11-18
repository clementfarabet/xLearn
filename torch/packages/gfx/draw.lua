
------------------------------------- Extend gfx with basic draw functions

function gfx.setPenColor(self,col)
	self:set('penColor',col);
end

function gfx.setMarkerSize(self,sz)
	self:set('markerSize',sz);
end

function gfx.setLineWidth(self,width)
	self:set('lineWidth',width);
end

function gfx.setLineStyle(self,type)
	self:set('lineStyle',type);
end
 
gfx.primitive={}
 
function gfx.primitive.line(self,x1,y1,x2,y2,no_dash)
	local cr=self.windowHandle.cairoHandle;
	if self:get('lineStyle')==':' and no_dash==nil then
		cr:set_dash(4,0);
	end
	cr:move_to(x1,y1)
	cr:line_to(x2,y2)
	cr:stroke();
	if self:get('lineStyle')==':' then
		cr:set_not_dash();
	end
end

function gfx.primitive.dot(self,x,y)
	local cr=self.windowHandle.cairoHandle;
	local w=self:get('markerSize');
	cr:move_to(x,y)
	cr:rectangle(x-w/2,y-w/2,w,w)
	cr:fill()
end

function gfx.primitive.circle(self,x,y)
	local cr=self.windowHandle.cairoHandle;
	local w=self:get('markerSize')*2;
	cr:move_to(x+w,y)
	cr:arc(x,y,w,0,3.14159*2.0)
	cr:stroke()
end


function gfx.primitive.cross(self,x,y)
	local cr=self.windowHandle.cairoHandle;
	local w=self:get('markerSize')*2;
	self.primitive.line(self,x-w,y-w,x+w,y+w);
	self.primitive.line(self,x+w,y-w,x-w,y+w);
end

function gfx.primitive.plus(self,x,y)
	local cr=self.windowHandle.cairoHandle;
	local w=self:get('markerSize')*4;
	self.primitive.line(self,x-w,y,x+w,y);
	self.primitive.line(self,x,y-w,x,y+w);
end

function gfx.primitive.minus(self,x,y)
	local cr=self.windowHandle.cairoHandle;
	local w=self:get('markerSize')*4;
	self.primitive.line(self,x-w,y,x+w,y); 
end

function gfx.primitive.star(self,x,y) 
	local cr=self.windowHandle.cairoHandle;
	local w=self:get('markerSize')*2;
	self.primitive.line(self,x-w,y-w,x+w,y+w,true);
	self.primitive.line(self,x+w,y-w,x-w,y+w,true);
	local w=self:get('markerSize')*3;
	self.primitive.line(self,x-w,y,x+w,y,true);
	self.primitive.line(self,x,y-w,x,y+w,true);
end

function gfx.primitive.square(self,x,y)
	local cr=self.windowHandle.cairoHandle;
	local w=self:get('markerSize')*3;
	cr:rectangle(x-w/2,y-w/2,w,w) 
	cr:stroke()
end

function gfx.primitive.diamond(self,x,y)
	local cr=self.windowHandle.cairoHandle;
	local w=self:get('markerSize')*2;
	self.primitive.line(self,x-w,y,x,y-w); 
	self.primitive.line(self,x-w,y,x,y+w);
	self.primitive.line(self,x+w,y,x,y-w);
	self.primitive.line(self,x+w,y,x,y+w);	
end


function gfx.primitive.rectangle(self,x0,y0,x1,y1)
	local cr=self.windowHandle.cairoHandle;
	--cr:set_line_width(self:get('lineWidth'));
	cr:rectangle(x0,y0,x1-x0,y1-y0)  
	cr:stroke()
end

function gfx.primitive.text(self,str,x,y,height)
	local cr=self.windowHandle.cairoHandle;
	if str==nil then error('No string given');end;
	if height==nil then height=self:get('fontSize'); end;
	cr:set_font_size(height)
	cr:move_to(x,y)
	cr:show_text(str)
end

function gfx.primitive.text_plot(self,x,y)
	self.primitive.text(self,self.globals.markerText,x,y,self:get('markerSize')*10)
end



function gfx.primitive.blit(self,x,scale,xpos,ypos)

-- we have to create our own cairo here because the surface is only temporary
-- or else we get hit at the end on nested ends 
-- (when cairo might still want to be drawing)

	local cr=self:begin()
 		 
	if self.lcairo_object == self.lcairo_write_object then
           -- not saving PNG,PDF or PS, create a new special object
           cr=self.windowHandle.lcairo_write_object:begin(); 
	end  
	cr:save();
	cr:translate(self:get('startX'),self:get('startY'))
        
	local image = graphics.newimage(x)
  	if xpos~=nil then cr:translate(xpos,ypos); end;
	if scale~=nil then cr:scale(scale,scale); end;
	cr:set_source_surface(image,0,0)
	cr:rectangle(0,0,x:size(1),x:size(2))
	cr:fill()
	cr:restore();
	cr=nil 
	if self.lcairo_object == self.lcairo_write_object then
           self.windowHandle.lcairo_write_object:theEnd();
	end  
	collectgarbage()
	self:theEnd()
end


function gfx.toTensor(self,x)
	self.lcairo_write_object:to_tensor(x);
end 

-- define user functions for each of the primitive drawing commands..
for i,k in pairs(gfx.primitive) do
	local f= function (self,...) 
		local figure=self:getFigure();
		local a={...}; a.n=nil; 
		local handle=figure:newPrimitiveObject(self.primitive[i],unpack(a));
		handle.className=i;
		figure:addObject(handle);	
		handle:draw();
		return handle;
	end
	gfx[i]=f;
end



function gfx.draw(self)
	-- redraw all previous drawing functions..
	local tmp=self.recordHistory;
	self.recordHistory=false;
 	self:cls(true);      -- keep history
	self:drawObjects(); -- draw underlying objects
	self.recordHistory=tmp;
end

gfx.redraw=gfx.draw; 


function gfx.drawObjects(self)
	self.windowHandle:begin();
	local h=self.objects;
	for i=1,#h do
		h[i]:draw();
	end
	self.windowHandle:theEnd();
end

