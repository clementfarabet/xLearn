
------------------------------------------ Define subplot class..
 
local subplot={}

function subplot.new(self)
	local obj=self.windowHandle:newObject();
	obj.className="subplot"
	obj.draw = subplot.draw;
	obj.redraw = subplot.draw;
	obj.beginScaling=subplot.beginScaling;
	obj.endScaling=subplot.endScaling;
	return obj;
end
 

function subplot.draw(self)
	self:beginScaling();
	self:cls(true);
	self:drawObjects();
	self:endScaling(); 
end
 

function subplot.beginScaling(self)

	local cr=self.windowHandle:begin(); 

--	cr:translate(self:get('startX'),self:get('startY'))
--	cr:scale((self:get('width'))/(self.parent:get('width')),
--		 (self:get('height'))/(self.parent:get('height')))

 	-- clipping ...
	cr:save();
	cr:rectangle(self:get('startX'),self:get('startY'),
		    self:get('width'),self:get('height'))
	cr:clip() 
end

function subplot.endScaling(self)
	local cr=self.windowHandle.cairoHandle; 
	cr:reset_clip() 
	cr:restore();
	cr=self.windowHandle:theEnd();	
end


------------------------------------------ Extend gfx..

function gfx.subplot(self,rows,cols,index)


	-- one argument => index an already existing subplot..
        if cols==nil then index=rows; rows=#self.objects; cols=1; end	


        if #self.objects~=rows*cols then -- already created, return handle only
	 self:cls();

	 local width=self:get('width')/rows;
	 local height=self:get('height')/cols;

	 local windowHandle;
	 for j=1,cols do
	 for i=1,rows do
		windowHandle=subplot.new(self);	-- create new subplot
		windowHandle:setg('startX',width*(i-1))
		windowHandle:setg('startY',height*(j-1))
		windowHandle:setg('width',width)
		windowHandle:setg('height',height)
		windowHandle:setg('currentAxesHandle',false) 
		self:addObject(windowHandle);  	-- add subplot to window
	 end
	 end 
	 self:draw();
	end

	if index==nil then index=1; end

	local figure=self.objects[index];
	figure:setg('currentFigureHandle',figure);
	self:setg('currentFigureHandle',figure);
	self.windowHandle:setg('currentFigureHandle',figure);
	self.windowHandle:setg('currentAxesHandle',figure:get('currentAxesHandle'))

	-- figure:setg('currentAxesHandle',figure:get('currentAxesHandle'))

	--local axesHandle=figure:getAxes(); 
	--self.windowHandle:setg('currentAxesHandle',axesHandle)


	
	return figure;
end


function gfx.getFigure(self)
	local handle= self:get('currentFigureHandle'); 
	if handle==nil then handle=self.windowHandle; end
	return handle;
end





