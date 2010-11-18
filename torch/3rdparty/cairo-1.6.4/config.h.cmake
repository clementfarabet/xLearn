/* config.h.in.  Generated from configure.in by autoheader.  */

/* whether Cairo needs memory barriers around atomic ops */
#undef CAIRO_ATOMIC_OP_NEEDS_MEMORY_BARRIER

/* define if glitz backend can be tested against agl */
#undef CAIRO_CAN_TEST_GLITZ_AGL_SURFACE

/* define if glitz backend can be tested against egl */
#undef CAIRO_CAN_TEST_GLITZ_EGL_SURFACE

/* define if glitz backend can be tested against glx */
#undef CAIRO_CAN_TEST_GLITZ_GLX_SURFACE

/* define if glitz backend can be tested against wgl */
#undef CAIRO_CAN_TEST_GLITZ_WGL_SURFACE

/* Define to 1 if the PDF backend can be tested (need poppler and other
   dependencies for pdf2png) */
#undef CAIRO_CAN_TEST_PDF_SURFACE

/* Define to 1 if the PS backend can be tested (needs ghostscript) */
#undef CAIRO_CAN_TEST_PS_SURFACE

/* Define to 1 if the SVG backend can be tested */
#undef CAIRO_CAN_TEST_SVG_SURFACE

/* Define to 1 if the Win32 Printing backend can be tested (needs ghostscript)
   */
#undef CAIRO_CAN_TEST_WIN32_PRINTING_SURFACE

/* Enable if your compiler supports the Intel __sync_* atomic primitives */
#undef CAIRO_HAS_INTEL_ATOMIC_PRIMITIVES

/* define in the extra test surface have been built into cairo for the test
   suite */
#undef CAIRO_HAS_TEST_SURFACES

/* Define to 1 to disable certain code paths that rely heavily on double
   precision floating-point calculation */
#undef DISABLE_SOME_FLOATING_POINT

/* Define to 1 if your system stores words within floats with the most
   significant word first */
#undef FLOAT_WORDS_BIGENDIAN

/* Define to 1 if you have the `ctime_r' function. */
#cmakedefine HAVE_CTIME_R @HAVE_CTIME_R@

/* Define to 1 if you have the <dlfcn.h> header file. */
#cmakedefine HAVE_DLFCN_H @HAVE_DLFCN_H@

/* Define to 1 if you have the `drand48' function. */
#cmakedefine HAVE_DRAND48 @HAVE_DRAND48@

/* Define to 1 if you have the `FcFini' function. */
#cmakedefine HAVE_FCFINI @HAVE_FCFINI@

/* FT_Bitmap_Size structure includes y_ppem field */
#cmakedefine HAVE_FT_BITMAP_SIZE_Y_PPEM @HAVE_FT_BITMAP_SIZE_Y_PPEM@

/* Define to 1 if you have the `FT_GlyphSlot_Embolden' function. */
#cmakedefine HAVE_FT_GLYPHSLOT_EMBOLDEN @HAVE_FT_GLYPHSLOT_EMBOLDEN@

/* Define to 1 if you have the `FT_Load_Sfnt_Table' function. */
#cmakedefine HAVE_FT_LOAD_SFNT_TABLE @HAVE_FT_LOAD_SFNT_TABLE@

/* Whether you have gcov */
#cmakedefine HAVE_GCOV @HAVE_GCOV@

/* Define to 1 if you have the <inttypes.h> header file. */
#cmakedefine HAVE_INTTYPES_H @HAVE_INTTYPES_H@

/* Define to 1 if you have the <libgen.h> header file. */
#cmakedefine HAVE_LIBGEN_H @HAVE_LIBGEN_H@

/* Define to 1 if you have the <memory.h> header file. */
#cmakedefine HAVE_MEMORY_H @HAVE_MEMORY_H@

/* Define to 1 if you have the `poppler_page_render_to_pixbuf' function. */
#cmakedefine HAVE_POPPLER_PAGE_RENDER_TO_PIXBUF @HAVE_POPPLER_PAGE_RENDER_TO_PIXBUF@

/* Define to 1 if you have the <pthread.h> header file. */
#cmakedefine HAVE_PTHREAD_H @HAVE_PTHREAD_H@

/* Define to 1 if you have the `rsvg_pixbuf_from_file' function. */
#cmakedefine HAVE_RSVG_PIXBUF_FROM_FILE @HAVE_RSVG_PIXBUF_FROM_FILE@

