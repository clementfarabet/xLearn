--------------------------------------------------------------------------------
-- DrLimCriterion discrimintive loss can be used for DrLim type training
-- 
-- Authors: Benoit Corda / cordaben@gmail.com
--------------------------------------------------------------------------------
local help_desc =
   [[This module computes the loss (the distance between the outputs)
      of two different samples using a square square loss]]

local help_example =
[["-- example of gradient step given 2 inputs
   function gradUpdate(model, model_clone, input1, input2, learningRateSimilar, learningRateDissimilar)
      local criterion = nn.DrLimCriterion()
      local similar = true
      loss = DrLimCriterion:forward(input1,input2, similar)
      model:zerogradParameters();
      local grad = criterion:backward(input1,input2, similar)
      model:backward(input1, grad[1])
      -- set the learning rate
      local learningRate
      if similar then
         learningRate = learningRateSimilar
      else
         learningRate = learningRateDissimilar
      end
      -- update the weights
      model:updateParameters(learningRate)
      model_clone:backward(input2, grad[2])
      model_clone:updateParameters(learningRate)
   end"
]]      

local DrLimCriterion, parent = torch.class('nn.DrLimCriterion', 'nn.Criterion')

function DrLimCriterion:__usage()
   local usage = toolBox.usage('nn.DrLimCriterion',
                 help_desc,
                 help_example,
                 {type='number', help='margin of the loss function, default is 1.', req=false})
   print(usage)
end

function DrLimCriterion:__init(margin)
   self.gradInput = {torch.Tensor(),torch.Tensor()}
   self.output = 0
   self.margin = margin or 1
end

function DrLimCriterion:forward(input1, input2, similar)
   local squaredist = (input1 - input2):pow(2):sum()

   if similar then
      self.output = squaredist * 0.5
   else
      local eucldist = math.sqrt(squaredist)
      local dist = (self.margin - eucldist)
      self.output = 0
      if dist > 0 then
         self.output = (dist*dist) * 0.5
      end
   end

   return self.output
end

function DrLimCriterion:backward(input1, input2, similar)
   self.gradInput[1]:resizeAs(input1)
   self.gradInput[2]:resizeAs(input2)
   self.gradInput[1]:zero()
   self.gradInput[2]:zero()

   local diff = input1 - input2
   
   if similar then
      self.gradInput[1] = diff
      self.gradInput[2] = -diff
   else -- only gradient if the two are dissimilars by less than margin
      -- meaning the energy is not null
      if self.output ~= 0 then
         local l2dist = input1:dist(input2)
         local c = 0
         if l2dist == 0 then
            c = 100 --arbitrary big value to avoid zero division
         else
            c = (self.margin - l2dist) / l2dist
         end
         self.gradInput[1] = diff * (-c)
         self.gradInput[2] = diff * c
      end
   end
         
   return self.gradInput
end