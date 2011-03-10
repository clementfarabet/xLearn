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
/*
   Symetrie verticale, horizontale ou centrale

   Michel Couprie  -  Mai 1998 
*/

#include <stdio.h>
#include <stdint.h>
#include <sys/types.h>
#include <stdlib.h>
#include <mcutil.h>
#include <mccodimage.h>
#include <lsym.h>

/* ==================================== */
int32_t lsym(struct xvimage * image1, char mode)
/* ==================================== */
{
  int32_t i, j, k, ip, jp, kp;
  uint8_t *pt1;
  uint8_t tmp;
  int32_t rs, cs, ps, ds, N;

  rs = rowsize(image1);
  cs = colsize(image1);
  ds = depth(image1);
  ps = rs * cs;
  N = ps * ds;
  pt1 = UCHARDATA(image1);
  
  /* ---------------------------------------------------------- */
  /* calcul du resultat */
  /* ---------------------------------------------------------- */

  if (mode != 'z')
  {
    for (k = 0; k < ds; k++)
    {  
      switch (mode)
      {
        case 'v': 
        case 'y': 
          for (j = 0, jp = cs - 1; j < jp; j++, jp--)
            for (i = 0; i < rs; i++)
            {
              tmp = pt1[k * ps + j * rs + i];
              pt1[k * ps + j * rs + i] = pt1[k * ps + jp * rs + i];
              pt1[k * ps + jp * rs + i] = tmp;
            }
          break;
        case 'h': 
        case 'x': 
          for (j = 0; j < cs; j++)
            for (i = 0, ip = rs - 1; i < ip; i++, ip--)
            {
              tmp = pt1[k * ps + j * rs + i];
              pt1[k * ps + j * rs + i] = pt1[k * ps + j * rs + ip];
              pt1[k * ps + j * rs + ip] = tmp;
            }
          break;
        case 'c': 
          for (j = 0, jp = cs - 1; j < jp; j++, jp--)
            for (i = 0; i < rs; i++)
            {
              tmp = pt1[k * ps + j * rs + i];
              pt1[k * ps + j * rs + i] = pt1[k * ps + jp * rs + i];
              pt1[k * ps + jp * rs + i] = tmp;
            }
          for (j = 0; j < cs; j++)
            for (i = 0, ip = rs - 1; i < ip; i++, ip--)
            {
              tmp = pt1[k * ps + j * rs + i];
              pt1[k * ps + j * rs + i] = pt1[k * ps + j * rs + ip];
              pt1[k * ps + j * rs + ip] = tmp;
            }
          break;
      } /* switch (mode) */
    } /* for (k = 0; k < d; k++) */
  }
  else
  {
    for (k = 0, kp = ds - 1; k < kp; k++, kp--)
      for (j = 0; j < cs; j++)
        for (i = 0; i < rs; i++)
        {
          tmp = pt1[k * ps + j * rs + i];
          pt1[k * ps + j * rs + i] = pt1[kp * ps + j * rs + i];
          pt1[kp * ps + j * rs + i] = tmp;
        }

  }
  return 1;
}