/* Define to 1 if you have Linux compatible sched_getaffinity */
#cmakedefine HAVE_SCHED_GETAFFINITY @HAVE_SCHED_GETAFFINITY@

/* Define to 1 if you have the <sched.h> header file. */
#cmakedefine HAVE_SCHED_H @HAVE_SCHED_H@

/* Define to 1 if you have the <signal.h> header file. */
#cmakedefine HAVE_SIGNAL_H @HAVE_SIGNAL_H@

/* Define to 1 if you have the <stdint.h> header file. */
#cmakedefine HAVE_STDINT_H @HAVE_STDINT_H@

/* Define to 1 if you have the <stdlib.h> header file. */
#cmakedefine HAVE_STDLIB_H @HAVE_STDLIB_H@

/* Define to 1 if you have the <strings.h> header file. */
#cmakedefine HAVE_STRINGS_H @HAVE_STRINGS_H@

/* Define to 1 if you have the <string.h> header file. */
#cmakedefine HAVE_STRING_H @HAVE_STRING_H@

/* Define to 1 if you have the <sys/int_types.h> header file. */
#cmakedefine HAVE_SYS_INT_TYPES_H @HAVE_SYS_INT_TYPES_H@

/* Define to 1 if you have the <sys/stat.h> header file. */
#cmakedefine HAVE_SYS_STAT_H @HAVE_SYS_STAT_H@

/* Define to 1 if you have the <sys/types.h> header file. */
#cmakedefine HAVE_SYS_TYPES_H @HAVE_SYS_TYPES_H@

/* Define to 1 if the system has the type `uint128_t'. */
#cmakedefine HAVE_UINT128_T @HAVE_UINT128_T@

/* Define to 1 if the system has the type `uint64_t'. */
#cmakedefine HAVE_UINT64_T @HAVE_UINT64_T@

/* Define to 1 if you have the <unistd.h> header file. */
#cmakedefine HAVE_UNISTD_H @HAVE_UNISTD_H@

/* Define to 1 if you have the `vasnprintf' function. */
#cmakedefine HAVE_VASNPRINTF @HAVE_VASNPRINTF@

/* Define to 1 if you have the <windows.h> header file. */
#cmakedefine HAVE_WINDOWS_H @HAVE_WINDOWS_H@

/* Name of package */
#define PACKAGE "@CAIRO_PACKAGE@"

/* Define to the address where bug reports for this package should be sent. */
#define PACKAGE_BUGREPORT "@CAIRO_PACKAGE_BUGREPORT@"

/* Define to the full name of this package. */
#define CAIRO_PACKAGE_NAME "@CAIRO_PACKAGE_NAME@"

/* Define to the full name and version of this package. */
#define PACKAGE_STRING "@CAIRO_PACKAGE_STRING@"

/* Define to the one symbol short name of this package. */
#define PACKAGE_TARNAME "@CAIRO_PACKAGE_TARNAME@"

/* Define to the version of this package. */
#define PACKAGE_VERSION "@CAIRO_PACKAGE_VERSION@"

/* Define to 1 if you have the ANSI C header files. */
#cmakedefine STDC_HEADERS @STDC_HEADERS@

/* Version number of package */
#define VERSION "@CAIRO_VERSION@"

/* Define to the value your compiler uses to support the warn-unused-result
   attribute */
#cmakedefine WARN_UNUSED_RESULT

/* Define to 1 if your processor stores words with the most significant byte
   first (like Motorola and SPARC, unlike Intel and VAX). */
#undef WORDS_BIGENDIAN


/* Deal with multiple architecture compiles on Mac OS X */
#ifdef __APPLE_CC__
#ifdef __BIG_ENDIAN__
#define WORDS_BIGENDIAN 1
#define FLOAT_WORDS_BIGENDIAN 1
#else
#undef WORDS_BIGENDIAN
#undef FLOAT_WORDS_BIGENDIAN
#endif
#endif


/* Define to 1 if the X Window System is missing or not being used. */
#define X_DISPLAY_MISSING 1

/* Define to `__inline__' or `__inline' if that's what the C compiler
   calls it, or to nothing if 'inline' is not supported under any name.  */
#ifndef __cplusplus
/* Cairo config bug */
#ifdef _MSC_VER
#undef inline
#endif
#define inline @C_INLINE@
#endif
