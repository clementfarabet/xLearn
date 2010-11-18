# -*- cmake -*-

SET(src 
     adler32.c
     crc32.c
     infback.c
     inflate.c
     uncompr.c
     compress.c
     deflate.c
     gzio.c
     inffast.c
     inftrees.c
     trees.c
     zutil.c)

ADD_LIBRARY(zlib SHARED ${src})

IF(WIN32)
  ADD_DEFINITIONS(-DZLIB_DLL)
ENDIF(WIN32)

# To compile examples
# ADD_EXECUTABLE(example example.c)
# TARGET_LINK_LIBRARIES(example zlib)
#
# ADD_EXECUTABLE(minigzip minigzip.c)
# TARGET_LINK_LIBRARIES(minigzip zlib)

INSTALL(TARGETS zlib 
  LIBRARY DESTINATION "${Torch_INSTALL_LIB_SUBDIR}"
  ARCHIVE DESTINATION "${Torch_INSTALL_LIB_SUBDIR}"
  RUNTIME DESTINATION "${Torch_INSTALL_BIN_SUBDIR}")
INSTALL(FILES zlib.h zconf.h 
  DESTINATION "${Torch_INSTALL_INCLUDE_SUBDIR}")
