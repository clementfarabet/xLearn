local PairwiseDistance, parent = torch.class('nn.PairwiseDistance', 'nn.Module')

function PairwiseDistance:__init(p)
   parent.__init(self)

   -- state
   self.gradInput = {torch.Tensor(), torch.Tensor()}
   self.output = torch.Tensor(1)
   self.norm=p
end 
  
function PairwiseDistance:forward(input)
   self.output[1]=input[1]:dist(input[2],self.norm);
   return self.output
end

local function mathsign(x) 
   if x==0 then return  2*math.random(2)-3; end
   if x>0 then return 1; else return -1; end
end

function PairwiseDistance:backward(input, gradOutput)
  self.gradInput[1]:resizeAs(input[1]) 
  self.gradInput[2]:resizeAs(input[2]) 
  self.gradInput[1]:copy(input[1])
  self.gradInput[1]:add(-1, input[2])
  if self.norm==1 then
     self.gradInput[1]:apply(mathsign)
  end
  self.gradInput[1]:mul(gradOutput[1]);
  self.gradInput[2]:zero():add(-1, self.gradInput[1])
  return self.gradInput
end


function PairwiseDistance:zeroGradParameters()
	-- no weights to adjust
end

function PairwiseDistance:updateParameters(learningRate)
	-- no weights to adjust
end


function PairwiseDistance:write(file)
   parent.write(self, file)
   file:writeDouble(self.norm)
end

function PairwiseDistance:read(file)
   parent.read(self, file)
   self.norm = file:readDouble()
end



