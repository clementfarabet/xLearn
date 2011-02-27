/*------------------------------------------------------------------------
 *
 * Prototypes for the minimal version of the C Wrappers for
 * Library of Image Analysis Routines
 *
 * Hugues Talbot	 4 Jan 2001
 *      
 *-----------------------------------------------------------------------*/

#ifndef LIARWRAP_H
#define LIARWRAP_H

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

typedef enum {
    IM_SINGLE = 0, /* only one component */
    IM_RGB = 1,    /* exactly 3 components */
    IM_SPECTRUM = 2, /* any number of components */
    IM_MULTI = 3, /* those as well */
    IM_BADIMAGE = 4,
    IM_UNSPEC = 5,
    IM_ERROR = 6
} imgtype;



typedef struct image_ {
    int ox; /* X offset */
    int oy; /* Y offset */
    int oz; /* Z offset */
    int ot; /* T offset */
    int nx; /* X dimension */
    int ny; /* Y dimension */
    int nz; /* Z dimension */
    int nt; /* T dimension */
    int nc; /* nb of components */
    imgtype it;
    pixtype   pt;
    void **buff; /* data buffers */
} IMAGE;

typedef unsigned int wsize_t;

/* macros */
#define BUFSIZE     1024
#define DEBUGPROMPT "DEBUG> "
#define ERRORPROMPT "*ERROR*> "
#define EXCEPTPROMPT "**EXCEPTION**> "

/* get */
#define imgetfirstx(i,c) (*i).ox
#define imgetfirsty(i,c) (*i).oy
#define imgetfirstz(i,c) (*i).oz
#define imgetnx(i,c)     (*i).nx
#define imgetny(i,c)     (*i).ny
#define imgetnz(i,c)     (*i).nz
#define imgetnt(i,c)     (*i).nt
#define imgetnumcomp(i)  (*i).nc

#define imgetpixtype(i,c) (*i).pt
#define imgetimgtype(i)   (*i).it
#define imgetpixsize(i,c) pixsize_[(*i).pt]

#define imgetimgtypestr(i) imgtypestr_[(*i).it]
#define imgetpixtypestr(i,c) pixtypestr_[(*i).pt]

#define imgetbuff(i, c)   (*i).buff[(c)]


/* set */
#define imsetfirstx(i,c,v) (*i).ox = (v)
#define imsetfirsty(i,c,v) (*i).oy = (v)
#define imsetfirstz(i,c,v) (*i).oz = (v)
#define imsetfirstt(i,c,v) (*i).ot = (v)
#define imsetnx(i,c,v)     (*i).nx = (v)
#define imsetny(i,c,v)     (*i).ny = (v)
#define imsetnz(i,c,v)     (*i).nz = (v)
#define imsetnt(i,c,v)     (*i).nt = (v)
#define imsetnumcomp(i,v)  (*i).nc = (v)

#define imsetpixtype(i,c,v) (*i).pt = (v)
#define imsetimgtype(i,v)   (*i).it = (v)

#define imsetbuff(i,c,v)    (i).buff[(c)] = (v)

/* others */
#define im_va_start va_start

/* variables */
extern char *imgtypestr_[];
extern char *pixtypestr_[];
extern int   pixsize_[];

/* prototypes */

    IMAGE *imloadtiff(const char *path);

    void LIAREnableDebug(void);
    void LIARDisableDebug(void);
    int LIARdebug(const char * msg,...);
    
#ifdef __cplusplus
}
#endif /* __cplusplus */
    
#endif
