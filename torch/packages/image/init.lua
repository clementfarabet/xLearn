require "torch"
require('libimage')

--use_gfx="" 
if use_gfx then  -- This creates an uneccesary dependency - should move to gfx
require "gfx"
	local function loadPng(filename,type,depth)
		local surf=gfx.ImageSurface(filename);
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
		return x;
	end
	rawset(image, 'load', loadPng)
	 
	local function savePng(filename,x)
		local surf = gfx.ImageSurface(x:size(1),x:size(2))
	        surf.lcairo_object:from_tensor(x);
	 	surf.lcairo_object:write_to_png(filename)
	end  
	rawset(image, 'save', savePng)
end

local function crop(src,dst,startx,starty,endx,endy)
	if endx==nil then
		return image.cropNoScale(src,dst,startx,starty);
	else
		local depth=0;
		if src:nDimension()>2 then
			depth=src:size(3);
		end
		local x=torch.Tensor(endx-startx,endy-starty,depth);
		image.cropNoScale(src,x,startx,starty);
		image.scale(x,dst);
	end
end
rawset(image, 'crop', crop)

local function scale(src,dst,type)
	if type=='bilinear' then
		image.scaleBilinear(src,dst);
	else
		image.scaleSimple(src,dst);
	end

end
rawset(image, 'scale', scale)

local function convolveInPlace(mysrc,kernel,pad_const)
	local kH=kernel:size(1);  
	local kW=kernel:size(2); 
	local stepW=1; 
	local stepH=1;  

	local inputHeight =mysrc:size(1); 
	local outputHeight = (inputHeight-kH)/stepH + 1
	local inputWidth = mysrc:size(2);  
	local outputWidth = (inputWidth-kW)/stepW + 1

	-- create destination so it is the same size as input,
	-- and pad input so convolution makes the same size
	outputHeight=inputHeight; 
	outputWidth=inputWidth;
	inputWidth=((outputWidth-1)*stepW)+kW; 
	inputHeight=((outputHeight-1)*stepH)+kH; 
	local src;
	src=torch.Tensor(inputHeight,inputWidth);
	src:zero(); src=src + pad_const;
	image.translate(mysrc,src,math.floor(kW/2),math.floor(kH/2));

	mysrc:zero(); 

	mysrc:addT4dotT2(1,
		src:unfold(1, kW, stepW):unfold(2, kH, stepH),
		kernel)
	return mysrc;
end 
rawset(image, 'convolveInPlace', convolveInPlace) 

local function convolveToDst(src,dst,kernel)
	local kH=kernel:size(1);  
	local kW=kernel:size(2); 
	local stepW=1; 
	local stepH=1; 

	local inputHeight =src:size(1); 
	local outputHeight = (inputHeight-kH)/stepH + 1
	local inputWidth = src:size(2);  
	local outputWidth = (inputWidth-kW)/stepW + 1

	if dst==nil then
		dst=torch.Tensor(outputHeight,outputWidth); 
		dst:zero();
	end  

	dst:addT4dotT2(1,
		src:unfold(1, kW, stepW):unfold(2, kH, stepH),
		kernel)
	return dst;
end 
rawset(image, 'convolveToDst', convolveToDst) 

local function convolve(p1,p2,p3)
	if type(p3)=="number" then
		image.convolveInPlace(p1,p2,p3)
	else
		image.convolveToDst(p1,p2,p3)
	end
end
rawset(image, 'convolve', convolve) 


-- Backwards compatibility functions (remove later)::
rawset(image, 'scale_bilinear', image.scaleBilinear)
rawset(image, 'crop_noscale', image.cropNoScale) 
rawset(image, 'convolve_inplace', convolveInPlace) 
