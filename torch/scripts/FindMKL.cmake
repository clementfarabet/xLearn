# 
# CMake Module for finding Intel's MKL
#
#   Charles O'Neill 8 April 2010
#    charles.oneill@gmail.com
#    www.caselab.okstate.edu

FIND_PATH(MKL_INCLUDE_DIR mkl_blas.h
  HINTS
  $ENV{MKLROOT}
  PATHS
  /usr/local
  /usr
  /opt
  $ENV{MKLROOT}
  )

FIND_LIBRARY(MKL_LIBRARY
  NAMES mkl_blas95 mkl_core
  PATH_SUFFIXES lib/32
  HINTS
  $ENV{MKLROOT}
  PATHS
  /opt
  $ENV{MKLROOT}
  )

SET(MKL_FOUND "NO")
IF(MKL_LIBRARY AND MKL_INCLUDE_DIR)
  SET(MKL_FOUND "YES")
ENDIF(MKL_LIBRARY AND MKL_INCLUDE_DIR)

IF( MKL_FOUND)
  MESSAGE(STATUS "Found intel mkl")
  MESSAGE(STATUS "  ${MKL_LIBRARY}")
  MESSAGE(STATUS "  ${MKL_INCLUDE_DIR}")
ELSE (MKL_FOUND)
  MESSAGE(STATUS "Could not find intel mkl")
ENDIF (MKL_FOUND)

MARK_AS_ADVANCED(MKL_LIBRARY 
  MKL_INCLUDE_DIR)
