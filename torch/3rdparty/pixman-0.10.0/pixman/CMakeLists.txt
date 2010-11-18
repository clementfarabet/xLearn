# -*- cmake -*-

CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/pixman-version.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/pixman-version.h)
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})

SET(src 
	pixman.h			
	pixman-access.c			
	pixman-access-accessors.c	
	pixman-region.c			
	pixman-image.c			
	pixman-combine.c		
	pixman-compose.c		
	pixman-compose-accessors.c	
	pixman-pict.c			
	pixman-source.c			
	pixman-transformed.c		
	pixman-transformed-accessors.c	
	pixman-utils.c			
	pixman-edge.c			
	pixman-edge-accessors.c		
	pixman-trap.c			
	pixman-compute-region.c
	pixman-timer.c)

IF(USE_MMX)
  SET(src ${src} pixman-mmx.c)
ENDIF(USE_MMX)

IF(USE_SSE2)
  SET(src ${src} pixman-sse.c)
ENDIF(USE_SSE2)

IF(WIN32)
	ADD_DEFINITIONS(-DLIBPIXMAN_EXPORTS)
ENDIF(WIN32)

ADD_LIBRARY(pixman SHARED ${src})

INSTALL(TARGETS pixman 
  LIBRARY DESTINATION "${Torch_INSTALL_LIB_SUBDIR}" 
  ARCHIVE DESTINATION "${Torch_INSTALL_LIB_SUBDIR}" 
  RUNTIME DESTINATION "${Torch_INSTALL_BIN_SUBDIR}")

INSTALL(FILES ${CMAKE_CURRENT_BINARY_DIR}/pixman-version.h pixman.h 
  DESTINATION "${Torch_INSTALL_INCLUDE_SUBDIR}/pixman-1") 

# IF(WIN32)
#   ADD_DEFINITIONS(-DPIXMAN_VERSION_DLL)
# ENDIF(WIN32)

# ADD_EXECUTABLE(example example.c)
# TARGET_LINK_LIBRARIES(example pixman)

# ADD_EXECUTABLE(minigzip minigzip.c)
# TARGET_LINK_LIBRARIES(minigzip pixman)
