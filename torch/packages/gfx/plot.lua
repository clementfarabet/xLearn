--------------------------------------------- Define plot object

local plot={}

function plot.new(self,x,y,params)
	local obj=gfx.newObject(self);
	obj.className="plot"
	obj.setParams=plot.setParams;
	obj.redraw=plot.redraw;
	obj.getLegendStyle=plot.getLegendStyle;
	local args={x,y}
	function obj.draw(slf)
		local f=plot.draw;   
		local a=args;	
		f(slf,unpack(a));
	end;
	if params==nil then params='b-x'; end; -- set parameters..
	if type(params)=="string" then
		obj:setParams(params);
	end
	return obj;
end

function plot.redraw(self)
	self.parent:redraw("inside");
end
  
function plot.getLegendStyle(self)
	local t={};
	t.lineStyle=self:get('lineStyle');
	t.lineWidth=self:get('lineWidth');
	t.penColor=self:get('penColor');
	t.markerStyle=self:get('markerStyle');
	return {t};
end
 
function plot.setParams(self,params)

	local r=self:get('penColor')[1];
	local g=self:get('penColor')[2];
	local b=self:get('penColor')[3];
	self:setg('penColor',{0,0,0}); -- default: black pen,remember old

	for i=1,params:len() do
		local c=params:sub(i,i);
		if c=='-' then
			self:setg('lineStyle','-');
		end
		if c==':' then
			self:setg('lineStyle',':');
		end
		if c=='r' then 
			self:setg('penColor',{1,0,0});
		end
		if c=='g' then
			self:setg('penColor',{0,1,0});
		end
		if c=='b' then
			self:setg('penColor',{0,0,1});
		end
		if c=='k' then
			self:setg('penColor',{0,0,0});
		end
		if c=='w' then
			self:setg('penColor',{1,1,1});
		end		
		if c=='y' then
			self:setg('penColor',{1,1,0});
		end
		if c=='m' then
			self:setg('penColor',{1,0,1});
		end
		if c=='c' then
			self:setg('penColor',{0,1,1});
		end
		if c=='x' then
			self:setg('markerStyle','cross');
		end
		if c=='+' then
			self:setg('markerStyle','plus');
		end
		if c=='*' then
			self:setg('markerStyle','star');
		end
		if c=='s' then
			self:setg('markerStyle','square');
		end
		if c=='d' then
			self:setg('markerStyle','diamond');
		end
		if c=='.' then
			self:setg('markerStyle','dot');
		end
		if c=='o' then
			self:setg('markerStyle','circle');
		end
		if c=='p' then 
			self:setg('penColor',{r,g,b});
		end	 
		if c=='T' then 
			self:setg('markerStyle','text_plot');
			self:setg('markerText',params:sub(i+1,-1));
			break;
		end
	end

end





function plot.draw(self,x,y)

	-- now do main draw ...

	local cr=self.windowHandle:begin();

	cr:set_source_rgb(unpack(self:get('penColor')))
	cr:set_line_width(self:get('lineWidth'));

	local lastx,lasty; -- last point to draw line
	 
	local axes=self:get('axes')
	local width=(axes[2]-axes[1]);
	local height=(axes[4]-axes[3]);
	local xscale=self:get('width')/width;
	local yscale=self:get('height')/height;

	for i=1,x:size(1) do
		local x1= (x[i]-axes[1])*xscale;
		local y1= self:get('height')- 
			  ((y[i]-axes[3])*yscale);
		if self:get('markerStyle')~=nil then
			local f=self.primitive[self:get('markerStyle')];
			f(self,x1,y1);
		end
		if lastx~=nil and self:get('lineStyle')~=nil then
			self.primitive.line(self,lastx,lasty,x1,y1);
		end 
		lastx=x1; lasty=y1;
	end

	self.windowHandle:theEnd();
end


--------------------------------------------- Extend gfx with plot command


local function new(t)    -- make Vector, a la  lab.new({a,b},{c,{d,e}}, ...)
	local v; 
	if type(t)=="number" then 
		v=torch.Tensor(1); v[1]=t;
	else
		v=torch.Tensor(#t); for i=1,#t do v[i]=t[i]; end 
	end
	return v; 
end 

local function range(r1,r2)
	if r2==nil then r2=r1; r1=1; end;
	local l=r2-r1+1;
	if l<1 then
		error('Range is not valid.');
	end 
	local t=torch.Tensor(l);
	for i=1,l do
		t[i]=r1+i-1;
	end
	return t;
end


local function convertParams(x,y,params)

	if type(x)=="table" or type(x)=="number" then
		x=new(x);
	end	
	if type(y)=="table" or type(y)=="number" then
		y=new(y);
	end  
 
	local function max(a,b)
		if a>b then return a,1; else return b,2; end;
	end

	if torch.typename(x)=="torch.Tensor" then
		if x:nDimension()==2 then
			local sz,ind;
			sz,ind=max(x:size(1),x:size(2));
			local t=torch.Tensor(sz);
			if x:size(1)>x:size(2) then
				for i=1,sz do t[i]=x[i][1];  end;
			else
				for i=1,sz do t[i]=x[1][i];  end;
			end
			x=t;
		end
	end
	if y~=nil and torch.typename(y)=="torch.Tensor" then
		if y:nDimension()==2 then
			local sz,ind;
			sz,ind=max(y:size(1),y:size(2));
			local t=torch.Tensor(sz);
			if y:size(1)>y:size(2) then
				for i=1,sz do t[i]=y[i][1];  end;
			else
				for i=1,sz do t[i]=y[1][i];  end;
			end
			y=t; 
		end
	end

	if type(y)=="string" or y==nil then
		params=y; 
		y=x; 
		x=range(1,y:size(1))-1; 
	end

	return x,y,params;
end



function gfx.plot(self,x,y,params)

	local axesHandle=self:getAxes();

	x,y,params=convertParams(x,y,params);
	if axesHandle:recalcAxes(x,y) then
		axesHandle:draw();
	end
	axesHandle:beginAxesScaling("inside");
	axesHandle:beginAxesClip("inside");

	local p=plot.new(axesHandle,x,y,params);	
	axesHandle:addObject(p)
 	p:draw(); 

	axesHandle:endAxesClip("inside");
	axesHandle:endAxesScaling("inside");
	axesHandle:drawAxes("inside");
	return p; 
end




