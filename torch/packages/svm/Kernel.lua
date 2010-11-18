local Kernel = torch.class('svm.Kernel')

local LinearKernel = torch.class('svm.LinearKernel', 'svm.Kernel')
function LinearKernel:__init(gamma)
   self.gamma = gamma or 1
   self:__cinit(self.gamma)
end

function LinearKernel:write(file)
   file:writeDouble(self.gamma)
end

function LinearKernel:read(file)
   self.gamma = file:readDouble()
   self:__cinit(self.gamma)
end

----------------------------------------------------------------------

local RBFKernel = torch.class('svm.RBFKernel', 'svm.Kernel')
function RBFKernel:__init(gamma)
   self.gamma = gamma or 1
   self:__cinit(self.gamma)
end

function RBFKernel:write(file)
   file:writeDouble(self.gamma)
end

function RBFKernel:read(file)
   self.gamma = file:readDouble()
   self:__cinit(self.gamma)
end

----------------------------------------------------------------------

local PolynomialKernel = torch.class('svm.PolynomialKernel', 'svm.Kernel')
function PolynomialKernel:__init(degree, gamma, bias)
   self.degree = degree or 1
   self.gamma = gamma or 1
   self.bias = bias or 0
   self:__cinit(self.degree, self.gamma, self.bias)
end

function PolynomialKernel:write(file)
   file:writeDouble(self.degree)
   file:writeDouble(self.gamma)
   file:writeDouble(self.bias)
end

function PolynomialKernel:read(file)
   self.degree = file:readDouble()
   self.gamma = file:readDouble()
   self.bias = file:readDouble()
   self:__cinit(self.degree, self.gamma, self.bias)
end

----------------------------------------------------------------------

local TanhKernel = torch.class('svm.TanhKernel', 'svm.Kernel')
function TanhKernel:__init(gamma, bias)
   self.gamma = gamma or 1
   self.bias = bias or 0
   self:__cinit(self.gamma, self.bias)
end

function TanhKernel:write(file)
   file:writeDouble(self.gamma)
   file:writeDouble(self.bias)
end

function TanhKernel:read(file)
   self.gamma = file:readDouble()
   self.bias = file:readDouble()
   self:__cinit(self.gamma, self.bias)
end

----------------------------------------------------------------------

-- DEBUG: note: __gc peut etre redirige pour caller aussi __cinit? (not sure)

-- on peut mettre un __gc optionel dans les tables... celui-ci pointe sur un userdata vide.
-- on peut pas tout mettre dans une table. genre un tensor, tu veux que ca prenne pas trop de place. euh, c'est sur ca?
-- parce que ca serait vraiment plus clean -- pour l'instant, dur d'apprehender le torch_newmetatable, plus chiant
-- pour heritage (lua vs non-lua... tricky pour kernel par exemple... t'as vu pourquoi ca marche??)
-- ou alors tout descend d'un userdata + eventuel get/set env...
-- pcall pour eviter plantage bus error? ou modification du code de svqp2?
