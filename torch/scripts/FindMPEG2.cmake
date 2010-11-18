# - Find the native MPEG2 includes and library
# This module defines
#  MPEG2_INCLUDE_DIR, path to mpeg2dec/mpeg2.h, etc.
#  MPEG2_LIBRARIES, the libraries required to use MPEG2.
#  MPEG2_FOUND, If false, do not try to use MPEG2.
# also defined, but not for general use are
#  MPEG2_mpeg2_LIBRARY, where to find the MPEG2 library.
#  MPEG2_vo_LIBRARY, where to find the vo library.

FIND_PATH(MPEG2_INCLUDE_DIR mpeg2.h
  /usr/include/mpeg2dec
  /usr/local/include
  /usr/local/include/mpeg2dec
  /usr/include
  /usr/local/livid
)

FIND_LIBRARY(MPEG2_mpeg2_LIBRARY mpeg2
  /usr/local/lib
  /usr/lib
  /usr/local/livid/mpeg2dec/libmpeg2/.libs
)

FIND_LIBRARY( MPEG2_mpeg2convert_LIBRARY mpeg2convert
  /usr/local/lib
  /usr/lib
  /usr/local/livid/mpeg2dec/libvo/.libs
)

MARK_AS_ADVANCED(
  MPEG2_INCLUDE_DIR
  MPEG2_mpeg2_LIBRARY
  MPEG2_mpeg2convert_LIBRARY)

SET( MPEG2_FOUND "NO" )
IF(MPEG2_INCLUDE_DIR)
  IF(MPEG2_mpeg2_LIBRARY)
    IF(MPEG2_mpeg2convert_LIBRARY)
      SET( MPEG2_FOUND "YES" )
      SET( MPEG2_LIBRARIES
        ${MPEG2_mpeg2_LIBRARY} 
        ${MPEG2_mpeg2convert_LIBRARY})

      #some native mpeg2 installations will depend
      #on libSDL, if found, add it in.
      INCLUDE( ${CMAKE_ROOT}/Modules/FindSDL.cmake )
      MARK_AS_ADVANCED(
        SDLMAIN_LIBRARY
        SDL_INCLUDE_DIR)
      IF(SDL_FOUND)
        SET( MPEG2_LIBRARIES ${MPEG2_LIBRARIES} ${SDL_LIBRARY})
      ENDIF(SDL_FOUND)
    ENDIF(MPEG2_mpeg2convert_LIBRARY)
  ENDIF(MPEG2_mpeg2_LIBRARY)
ENDIF(MPEG2_INCLUDE_DIR)

IF(MPEG2_FOUND)
  MESSAGE(STATUS "Found MPEG2 library")
ELSE(MPEG2_FOUND)
  IF(MPEG2_FIND_REQUIRED)
    MESSAGE(FATAL_ERROR "Could not find MPEG2 -- please give some paths to CMake")
  ELSE(MPEG2_FIND_REQUIRED)
    MESSAGE(STATUS "Could not find MPEG2 -- please give some paths to CMake")
  ENDIF(MPEG2_FIND_REQUIRED)
ENDIF(MPEG2_FOUND)
