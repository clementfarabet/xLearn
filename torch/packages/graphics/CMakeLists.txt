# -*- cmake -*-

SET(src )
SET(luasrc init.lua wrap-qt.lua wrap-cairo.lua)
ADD_TORCH_PACKAGE(graphics "${src}" "${luasrc}" "Graphics")

