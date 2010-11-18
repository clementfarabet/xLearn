#SET(src svqp2.h svqp2.cpp QPSolver.cpp init.c Kernel.c SVM.c SparseKernel.c)
#SET(luasrc init.lua Kernel.lua SVM.lua oaaSVM.lua SparseKernel.lua)

#ADD_TORCH_PACKAGE(svm "${src}" "${luasrc}")

#TARGET_LINK_LIBRARIES(svm luaT TH)
