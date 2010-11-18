SET(src init.c lcairo.c window.c window_win32.c window_X11.c threads.c keytable.c )
SET(luasrc init.lua )

FIND_PACKAGE(Cairo)
FIND_PACKAGE(X11)
FIND_PACKAGE(Readline)

IF(NOT WIN32)
  FIND_PACKAGE(Threads)
  IF(NOT CMAKE_USE_PTHREADS_INIT)
    MESSAGE(STATUS "Cannot compile cairo because pthread library not found")
    SET(CAIRO_FOUND 0)
  ENDIF(NOT CMAKE_USE_PTHREADS_INIT)  
ENDIF(NOT WIN32)

INCLUDE(CheckSymbolExists)

IF(CAIRO_FOUND)
  IF(WIN32 OR X11_FOUND)    
    
    # Module definition
    ADD_TORCH_PACKAGE(lcairo "${src}" "${luasrc}" "Graphics")
    
    # Readline for callback
    IF(READLINE_FOUND)
      SET(CMAKE_REQUIRED_INCLUDES ${READLINE_INCLUDE_DIR})
      SET(CMAKE_REQUIRED_LIBRARIES ${READLINE_LIBRARIES})
      CHECK_SYMBOL_EXISTS("rl_event_hook" "stdio.h;readline/readline.h" LCAIRO_USE_READLINE)
      IF(LCAIRO_USE_READLINE)
        ADD_DEFINITIONS("-DLCAIRO_USE_READLINE=1")
        INCLUDE_DIRECTORIES(${READLINE_INCLUDE_DIR})
        TARGET_LINK_LIBRARIES(lcairo ${READLINE_LIBRARIES})
      ENDIF(LCAIRO_USE_READLINE)
    ENDIF(READLINE_FOUND)
    
    IF(WIN32 OR APPLE)
      ADD_DEFINITIONS("-DCAIRO_HAS_PDF_SURFACE=1")
      ADD_DEFINITIONS("-DCAIRO_HAS_PS_SURFACE=1")
      ADD_DEFINITIONS("-DCAIRO_HAS_SVG_SURFACE=1")
    ELSE(WIN32 OR APPLE)
      # Linux: some cairo installs don't have all of these
      IF(CAIRO_cairo_LIBRARY)
        # PDF, PS and SVG support
        CHECK_LIBRARY_EXISTS(${CAIRO_cairo_LIBRARY} cairo_pdf_surface_create "" CAIRO_HAS_PDF_SURFACE)
        IF(CAIRO_HAS_PDF_SURFACE)
          ADD_DEFINITIONS("-DCAIRO_HAS_PDF_SURFACE=1")
        ENDIF(CAIRO_HAS_PDF_SURFACE)
        
        CHECK_LIBRARY_EXISTS(${CAIRO_cairo_LIBRARY} cairo_ps_surface_create "" CAIRO_HAS_PS_SURFACE)
        IF(CAIRO_HAS_PS_SURFACE)
          ADD_DEFINITIONS("-DCAIRO_HAS_PS_SURFACE=1")
        ENDIF(CAIRO_HAS_PS_SURFACE)
        
        CHECK_LIBRARY_EXISTS(${CAIRO_cairo_LIBRARY} cairo_svg_surface_create "" CAIRO_HAS_SVG_SURFACE)
        IF(CAIRO_HAS_SVG_SURFACE)
          ADD_DEFINITIONS("-DCAIRO_HAS_SVG_SURFACE=1")
        ENDIF(CAIRO_HAS_SVG_SURFACE)
      ENDIF(CAIRO_cairo_LIBRARY)
      
    ENDIF(WIN32 OR APPLE)
    
    # X11
    IF(NOT WIN32)
      INCLUDE_DIRECTORIES(${X11_INCLUDE_DIR})
      TARGET_LINK_LIBRARIES(lcairo ${CMAKE_THREAD_LIBS_INIT} ${X11_LIBRARIES})
    ENDIF(NOT WIN32)
    
    # Mandatory stuff
    TARGET_LINK_LIBRARIES(lcairo luaT TH)
    TARGET_LINK_LIBRARIES(lcairo ${CAIRO_LIBRARIES})
    INCLUDE_DIRECTORIES(${CAIRO_INCLUDE_DIR})
    
  ELSE(WIN32 OR X11_FOUND)
    MESSAGE(STATUS "Package lcairo not compiled: platform is not Windows and cannot find X11")
  ENDIF(WIN32 OR X11_FOUND)
ELSE(CAIRO_FOUND)
  MESSAGE(STATUS "Package lcairo not compiled")
ENDIF(CAIRO_FOUND)
