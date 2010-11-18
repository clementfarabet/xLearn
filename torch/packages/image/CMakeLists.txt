SET(src init.c transform.c)
SET(luasrc init.lua)

ADD_TORCH_PACKAGE(image "${src}" "${luasrc}" "Graphics")

TARGET_LINK_LIBRARIES(image luaT TH)
