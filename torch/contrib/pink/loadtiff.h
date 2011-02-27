/*------------------------------------------------------------------------
 *
 *
 * Decodes the TIFF format
 *
 *      
 *-----------------------------------------------------------------------*/

#ifndef LOADTIFF_H
#define LOADTIFF_H

#include <tiffio.h>

int  getTiffType(int pi, int sf, int spp, int bps, pixtype *pixeltype, imgtype *imagetype);
int  load_tiff(const char *fname,	 /* file name  */
	       int        imageindex,     /* image index in the file to load */
	       int        start[3],	 /* dimensions start */
	       int        end[3],         /* dimensions end */
	       int        *pi,            /* photometric interpretation */
	       int        *sf,            /* storage format */
	       int        *spp,	         /* samples per pixel */
	       int        *bps,           /* bits per pixel */
	       unsigned short      *colourmap[3],  /* RGB lut */
	       long       *ncolours,      /* nb of colours in the lut */      
	       void       **inbuffp);	 /* data buffer for this image */

#endif /* defined(LOADTIFF_H) */
