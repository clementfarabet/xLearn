# build lib + wrapper

include_directories(include imagelib matrix sba-1.5 sfm-driver)

# imagelib
FILE( GLOB imagelib_src_c imagelib/affine.c imagelib/bmp.c
 imagelib/canny.c imagelib/color.c imagelib/fileio.c
 imagelib/filter.c imagelib/fit.c imagelib/fmatrix.c
 imagelib/homography.c imagelib/horn.c imagelib/image.c
 imagelib/lerp.c imagelib/morphology.c imagelib/pgm.c
 imagelib/poly.c imagelib/qsort.c imagelib/resample.c
 imagelib/tps.c imagelib/transform.c imagelib/triangulate.c
 imagelib/util.c ) 
ADD_LIBRARY(sfmimagelib STATIC ${imagelib_src_c})

#matrix
FILE( GLOB matrix_src_c matrix/matrix.c matrix/svd.c
  matrix/vector.c) 
ADD_LIBRARY(sfmmatrix STATIC ${matrix_src_c})
TARGET_LINK_LIBRARIES(sfmmatrix sfmimagelib cblas lapack minpack)
#sba
FILE( GLOB sba_src_c sba-1.5/sba_chkjac.c sba-1.5/sba_crsm.c
  sba-1.5/sba_lapack.c sba-1.5/sba_levmar.c
  sba-1.5/sba_levmar_wrap.c) 
ADD_LIBRARY(sbalib STATIC ${sba_src_c})
TARGET_LINK_LIBRARIES(sbalib sfmmatrix)

#sfmdrv
FILE ( GLOB sfmdrv_src_c sfm-driver/sfm.c)
ADD_LIBRARY(sfmdrv STATIC ${sfmdrv_src_c})
TARGET_LINK_LIBRARIES(sfmdrv sbalib sfmmatrix sfmimagelib)

SET(src bundler.c)
SET(luasrc init.lua)
SET(CMAKE_C_FLAGS "-O3 -Wall -D__NO_MKL__")
ADD_TORCH_PACKAGE(bundler "${src}" "${luasrc}")
TARGET_LINK_LIBRARIES(bundler 
  luaT TH sfmimagelib sfmmatrix sbalib sfmdrv )
