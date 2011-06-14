
SET(src)
SET(luasrc init.lua)
ADD_TORCH_PACKAGE(inline "${src}" "${luasrc}" "C/CPP API")
