SET(src )
SET(luasrc init.lua axes.lua draw.lua hist.lua init.lua io.lua legend.lua libgfx.lua plot.lua subplot.lua)
ADD_TORCH_PACKAGE(gfx "${src}" "${luasrc}" "Graphics")
