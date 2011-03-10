/*Copyright ESIEE (2009) 

Author :
Camille Couprie (c.couprie@esiee.fr)

Contributors : 
Hugues Talbot (h.talbot@esiee.fr)
Leo Grady (leo.grady@siemens.com)
Laurent Najman (l.najman@esiee.fr)

This software contains some image processing algorithms whose purpose is to be
used primarily for research.

This software is governed by the CeCILL license under French law and
abiding by the rules of distribution of free software.  You can  use, 
modify and/ or redistribute the software under the terms of the CeCILL
license as circulated by CEA, CNRS and INRIA at the following URL
"http://www.cecill.info". 

As a counterpart to the access to the source code and  rights to copy,
modify and redistribute granted by the license, users are provided only
with a limited warranty  and the software's author,  the holder of the
economic rights,  and the successive licensors  have only  limited
liability. 

In this respect, the user's attention is drawn to the risks associated
with loading,  using,  modifying and/or developing or reproducing the
software by the user in light of its specific status of free software,
that may mean  that it is complicated to manipulate,  and  that  also
therefore means  that it is reserved for developers  and  experienced
professionals having in-depth computer knowledge. Users are therefore
encouraged to load and test the software's suitability as regards their
requirements in conditions enabling the security of their systems and/or 
data to be ensured and,  more generally, to use and operate it in the 
same conditions as regards security. 

The fact that you are presently reading this means that you have had
knowledge of the CeCILL license and that you accept its terms.
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <mcimage.h>
#include <mccodimage.h>
#include <larith.h>
#include <image_toolbox.h>
#include <cccodimage.h>


/* ================================================================ */
void printweights(uint32_t * weights, /* weights value on the edges */ 
		  int rs, /* row size */
		  int cs) /*col size */
/* ================================================================ */
{ 
  int V = (cs-1)*rs;
  int i,l,j,k;
  k=0;
  int nb_edges = V+ (rs-1)*cs;
  for (i = V;i<nb_edges;i=i+(rs-1))
    {
  for (j = k;j<k+cs;j++) printf("  |      ");
      printf("\n   ");
      for (l = i ; l<i+rs-1 ; l++ )
	{
	  printf(" -- %ld --", weights[l]);
	}
      printf("\n");
        for (j = k;j<k+cs;j++) printf("  |      ");
      printf("\n");
      if (k>=V) break;
      for (j = k;j<k+cs;j++)
	{
	  printf("%ld       ", weights[j]);
	}
      printf("\n");
      k=j; 
      
    }
}


/* ===================================================================================================== */
void overlay(int algo,                 /* algorithm index (1, 2, or 3)*/
	     struct xvimage * image_r, /* red componant of the image */
	     struct xvimage * image_v, /* green componant of the image */
	     struct xvimage * image_b, /* blue componant of the image */
	     struct xvimage * output,  /* segmentation mask image */
	     bool color,               /* true if the image is in color, false if image is in grey levels*/
	     char * name)              /* name of the overlay image */
/* ===================================================================================================== */
/* from an input segmentation mask and the original 2D image, prints the overlay in "overlay.pgm" */
{
  struct xvimage * tmp_r;
  struct xvimage * tmp_g;
  struct xvimage * tmp_b;
  int i, rs, cs;
  rs = rowsize(image_r);
  cs = colsize(image_r);
 
  tmp_r = copyimage(image_r);
  if (color)
    {
      tmp_g = copyimage(image_v);
      tmp_b = copyimage(image_b);
    }
  else {
    tmp_g = copyimage(image_r);
    tmp_b = copyimage(image_r);}
  
  if (algo == 2) // red
    {
      image_r = boundary(output);
      lmax(image_r,tmp_r);
      image_v = boundary(output);
      linverse(image_v);
      lmin(image_v,tmp_g);
      image_b = boundary(output);
      linverse(image_b);
      lmin(image_b,tmp_b);
      
      writergbimage(image_r, image_v, image_b,  name);
      freeimage(image_r);
      freeimage(image_v);
      freeimage(image_b);
    }

  else if (algo == 3) // orange
    { 
      struct xvimage * nimage;
      image_r = boundary(output);
      nimage = mult_image_value(image_r, 1);
      lmax(nimage,tmp_r);
      unsigned char *utmp_b = UCHARDATA(tmp_b);
      unsigned char *utmp_g = UCHARDATA(tmp_g);
      image_b = boundary(output);
      unsigned char *uimage_b = UCHARDATA(image_b);
      for (i=0;i<rs*cs;i++)
	if(uimage_b[i]==255) {uimage_b[i] = 60; utmp_g[i]=150;}
	else uimage_b[i] = utmp_b[i];
      writergbimage(nimage, tmp_g, image_b, name);
      freeimage(image_r);
      freeimage(image_b);
      freeimage(nimage);
      
    }
  else if (algo == 1) // pink
    { 
      struct xvimage * nimage;
      image_r = boundary(output);
      nimage = mult_image_value(image_r, 1);
      lmax(nimage,tmp_r);
      unsigned char *utmp_b = UCHARDATA(tmp_b);
      unsigned char *utmp_g = UCHARDATA(tmp_g);
      image_b = boundary(output);
      unsigned char *uimage_b = UCHARDATA(image_b);
      for (i=0;i<rs*cs;i++)
	if(uimage_b[i]==255) {uimage_b[i] = 200; utmp_g[i]=0;}
	else uimage_b[i] = utmp_b[i];
      writergbimage(nimage, tmp_g, image_b,  name);
      freeimage(nimage);
      freeimage(image_r);
      freeimage(image_b);
    }

  freeimage(tmp_r);
  freeimage(tmp_g);
  freeimage(tmp_b);
    
}
