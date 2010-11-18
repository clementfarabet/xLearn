

function lab.new(...)
   local x;
   if type(arg[1])=="table" then
      x=torch.Tensor(#arg,#arg[1])
      for i=1,#arg do
	 for j=1,#arg[1] do
	    x[i][j]=arg[i][j];
	 end
      end
   else
      x=torch.Tensor(#arg)
      for i=1,#arg do
	 x[i]=arg[i];
      end
   end
   return x;
end


function lab.cat(...)
	 local dim=arg[1]:nDimension(); -- default cat to last dimension
	 local mats=arg.n
	 if type(arg[arg.n])=="number" then
	    dim=arg[arg.n];
	    mats=mats-1;
	 end	
	 local sz=0;
	 for i=1,mats do
	     sz=sz+arg[i]:size(dim)
	 end
	 local s=arg[1]:size(); s[dim]=sz; 
	 local x=torch.Tensor(s)    
	 local pos=1;
	 for i=1,mats do
	     x:narrow(dim,pos,arg[i]:size(dim)):copy(arg[i])
	     pos=pos+arg[i]:size(dim)
	 end
	 return x;
end