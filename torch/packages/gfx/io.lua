------------------------------------------ Extend gfx with i/o functions

function gfx.tensorFromPNG(self,fname,depth)
	local surf=graphics.newimage(fname)
	if surf == nil then
		return nil
	end 

	if depth==nil then depth=4; end;
	if not( depth==1 or depth==3 or depth==4) then
		error('gfx: invalid depth');
	end;
	
	local x;
	if depth==1 then
		 x = torch.Tensor(surf.width,surf.height);         
			-- Black & White
	else 
                 x = torch.Tensor(surf.width,surf.height, depth);  
			-- RGB or RGBA
	end

	surf:toTensor(x)
	surf=nil;
	collectgarbage()
	return x
end

function gfx.loadPNG(fname,depth) 
	return gfx.tensorFromPNG(gfx,fname,depth);
end

function gfx.writePNG(self,fname)

-- 	Use this line below to save out image surface, rather than writing history:
--	self.lcairo_object:write_to_png(fname)

	local surf = graphics.newimage(self:get('width'),self:get('height'));
	self.windowHandle.lcairo_write_object=surf; 
 	local cr; 

	-- orientate this object at start of page for writing..
	local x,y,w,h;
	x=self:get('startX');y=self:get('startY');
	w=self:get('width');  h=self:get('height');
	self:setPosition(0,0,w,h,false)

	cr=surf:begin();
	self.windowHandle.cairoHandle=cr; 
	self.windowHandle.cairoLevel=1;
	self:draw()
	self.windowHandle.cairoHandle=nil;
        surf:theEnd();
	surf:write_to_png(fname)
	
	--test: copy image
	--local x1 = torch.Tensor(surf.width,surf.height, 4);  
	--surf:to_tensor(x1)
	--local win=gfx.New(surf.width,surf.height);
	--win:blit(x1)

	cr=nil;
	surf=nil;
	collectgarbage()
	self.windowHandle.cairoLevel=0;
	self.windowHandle.cairoHandle=nil; 
	self.windowHandle.lcairo_write_object=self.windowHandle.lcairo_object;
	self:setPosition(x,y,w,h,false) -- put position back
end




function gfx.writePDF(self,fname,xunit,yunit)
	
        local pdf=graphics.newpdf(self:get('width'),self:get('height'),fname);
	self.windowHandle.lcairo_write_object=pdf; 
 	local cr;

	local whitepaper = false; -- don't force white paper for print
	if self:get('backgroundColor')==
	   self.windowHandle.globalDefaults.backgroundColor and
	   self.globals.backgroundColor==nil then
		whitepaper=true;
		self:setg('backgroundColor',{1,1,1})
	end

	-- orientate this object at start of page for writing..
	local x,y,w,h;
	x=self:get('startX');y=self:get('startY');
	w=self:get('width');  h=self:get('height');
	self:setPosition(0,0,w,h,false)

	cr=pdf:begin();
	self.windowHandle.cairoHandle=cr; 
	self.windowHandle.cairoLevel=1;
	self:draw()
	cr:show_page();
        pdf:theEnd();
        pdf:close()

	cr=nil;
	pdf=nil;
        
	collectgarbage()
	self.windowHandle.cairoLevel=0;
	self.windowHandle.cairoHandle=nil; 
	self.windowHandle.lcairo_write_object=self.windowHandle.lcairo_object;
	if whitepaper then
		self:setg('backgroundColor',nil)
	end
	self:setPosition(x,y,w,h,false) -- put position back
end


function gfx.writePS(self,fname,xunit,yunit)
        local ps=graphics.newps(self:get('width'),self:get('height'),fname);
        self.windowHandle.lcairo_write_object=ps; 
 	local cr; 

	local whitepaper = false; -- don't force white paper for print
	if self:get('backgroundColor')==
	   self.windowHandle.globalDefaults.backgroundColor and
	   self.globals.backgroundColor==nil then
		whitepaper=true;
		self:setg('backgroundColor',{1,1,1})
	end

	-- orientate this object at start of page for writing..
	local x,y,w,h;
	x=self:get('startX');y=self:get('startY');
	w=self:get('width');  h=self:get('height');
	self:setPosition(0,0,w,h,false)

	cr=ps:begin();
	self.windowHandle.cairoHandle=cr; 
	self.windowHandle.cairoLevel=1;
	self:draw()

	self.windowHandle.cairoHandle=nil; 
	cr:show_page();
        ps:theEnd();
        ps:close()
	cr=nil;
	collectgarbage()
	ps=nil;
	collectgarbage()
		
	self.windowHandle.cairoLevel=0;
	self.windowHandle.cairoHandle=nil; 
	self.windowHandle.lcairo_write_object=self.windowHandle.lcairo_object;
	collectgarbage()

	if whitepaper then
		self:setg('backgroundColor',nil)
	end
	self:setPosition(x,y,w,h,false) -- put position back
end

