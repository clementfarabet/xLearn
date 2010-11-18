local LinearSparseKernel = torch.class('svm.LinearSparseKernel', 'svm.Kernel')
function LinearSparseKernel:__init(gamma)
   self.gamma = gamma or 1
   self:__cinit(self.gamma)
end

function LinearSparseKernel:write(file)
   file:writeDouble(self.gamma)
end

function LinearSparseKernel:read(file)
   self.gamma = file:readDouble()
   self:__cinit(self.gamma)
end

----------------------------------------------------------------------

local RBFSparseKernel = torch.class('svm.RBFSparseKernel', 'svm.Kernel')
function RBFSparseKernel:__init(gamma)
   self.gamma = gamma or 1
   self:__cinit(self.gamma)
end

function RBFSparseKernel:write(file)
   file:writeDouble(self.gamma)
end

function RBFSparseKernel:read(file)
   self.gamma = file:readDouble()
   self:__cinit(self.gamma)
end

----------------------------------------------------------------------

local PolynomialSparseKernel = torch.class('svm.PolynomialSparseKernel', 'svm.Kernel')
function PolynomialSparseKernel:__init(degree, gamma, bias)
   self.degree = degree or 1
   self.gamma = gamma or 1
   self.bias = bias or 0
   self:__cinit(self.degree, self.gamma, self.bias)
end

function PolynomialSparseKernel:write(file)
   file:writeDouble(self.degree)
   file:writeDouble(self.gamma)
   file:writeDouble(self.bias)
end

function PolynomialSparseKernel:read(file)
   self.degree = file:readDouble()
   self.gamma = file:readDouble()
   self.bias = file:readDouble()
   self:__cinit(self.degree, self.gamma, self.bias)
end

----------------------------------------------------------------------

local TanhSparseKernel = torch.class('svm.TanhSparseKernel', 'svm.Kernel')
function TanhSparseKernel:__init(gamma, bias)
   self.gamma = gamma or 1
   self.bias = bias or 0
   self:__cinit(self.gamma, self.bias)
end

function TanhSparseKernel:write(file)
   file:writeDouble(self.gamma)
   file:writeDouble(self.bias)
end

function TanhSparseKernel:read(file)
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
