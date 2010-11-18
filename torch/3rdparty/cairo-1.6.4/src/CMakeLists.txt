# -*- cmake -*-

SET(CAIRO_SOURCE)
SET(CAIRO_HEADERS)

# Basic stuff
SET(CAIRO_SOURCE ${CAIRO_SOURCE}
	cairo.c					
	cairo.h					
	cairo-private.h				
	cairo-arc.c				
	cairo-arc-private.h			
	cairo-array.c				
	cairo-atomic.c				
	cairo-atomic-private.h			
	cairo-base85-stream.c			
	cairo-bentley-ottmann.c			
	cairo-cache.c				
	cairo-clip.c				
	cairo-clip-private.h			
	cairo-color.c				
	cairo-compiler-private.h		
	cairo-debug.c				
	cairo-fixed.c				
	cairo-font-face.c			
	cairo-font-options.c			
	cairo-freelist.c			
	cairo-freelist-private.h		
	cairo-gstate.c				
	cairo-gstate-private.h			
	cairo-hash.c				
	cairo-hash-private.h			
	cairo-hull.c				
	cairo-image-surface.c			
	cairo-lzw.c				
	cairo-misc.c				
	cairo-malloc-private.h			
	cairo-matrix.c				
	cairo-mutex.c				
	cairo-mutex-private.h			
	cairo-mutex-type-private.h		
	cairo-path.c				
	cairo-path-private.h			
	cairo-path-bounds.c			
	cairo-path-fill.c			
	cairo-path-fixed.c			
	cairo-path-fixed-private.h		
	cairo-path-stroke.c			
	cairo-pen.c				
	cairo-polygon.c				
	cairo-rectangle.c			
	cairo-reference-count-private.h		
	cairo-region.c				
	cairo-region-private.h			
	cairo-scaled-font.c			
	cairo-scaled-font-private.h		
	cairo-skiplist.c			
	cairo-skiplist-private.h		
	cairo-slope.c				
	cairo-spline.c				
	cairo-stroke-style.c			
	cairo-surface.c				
	cairo-surface-fallback.c		
	cairo-surface-fallback-private.h	
	cairo-surface-private.h			
	cairo-traps.c				
	cairo-pattern.c				
	cairo-unicode.c				
	cairo-output-stream.c			
	cairo-output-stream-private.h		
	cairo-wideint.c				
	cairo-wideint-private.h			
	cairo-wideint-type-private.h		
	cairo-meta-surface.c			
	cairo-meta-surface-private.h		
	cairo-paginated-private.h		
	cairo-paginated-surface.c		
	cairo-paginated-surface-private.h	
	cairo-analysis-surface.c		
	cairo-analysis-surface-private.h	
	cairoint.h)

SET(CAIRO_HEADERS ${CAIRO_HEADERS} cairo.h cairo-deprecated.h)

# Font base
SET(CAIRO_SOURCE ${CAIRO_SOURCE}
	cairo-cff-subset.c
	cairo-truetype-subset.c
	cairo-type1-fallback.c
	cairo-type1-private.h	
	cairo-truetype-subset-private.h
	cairo-scaled-font-subsets.c
	cairo-scaled-font-subsets-private.h)

# FT fonts
INCLUDE(${3RDPARTY_SOURCE_DIR}/cmake/FindFontConfig.cmake)
INCLUDE(${3RDPARTY_SOURCE_DIR}/cmake/FindFreetype2.cmake)
IF(FREETYPE2_FOUND AND FONTCONFIG_FOUND)
  INCLUDE_DIRECTORIES(${FREETYPE2_INCLUDE_DIRS} ${FONTCONFIG_INCLUDE_DIR})
  SET(CAIRO_LIBS ${FREETYPE2_LIBRARIES} ${FONTCONFIG_LIBRARIES})
  SET(CAIRO_HAS_FT_FONT 1)
  SET(CAIRO_SOURCE ${CAIRO_SOURCE} cairo-type1-subset.c
    cairo-ft.h
    cairo-ft-font.c cairo-ft-private.h)

  SET(CAIRO_HEADERS ${CAIRO_HEADERS} cairo-ft.h)

ENDIF(FREETYPE2_FOUND AND FONTCONFIG_FOUND)

# PS
# Requires only zlib that we do compile
SET(CAIRO_HAS_PS_SURFACE 1)
SET(CAIRO_SOURCE ${CAIRO_SOURCE} cairo-ps-surface.c cairo-ps-surface-private.h)
SET(CAIRO_HEADERS ${CAIRO_HEADERS} cairo-ps.h)

# Pdf
# Requires only zlib that we do compile
SET(CAIRO_HAS_PDF_SURFACE 1)
SET(CAIRO_SOURCE ${CAIRO_SOURCE} cairo-pdf.h
                                 cairo-pdf-surface.c cairo-pdf-surface-private.h
	                         cairo-pdf-operators.c cairo-pdf-operators-private.h
	                         cairo-deflate-stream.c)
SET(CAIRO_HEADERS ${CAIRO_HEADERS} cairo-pdf.h)

# Png
# Requires libpng that we do compile
SET(CAIRO_HAS_PNG_FUNCTIONS 1)
SET(CAIRO_SOURCE ${CAIRO_SOURCE} cairo-png.c)

