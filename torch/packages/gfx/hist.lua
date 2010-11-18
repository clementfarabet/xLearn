---------------------------------------------- Define hist object

local hist={}

function hist.new(self)
	local obj=gfx.newObject(self);
	obj.className="hist"
	obj.redraw=hist.redraw;
	obj.getLegendStyle=hist.getLegendStyle;

	local colorOrder={};
	colorOrder={};
 	colorOrder[1]={1,0,0};    
 	colorOrder[2]={0,1,1};
	colorOrder[3]={1,1,0};
	colorOrder[4]={0,0.5,0};
	colorOrder[5]={0.75,0,0.75};
	colorOrder[6]={0.25,0.25,0.25};
 	colorOrder[7]={0,0,1};
	obj:setg('lineStyle','-');
	obj:setg('colorOrder',colorOrder);


	return obj;
end
 
function hist.getLegendStyle(self)
	local methods=self:get('methods')
	local m={};
	local colorOrder= self:get('colorOrder');
	for i=1,methods do
		local t={};
		t.lineStyle='-'; 
		t.lineWidth=self:get('lineWidth');
		t.penColor=colorOrder[ ((i-1)%(#colorOrder))+1]
		t.markerStyle=nil;
		m[#m+1]=t;
	end
	return m;
end


function hist.redraw(self)
	self.parent:redraw();
end



function hist.draw(self,methods,x,bins,bincnt)	

	local cr=self.windowHandle:begin();
	cr:set_source_rgb(unpack(self:get('penColor')))
	cr:set_line_width(self:get('lineWidth'));

	local axes=self:get('axes')
	local width=(axes[2]-axes[1]);
	local height=(axes[4]-axes[3]);
	local xscale=self:get('width')/width;
	local yscale=self:get('height')/height;
	local h=self:get('height')
	local w=self:get('width')

	local lastx,lasty; -- last point to draw line
	
	local colorOrder=self:get('colorOrder')

	for j=1,methods do

	-- set color for method..
	local p;
	p=colorOrder[ ((j-1)%(#colorOrder))+1]
	cr:set_source_rgb(unpack(p))

	for i=1,x:size(1) do  
		local x1= (x[i]-axes[1])*xscale;
		local y1= h- ((bincnt[i][j]-axes[3])*yscale);
		local width=((w/bins)*0.75);
		cr:rectangle(x1-width/2+(width/methods)*(j-1),y1,
				width/methods,h-y1);
		cr:fill();  
		lastx=x1; lasty=y1;
	end
	end   

	self.windowHandle:theEnd();
end


---------------------------------------------- Extend gfx with hist command

require "lab"


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
	if type(y)=="table" then
		y=new(y);
	end  
 
	local function max(a,b)
		if a>b then return a,1; else return b,2; end;
	end 

	if torch.typename(x)=="torch.Tensor" then
		-- has to be an n x p matrix, not an n x 0
		-- where n is number of example, and p number of methods
		if x:nDimension()==1 then
			local t=torch.Tensor(x:size(1),1);
			for i=1,x:size(1) do t[i][1]=x[i];  end;
			x=t;
		end
	end
 
	if type(y)=="string" or y==nil then
		params=y; 
		y=nil;
	end 	

	local bmin=x:min();
	local bmax=x:max();
	local bins=10;
	local methods=x:size(2);
	if y==nil or type(y)=="number" then -- find bin centers
		if type(y)=="number" then bins=y; end
		y=torch.Tensor(bins);
		for i=1,bins do 
			y[i]=bmin*(1-(i-1)/(bins-1))+bmax*((i-1)/(bins-1));  
		end;
	else
		bins=y:size(1);
	end
	local bincnt=torch.Tensor(bins,methods);
	bincnt:zero();
	for i=1,methods do
	for j=1,x:size(1) do 
		local b= y-x[j][i];
		-- find argmin
		local min=1;
		for k=1,b:size(1) do
			if math.abs(b[k])<math.abs(b[min]) then 
				min=k; 
			end;
		end
		bincnt[min][i]=bincnt[min][i]+1;
	end
	end  

	local width1=(((y:max()-y:min())/bins)*0.75);
	local haxes={y:min()-width1,y:max()+width1,0,bincnt:max()};

	return methods,y,bins,bincnt,haxes;
end

function gfx.hist(self,x,y,params)

	local methods,bins,bincnt,haxes;
	methods,x,bins,bincnt,haxes=convertParams(x,y,params);
	if params==nil then params='b-x'; end; -- set parameters..

	local axesHandle=self:getAxes();
	--axesHandle:cls();
	axesHandle:axis(unpack(haxes));
	axesHandle:beginAxesScaling();
	axesHandle:beginAxesClip();
	local p;
	p=hist.new(axesHandle);	
	axesHandle:addObject(p)
	local args={methods,x,bins,bincnt,haxes}
	function p.draw(slf)
		local f=hist.draw;   
		local a=args;	
		f(slf,unpack(a));
	end;
	p:setg('methods',methods)
	--if type(params)=="string" then
	--	p:setParams(params);
	--end
	p:draw();
	axesHandle:endAxesClip();
	axesHandle:endAxesScaling();
	axesHandle:drawAxes("inside");
	return p; 
end



