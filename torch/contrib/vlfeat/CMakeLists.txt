
# build lib + wrapper
SET(src aib.c generic.c hikmeans.c 
        host.c imopv.c kdtree.c  mathop.c  mser.c pgm.c random.c sift.c
        dsift.c getopt_long.c  homkermap.c ikmeans.c imopv_sse2.c
        kmeans.c mathop_sse2.c pegasos.c quickshift.c rodrigues.c
        stringop.c vlfeat.c)
SET(luasrc init.lua pattern-1.jpg)

SET(CMAKE_C_FLAGS "-msse2 -std=gnu99")

ADD_TORCH_PACKAGE(vlfeat "${src}" "${luasrc}")
TARGET_LINK_LIBRARIES(vlfeat luaT TH)
