
SET(src cudaflow.c)

SET(luasrc init.lua)

ADD_TORCH_PACKAGE(cudaFlow "${src}" "${luasrc}")
