# -*- cmake -*-

INCLUDE_DIRECTORIES(${3RDPARTY_SOURCE_DIR}/${ZLIB_SUBDIR})

SET(src png.c pngset.c pngget.c pngrutil.c pngtrans.c pngwutil.c
        pngread.c pngrio.c pngwio.c pngwrite.c pngrtran.c
        pngwtran.c pngmem.c pngerror.c pngpread.c
        png.h pngconf.h)

IF(MSVC)
  SET(src ${src} scripts/pngw32.def)
ENDIF(MSVC)

ADD_LIBRARY(png SHARED ${src})
TARGET_LINK_LIBRARIES(png zlib)

IF(UNIX)
  TARGET_LINK_LIBRARIES(png m)
ENDIF(UNIX)

# To compile example
# ADD_EXECUTABLE(pngtest pngtest.c)
# TARGET_LINK_LIBRARIES(pngtest png)

INSTALL(TARGETS png 
  LIBRARY DESTINATION "${Torch_INSTALL_LIB_SUBDIR}"
  ARCHIVE DESTINATION "${Torch_INSTALL_LIB_SUBDIR}"
  RUNTIME DESTINATION "${Torch_INSTALL_BIN_SUBDIR}")
INSTALL(FILES png.h pngconf.h DESTINATION "${Torch_INSTALL_INCLUDE_SUBDIR}")
