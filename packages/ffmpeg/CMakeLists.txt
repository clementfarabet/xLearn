
SET(src)

SET(luasrc init.lua Video.lua)

ADD_TORCH_PACKAGE(ffmpeg "${src}" "${luasrc}" "Video")
