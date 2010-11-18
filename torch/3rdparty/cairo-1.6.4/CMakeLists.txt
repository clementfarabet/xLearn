INCLUDE(CheckIncludeFile)
INCLUDE(CheckFunctionExists)
INCLUDE(CheckTypeSize)

CHECK_FUNCTION_EXISTS(ctime_r HAVE_CTIME_R)
CHECK_INCLUDE_FILE(dlfcn.h HAVE_DLFCN_H)
CHECK_FUNCTION_EXISTS(drand48 HAVE_DRAND48)

# freetype
CHECK_FUNCTION_EXISTS(FcFini HAVE_FCFINI)
# /* Define to 1 if you have the `FcFini' function. */
#cmakedefine HAVE_FCFINI
# /* FT_Bitmap_Size structure includes y_ppem field */
#cmakedefine HAVE_FT_BITMAP_SIZE_Y_PPEM
#/* Define to 1 if you have the `FT_GlyphSlot_Embolden' function. */
##cmakedefine HAVE_FT_GLYPHSLOT_EMBOLDEN
#/* Define to 1 if you have the `FT_Load_Sfnt_Table' function. */
##cmakedefine HAVE_FT_LOAD_SFNT_TABLE

#/* Whether you have gcov */
##cmakedefine HAVE_GCOV

CHECK_INCLUDE_FILE(inttypes.h HAVE_INTTYPES_H)
CHECK_INCLUDE_FILE(libgen.h HAVE_LIBGEN_H)
CHECK_INCLUDE_FILE(memory.h HAVE_MEMORY_H)

#/* Define to 1 if you have the `poppler_page_render_to_pixbuf' function. */
##cmakedefine HAVE_POPPLER_PAGE_RENDER_TO_PIXBUF

CHECK_INCLUDE_FILE(pthread.h HAVE_PTHREAD_H)
#/* Define to 1 if you have the <pthread.h> header file. */
##cmakedefine HAVE_PTHREAD_H

#/* Define to 1 if you have the `rsvg_pixbuf_from_file' function. */
#cmakedefine HAVE_RSVG_PIXBUF_FROM_FILE

CHECK_INCLUDE_FILE(sched.h HAVE_SCHED_H)
CHECK_FUNCTION_EXISTS(sched_getaffinity HAVE_SCHED_GETAFFINITY)

CHECK_INCLUDE_FILE(signal.h HAVE_SIGNAL_H)
CHECK_INCLUDE_FILE(stdint.h HAVE_STDINT_H)
CHECK_INCLUDE_FILE(stdlib.h HAVE_STDLIB_H)
CHECK_INCLUDE_FILE(strings.h HAVE_STRINGS_H)
CHECK_INCLUDE_FILE(string.h HAVE_STRING_H)
CHECK_INCLUDE_FILE(sys/int_types.h HAVE_SYS_INT_TYPES_H)
CHECK_INCLUDE_FILE(sys/stat.h HAVE_SYS_STAT_H)
CHECK_INCLUDE_FILE(sys/types.h HAVE_SYS_TYPES_H)

CHECK_TYPE_SIZE(uint128_t HAVE_UINT128_T)
CHECK_TYPE_SIZE(uint64_t HAVE_UINT64_T)

CHECK_INCLUDE_FILE(unistd.h HAVE_UNISTD_H)
CHECK_FUNCTION_EXISTS(vasnprintf HAVE_VASNPRINTF)
CHECK_INCLUDE_FILE(windows.h HAVE_WINDOWS_H)

SET(CAIRO_PACKAGE "cairo")
SET(CAIRO_VERSION "${CAIRO_VERSION_MAJOR}.${CAIRO_VERSION_MINOR}.${CAIRO_VERSION_MICRO}")
SET(CAIRO_PACKAGE_BUGREPORT "http://bugs.freedesktop.org/enter_bug.cgi?product=cairo")
SET(CAIRO_PACKAGE_NAME "cairo")
SET(CAIRO_PACKAGE_STRING "cairo ${CAIRO_VERSION}")
SET(CAIRO_PACKAGE_TARNAME "cairo")
SET(CAIRO_PACKAGE_VERSION ${CAIRO_VERSION})


INCLUDE(${3RDPARTY_SOURCE_DIR}/cmake/CMakeTestInline.cmake)

#/* Define to 1 if you have the ANSI C header files. */
#undef STDC_HEADERS

ADD_DEFINITIONS(-DHAVE_CONFIG_H)
CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config.h)
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})

ADD_SUBDIRECTORY(src)
