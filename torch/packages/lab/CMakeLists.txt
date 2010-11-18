SET(src init.c stat.c math.c op.c manip.c build.c utils.c)
SET(luasrc init.lua liblab.lua)

ADD_TORCH_PACKAGE(lab "${src}" "${luasrc}" "Maths")

TARGET_LINK_LIBRARIES(lab luaT TH)
