require "torch"
require "libsvm"

torch.include('svm', 'Kernel.lua')
torch.include('svm', 'SparseKernel.lua')
torch.include('svm', 'SVM.lua')
torch.include('svm', 'oaaSVM.lua')
