/*
Copyright ESIEE (2009) 

m.couprie@esiee.fr

This software is an image processing library whose purpose is to be
used primarily for research and teaching.

This software is governed by the CeCILL  license under French law and
abiding by the rules of distribution of free software. You can  use, 
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

/*************************************************
    Quelques fonctions pour la generation de postscript
    Michel Couprie
**************************************************/

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* ========================================== */
void EPSHeader(FILE *fd, double figure_width, double figure_height, double line_width, int font_size )
/* ========================================== */
{
  fprintf(fd, "%%!PS-Adobe-2.0 EPSF-2.0\n");
  fprintf(fd, "%%%%BoundingBox: 0 0 %g %g\n", figure_width, figure_height);
  fprintf(fd, "%g setlinewidth\n", line_width);
  fprintf(fd, "/corps %d def\n", font_size);
  fprintf(fd, "/Fn /Roman findfont corps scalefont def\n");
  fprintf(fd, "/setn {	/CurrentFont Fn def } def\n");
  fprintf(fd, "setn\n");
  fprintf(fd, "/o { CurrentFont setfont show } bind def\n");
} /* EPSHeader() */

/* ========================================== */
void PSHeader(FILE *fd, double figure_width, double figure_height, double line_width, int font_size )
/* ========================================== */
{
  fprintf(fd, "%%!\n");
  fprintf(fd, "%g setlinewidth\n", line_width);
  fprintf(fd, "/corps %d def\n", font_size);
  fprintf(fd, "/Fn /Courier findfont corps scalefont def\n");
  fprintf(fd, "/setn {	/CurrentFont Fn def } def\n");
  fprintf(fd, "setn\n");
  fprintf(fd, "/o { CurrentFont setfont show } bind def\n");
} /* PSHeader() */

/*========================================*/
void PSMove (FILE *fd, double x, double y) 
/*========================================*/
{
  int xx, yy;
  xx = (int)x;
  yy = (int)y;
  fprintf(fd, "%d %d moveto ", xx, yy);
} /* PSMove() */

/*========================================*/
void PSDraw (FILE *fd, double x, double y)
/*========================================*/
{
  int xx, yy;
  xx = (int)x;
  yy = (int)y;
  fprintf(fd, "%d %d lineto ", xx, yy);
} /* PSDraw() */


/*========================================*/
void PSLine (FILE *fd, double xd, double yd, double xf, double yf)
/*========================================*/
{
  fprintf(fd, "newpath ");
  PSMove(fd, xd, yd);
  PSDraw(fd, xf, yf);
  fprintf(fd, "stroke\n");
} /* PSLine() */

/*========================================*/
void PSDrawcircle (FILE *fd, double x, double y, double r)
/*========================================*/
{
  int xx, yy, rr;
  xx = (int)x;
  yy = (int)y;
  rr = (int)r;
  fprintf(fd, "newpath ");
  fprintf(fd, "%d %d %d 0 360 arc ", xx, yy, rr);
  fprintf(fd, "stroke\n");
} /* PSDrawcircle() */

/*========================================*/
void PSDrawdisc (FILE *fd, double x, double y, double r)
/*========================================*/
{
  int xx, yy, rr;
  xx = (int)x;
  yy = (int)y;
  rr = (int)r;
  fprintf(fd, "newpath ");
  fprintf(fd, "%d %d %d 0 360 arc ", xx, yy, rr);
  fprintf(fd, "fill stroke\n");
} /* PSDrawdisc() */

/*========================================*/
void PSSetColor (FILE *fd, double col)
/*========================================*/
/* change la couleur courante */ 
{
  fprintf(fd,"%g setgray\n", col);
} // PSSetColor()

/*========================================*/
void PSSetLineWidth (FILE *fd, double w)
/*========================================*/
/* change la largeur de ligne */ 
{
  fprintf(fd, "%g setlinewidth\n", w);
} // PSSetLineWidth()

/*========================================*/
void PSDrawRect (FILE *fd, double x, double y, double w, double h)
/*========================================*/
/* dessine un rectangle au point (x,y), de largeur w, de hauteur h */ 
{
  fprintf(fd,"newpath ");
  PSMove(fd, x, y);
  PSDraw(fd, x+w, y);
  PSDraw(fd, x+w, y+h);
  PSDraw(fd, x, y+h);
  PSDraw(fd, x, y);
  fprintf(fd,"fill stroke\n");
} /* PSDrawRect() */

/*========================================*/
void PSString (FILE *fd, double x, double y, char *s)
/*========================================*/
{
  int xx, yy;
  xx = (int)x;
  yy = (int)y;
  fprintf(fd, "newpath ");
  fprintf(fd, "%d %d moveto ", xx, yy);
  fprintf(fd, "(%s)o ", s);
  fprintf(fd, "stroke\n");
} /* PSString() */

/* ========================================== */
void PSFooter(FILE *fd)
/* ========================================== */
{
  fprintf(fd, "showpage\n");
} /* PSFooter() */
