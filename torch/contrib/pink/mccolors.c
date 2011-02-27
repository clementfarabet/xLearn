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
#include <stdio.h>
#include <stdint.h>
#include <sys/types.h>
#include <stdlib.h>
#include <mcutil.h>
#include <mccolors.h>

/* rgb2hls.c - construct 3 pgm from 1 ppm file:
** - basename.H : Hue (int32_t - 0 to 359)
** - basename.L : Luminance (byte)
** - basename.S : Saturation (byte)
**
** Michel Couprie  -  avril 1999
**
** Based on Foley, Van Damm & al: "Computer Graphics", 2nd ed., p. 595
**
** Quelques valeurs de HUE:
**     0  rouge
**    60  jaune
**   120  vert
**   180  cyan
**   240  bleu
**   300  magenta
*/

/* ======================================================================== */
char * colorname(double h)
/* ======================================================================== */
{
  if ((h < 30.0) || (h >= 330.0)) return (char *)CN_RED;
  else if ((h >= 30.0) && (h < 90.0)) return (char *)CN_YELLOW;
  else if ((h >= 90.0) && (h < 150.0)) return (char *)CN_GREEN;
  else if ((h >= 150.0) && (h < 210.0)) return (char *)CN_CYAN;
  else if ((h >= 210.0) && (h < 270.0)) return (char *)CN_BLUE;
  else /* if ((h >= 270.0) && (h < 330.0)) */ return (char *)CN_MAGENTA;
}

/* ======================================================================== */
char * nomcouleur(double h)
/* ======================================================================== */
{
  if ((h < 30.0) || (h >= 330.0)) return (char *)CN_ROUGE;
  else if ((h >= 30.0) && (h < 90.0)) return (char *)CN_JAUNE;
  else if ((h >= 90.0) && (h < 150.0)) return (char *)CN_VERT;
  else if ((h >= 150.0) && (h < 210.0)) return (char *)CN_CYAN;
  else if ((h >= 210.0) && (h < 270.0)) return (char *)CN_BLEU;
  else /* if ((h >= 270.0) && (h < 330.0)) */ return (char *)CN_MAGENTA;
}

/* ======================================================================== */
void rgb2hls(double r, double g, double b, double *h, double *l, double *s)
/* ======================================================================== */
/*
  Given: r, g, b each in [0,1].
  Desired: h in [0,360[, l and s in [0,1]. If s = 0, then h = UNDEFINED = 0.
*/
#define UNDEFINED 0.0
{
  double maxi = mcmax(mcmax(r,g),b); 
  double mini = mcmin(mcmin(r,g),b); 
  double delta;

  *l = (maxi + mini) / 2.0; /* lightness */
  if (maxi == mini)
  {                   /* achromatic case */
    *s = 0.0; *h = UNDEFINED;
  }
  else
  {                   /* chromatic case */
    delta = maxi - mini;
    if (*l <= 0.5)
      *s = delta / (maxi + mini);
    else
      *s = delta / (2 - maxi - mini);
    if (r == maxi) *h = (g - b) / delta;
    else if (g == maxi) *h = 2.0 + (b - r) / delta;
    else if (b == maxi) *h = 4.0 + (r - g) / delta;
    *h = *h * 60.0;
    if (*h < 0.0) *h = *h + 360.0;
  }
}

/* hls2rgb.c - construct 1 ppm from 3 pgm files:
** - basename.H : Hue (int32_t - 0 to 359)
** - basename.L : Luminance (byte)
** - basename.S : Saturation (byte)
**
** Michel Couprie  -  avril 1999
**
** Based on Foley, Van Damm & al: "Computer Graphics", 2nd ed., p. 595
**
** Quelques valeurs de HUE:
**     0  rouge
**    60  jaune
**   120  vert
**   180  cyan
**   240  bleu
**   300  magenta
*/

/* ======================================================================== */
static double value(double n1, double n2, double hue)
/* ======================================================================== */
{
  if (hue >= 360.0) hue -= 360.0;
  else if (hue < 0.0) hue += 360.0;
  if (hue < 60.0) return n1 + (n2 - n1) * hue / 60.0;
  if (hue < 180.0) return n2;
  if (hue < 240.0) return n1 + (n2 - n1) * (240 - hue) / 60.0;
  return n1;
}

/* ======================================================================== */
void hls2rgb(double *r, double *g, double *b, double h, double l, double s)
/* ======================================================================== */
/*
  Given: h in [0,360[, l and s in [0,1]. If s = 0, then h = UNDEFINED = 0.
  Desired: r, g, b each in [0,1].
*/
#define UNDEFINED 0.0
{
  double m1, m2;
  if (l <= 0.5)
    m2 = l * (1.0 + s);
  else
    m2 = l + s - l * s;
  m1 = 2 * l - m2;
  if (s == 0)
    *r = *g = *b = l;
  else
  {
    *r = value(m1, m2, h + 120.0);
    *g = value(m1, m2, h);
    *b = value(m1, m2, h - 120.0);
  }
}

/* ======================================================================== */
int32_t huedistance(int32_t h1, int32_t h2)
/* ======================================================================== */
{
  int32_t k, h = h1 - h2;
  h = mcabs(h);
  k = 360 - h;
  h = mcmin(h,k);
  return h;
} //huedistance()