# Xlib
INCLUDE(FindX11)
IF(X11_FOUND)
  INCLUDE_DIRECTORIES(${X11_INCLUDE_DIR})
  SET(CAIRO_LIBS ${CAIRO_LIBS} ${X11_LIBRARIES})
  SET(CAIRO_HAS_XLIB_SURFACE 1)
  SET(CAIRO_SOURCE ${CAIRO_SOURCE} cairo-xlib.h
                                   cairo-xlib-surface.c
	                           cairo-xlib-surface-private.h
		                   cairo-xlib-display.c
		                   cairo-xlib-screen.c
		                   cairo-xlib-visual.c
		                   cairo-xlib-private.h
		                   cairo-xlib-xrender-private.h)

  SET(CAIRO_HEADERS ${CAIRO_HEADERS} cairo-xlib.h)

ENDIF(X11_FOUND)

# Quartz
FIND_LIBRARY(APPLICATIONSERVICES_LIB ApplicationServices)
  MARK_AS_ADVANCED(APPLICATIONSERVICES_LIB)
IF(APPLICATIONSERVICES_LIB)
  FIND_LIBRARY(COREGRAPHICS_LIB CoreGraphics)
  MARK_AS_ADVANCED(COREGRAPHICS_LIB)
  SET(CAIRO_HAS_QUARTZ_SURFACE 1)
  SET(CAIRO_HAS_QUARTZ_FONT 1)
  SET(CAIRO_HAS_QUARTZ_IMAGE_SURFACE 1)
  SET(CAIRO_SOURCE ${CAIRO_SOURCE} cairo-quartz.h
                                   cairo-quartz-surface.c cairo-quartz-image-surface.c cairo-quartz-private.h
                                   cairo-quartz-image.h
                                   cairo-quartz-font.c)

  SET(CAIRO_HEADERS ${CAIRO_HEADERS} cairo-quartz.h)

  IF(COREGRAPHICS_LIB)
    SET(CAIRO_LIBS ${CAIRO_LIBS} ${COREGRAPHICS_LIB})
  ELSE(COREGRAPHICS_LIB)
    SET(CAIRO_LIBS ${CAIRO_LIBS} ${APPLICATIONSERVICES_LIB})
  ENDIF(COREGRAPHICS_LIB)

ENDIF(APPLICATIONSERVICES_LIB)

# Native WIN32
IF(MSVC)
  SET(CAIRO_SOURCE ${CAIRO_SOURCE} cairo-win32.c)
ENDIF(MSVC)

# Win32
IF(HAVE_WINDOWS_H)
  SET(CAIRO_SOURCE ${CAIRO_SOURCE} cairo-win32.h
                                   cairo-win32-surface.c cairo-win32-printing-surface.c cairo-win32-private.h
                                   cairo-win32-font.c)
  SET(CAIRO_HEADERS ${CAIRO_HEADERS} cairo-win32.h)
  SET(CAIRO_HAS_WIN32_SURFACE 1)
  SET(CAIRO_HAS_WIN32_FONT 1)
  ADD_DEFINITIONS(-DLIBCAIRO_EXPORTS)
ENDIF(HAVE_WINDOWS_H)

INCLUDE_DIRECTORIES(${3RDPARTY_SOURCE_DIR}/${PIXMAN_SUBDIR}/pixman ${3RDPARTY_BINARY_DIR}/${PIXMAN_SUBDIR}/pixman)
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})
INCLUDE_DIRECTORIES(${3RDPARTY_SOURCE_DIR}/${LIBPNG_SUBDIR})
INCLUDE_DIRECTORIES(${3RDPARTY_SOURCE_DIR}/${ZLIB_SUBDIR})
CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/cairo-features.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/cairo-features.h)
SET(CAIRO_HEADERS ${CAIRO_HEADERS} ${CMAKE_CURRENT_BINARY_DIR}/cairo-features.h)

ADD_LIBRARY(cairo SHARED ${CAIRO_SOURCE})

TARGET_LINK_LIBRARIES(cairo ${CAIRO_LIBS} png pixman zlib)

IF(WIN32)
  TARGET_LINK_LIBRARIES(cairo gdi32 msimg32)
ENDIF(WIN32)

INSTALL(TARGETS cairo 
  LIBRARY DESTINATION "${Torch_INSTALL_LIB_SUBDIR}"
  ARCHIVE DESTINATION "${Torch_INSTALL_LIB_SUBDIR}"
  RUNTIME DESTINATION "${Torch_INSTALL_BIN_SUBDIR}")

INSTALL(FILES ${CAIRO_HEADERS} 
  DESTINATION "${Torch_INSTALL_INCLUDE_SUBDIR}/cairo")

SET(CAIRO_LIBRARIES cairo png zlib pixman 
  CACHE STRING "Cairo libraries" FORCE)
SET(CAIRO_INCLUDE_DIR 
  ${3RDPARTY_SOURCE_DIR}/${CAIRO_SUBDIR}/src 
  ${3RDPARTY_BINARY_DIR}/${CAIRO_SUBDIR}/src 
  CACHE STRING "Cairo include directories" FORCE)
MARK_AS_ADVANCED(CAIRO_LIBRARIES CAIRO_INCLUDE_DIR)
CONFIGURE_FILE(CairoConfig.cmake.in "${Torch_BINARY_DIR}/cmake/CairoConfig.cmake")
SET(Cairo_DIR "${Torch_BINARY_DIR}/cmake" CACHE PATH "Directory containing CairoConfig.cmake")
MARK_AS_ADVANCED(Cairo_DIR)
