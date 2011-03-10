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
Librairie mctopo : 

Calcul des nombres topologiques en 2D et en 2,5D

Michel Couprie 1996

17 mars 1998: fonction nbtopoh
18 mars 1998: ajout et test des tableaux Comp4Tab et Comp8Tab
23 mars 1998: fonctions top4 et top8 pour les images binaires
28 mars 1998: fonctions t4, t4b, t8, t8b pour les images binaires
24 octobre 1998: detection des points "ridge" (d'apres Arcelli)
23 avril 1999: nouvelle definition des points lambda-simples
02 juin 1999: fonction deltaNm et deltaNp, avec N = 4,8
29 juin 2001: typetopobin et typetopobin8
Update sep. 2009 : ajout des tests is_on_frame()
*/

#include <sys/types.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <mctopo.h>
#include <mcimage.h>
#include <mccodimage.h>
#include <mcutil.h>

/* 
   Tableau des nombres topologiques - en global pour etre efficace.

   Ce tableau contient les nombres topologiques T4bar et T8, pour toutes les
     configurations de 8-voisinage possibles.
   Si on inverse logiquement (~) l'index dans ce tableau, on obtient respectivement
     T4 et T8bar.

   Les configurations sont codees sur 8 bits, selon l'ordre suivant:

		3	2	1			
		4	X	0
		5	6	7

   Le mot de 8 bits provenant du codage d'une configuration de voisinage sert
     a indexer le tableau TopoTab pour recuperer les nombres topologiques.
*/
static int32_t TopoTab[256][2] = 
{
  {1,0},  {1,1},  {1,1},  {1,1},  {1,1},  {1,1},  {1,1},  {1,1},  /*  0 -  7 */
  {1,1},  {2,2},  {2,2},  {2,2},  {1,1},  {1,1},  {1,1},  {1,1},  /*  8 - 1f */
  {1,1},  {2,2},  {2,2},  {2,2},  {1,1},  {1,1},  {1,1},  {1,1},  /* 10 - 17 */
  {1,1},  {2,2},  {2,2},  {2,2},  {1,1},  {1,1},  {1,1},  {1,1},  /* 18 - 1f */
  {1,1},  {2,2},  {2,2},  {2,2},  {2,2},  {2,2},  {2,2},  {2,2},  /* 20 - 27 */
  {2,2},  {3,3},  {3,3},  {3,3},  {2,2},  {2,2},  {2,2},  {2,2},  /* 28 - 2f */
  {1,1},  {2,2},  {2,2},  {2,2},  {1,1},  {1,1},  {1,1},  {1,1},  /* 30 - 37 */
  {1,1},  {2,2},  {2,2},  {2,2},  {1,1},  {1,1},  {1,1},  {1,1},  /* 38 - 3f */
  {1,1},  {1,1},  {2,2},  {1,1},  {2,2},  {1,1},  {2,2},  {1,1},  /* 40 - 47 */
  {2,2},  {2,2},  {3,3},  {2,2},  {2,2},  {1,1},  {2,2},  {1,1},  /* 48 - 4f */
  {1,1},  {1,1},  {2,2},  {1,1},  {1,1},  {0,1},  {1,1},  {0,1},  /* 50 - 57 */
  {1,1},  {1,1},  {2,2},  {1,1},  {1,1},  {0,1},  {1,1},  {0,1},  /* 58 - 5f */
  {1,1},  {1,1},  {2,2},  {1,1},  {2,2},  {1,1},  {2,2},  {1,1},  /* 60 - 67 */
  {2,2},  {2,2},  {3,3},  {2,2},  {2,2},  {1,1},  {2,2},  {1,1},  /* 68 - 6f */
  {1,1},  {1,1},  {2,2},  {1,1},  {1,1},  {0,1},  {1,1},  {0,1},  /* 70 - 77 */
  {1,1},  {1,1},  {2,2},  {1,1},  {1,1},  {0,1},  {1,1},  {0,1},  /* 78 - 7f */
  {1,1},  {1,1},  {2,2},  {1,1},  {2,2},  {1,1},  {2,2},  {1,1},  /* 80 - 87 */
  {2,2},  {2,2},  {3,3},  {2,2},  {2,2},  {1,1},  {2,2},  {1,1},  /* 88 - 8f */
  {2,2},  {2,2},  {3,3},  {2,2},  {2,2},  {1,1},  {2,2},  {1,1},  /* 90 - 97 */
  {2,2},  {2,2},  {3,3},  {2,2},  {2,2},  {1,1},  {2,2},  {1,1},  /* 98 - 9f */
  {2,2},  {2,2},  {3,3},  {2,2},  {3,3},  {2,2},  {3,3},  {2,2},  /* a0 - a7 */
  {3,3},  {3,3},  {4,4},  {3,3},  {3,3},  {2,2},  {3,3},  {2,2},  /* a8 - af */
  {2,2},  {2,2},  {3,3},  {2,2},  {2,2},  {1,1},  {2,2},  {1,1},  /* b0 - b7 */
  {2,2},  {2,2},  {3,3},  {2,2},  {2,2},  {1,1},  {2,2},  {1,1},  /* b8 - bf */
  {1,1},  {1,1},  {2,2},  {1,1},  {2,2},  {1,1},  {2,2},  {1,1},  /* c0 - c7 */
  {2,2},  {2,2},  {3,3},  {2,2},  {2,2},  {1,1},  {2,2},  {1,1},  /* c8 - cf */
  {1,1},  {1,1},  {2,2},  {1,1},  {1,1},  {0,1},  {1,1},  {0,1},  /* d0 - d7 */
  {1,1},  {1,1},  {2,2},  {1,1},  {1,1},  {0,1},  {1,1},  {0,1},  /* d8 - df */
  {1,1},  {1,1},  {2,2},  {1,1},  {2,2},  {1,1},  {2,2},  {1,1},  /* e0 - e7 */
  {2,2},  {2,2},  {3,3},  {2,2},  {2,2},  {1,1},  {2,2},  {1,1},  /* e8 - ef */
  {1,1},  {1,1},  {2,2},  {1,1},  {1,1},  {0,1},  {1,1},  {0,1},  /* f0 - f7 */
  {1,1},  {1,1},  {2,2},  {1,1},  {1,1},  {0,1},  {1,1},  {0,1}   /* f8 - ff */
};

/* 
   Tableau des composantes 4-connexes 4-adjacentes au point central (C4C4) 
     - en global pour etre efficace.

   Ce tableau contient les codes des C4C4, pour toutes les
   configurations de 8-voisinage possibles.
*/

static int32_t Comp4Tab[256][4] = 
{
  {0x0,0x0,0x0,0x0}, {0x1,0x0,0x0,0x0}, {0x0,0x0,0x0,0x0}, {0x3,0x0,0x0,0x0},
  {0x4,0x0,0x0,0x0}, {0x4,0x1,0x0,0x0}, {0x6,0x0,0x0,0x0}, {0x7,0x0,0x0,0x0},
  {0x0,0x0,0x0,0x0}, {0x1,0x0,0x0,0x0}, {0x0,0x0,0x0,0x0}, {0x3,0x0,0x0,0x0},
  {0xc,0x0,0x0,0x0}, {0xc,0x1,0x0,0x0}, {0xe,0x0,0x0,0x0}, {0xf,0x0,0x0,0x0},
  {0x10,0x0,0x0,0x0}, {0x10,0x1,0x0,0x0}, {0x10,0x0,0x0,0x0}, {0x3,0x10,0x0,0x0},
  {0x4,0x10,0x0,0x0}, {0x4,0x10,0x1,0x0}, {0x6,0x10,0x0,0x0}, {0x7,0x10,0x0,0x0},
  {0x18,0x0,0x0,0x0}, {0x18,0x1,0x0,0x0}, {0x18,0x0,0x0,0x0}, {0x18,0x3,0x0,0x0},
  {0x1c,0x0,0x0,0x0}, {0x1c,0x1,0x0,0x0}, {0x1e,0x0,0x0,0x0}, {0x1f,0x0,0x0,0x0},
  {0x0,0x0,0x0,0x0}, {0x1,0x0,0x0,0x0}, {0x0,0x0,0x0,0x0}, {0x3,0x0,0x0,0x0},
  {0x4,0x0,0x0,0x0}, {0x4,0x1,0x0,0x0}, {0x6,0x0,0x0,0x0}, {0x7,0x0,0x0,0x0},
  {0x0,0x0,0x0,0x0}, {0x1,0x0,0x0,0x0}, {0x0,0x0,0x0,0x0}, {0x3,0x0,0x0,0x0},
  {0xc,0x0,0x0,0x0}, {0xc,0x1,0x0,0x0}, {0xe,0x0,0x0,0x0}, {0xf,0x0,0x0,0x0},
  {0x30,0x0,0x0,0x0}, {0x30,0x1,0x0,0x0}, {0x30,0x0,0x0,0x0}, {0x3,0x30,0x0,0x0},
  {0x4,0x30,0x0,0x0}, {0x4,0x30,0x1,0x0}, {0x6,0x30,0x0,0x0}, {0x7,0x30,0x0,0x0},
  {0x38,0x0,0x0,0x0}, {0x38,0x1,0x0,0x0}, {0x38,0x0,0x0,0x0}, {0x38,0x3,0x0,0x0},
  {0x3c,0x0,0x0,0x0}, {0x3c,0x1,0x0,0x0}, {0x3e,0x0,0x0,0x0}, {0x3f,0x0,0x0,0x0},
  {0x40,0x0,0x0,0x0}, {0x1,0x40,0x0,0x0}, {0x40,0x0,0x0,0x0}, {0x3,0x40,0x0,0x0},
  {0x4,0x40,0x0,0x0}, {0x4,0x1,0x40,0x0}, {0x6,0x40,0x0,0x0}, {0x7,0x40,0x0,0x0},
  {0x40,0x0,0x0,0x0}, {0x1,0x40,0x0,0x0}, {0x40,0x0,0x0,0x0}, {0x3,0x40,0x0,0x0},
  {0xc,0x40,0x0,0x0}, {0xc,0x1,0x40,0x0}, {0xe,0x40,0x0,0x0}, {0xf,0x40,0x0,0x0},
  {0x10,0x40,0x0,0x0}, {0x10,0x1,0x40,0x0}, {0x10,0x40,0x0,0x0}, {0x3,0x10,0x40,0x0},
  {0x4,0x10,0x40,0x0}, {0x4,0x10,0x1,0x40}, {0x6,0x10,0x40,0x0}, {0x7,0x10,0x40,0x0},
  {0x18,0x40,0x0,0x0}, {0x18,0x1,0x40,0x0}, {0x18,0x40,0x0,0x0}, {0x18,0x3,0x40,0x0},
  {0x1c,0x40,0x0,0x0}, {0x1c,0x1,0x40,0x0}, {0x1e,0x40,0x0,0x0}, {0x1f,0x40,0x0,0x0},
  {0x60,0x0,0x0,0x0}, {0x1,0x60,0x0,0x0}, {0x60,0x0,0x0,0x0}, {0x3,0x60,0x0,0x0},
  {0x4,0x60,0x0,0x0}, {0x4,0x1,0x60,0x0}, {0x6,0x60,0x0,0x0}, {0x7,0x60,0x0,0x0},
  {0x60,0x0,0x0,0x0}, {0x1,0x60,0x0,0x0}, {0x60,0x0,0x0,0x0}, {0x3,0x60,0x0,0x0},
  {0xc,0x60,0x0,0x0}, {0xc,0x1,0x60,0x0}, {0xe,0x60,0x0,0x0}, {0xf,0x60,0x0,0x0},
  {0x70,0x0,0x0,0x0}, {0x70,0x1,0x0,0x0}, {0x70,0x0,0x0,0x0}, {0x3,0x70,0x0,0x0},
  {0x4,0x70,0x0,0x0}, {0x4,0x70,0x1,0x0}, {0x6,0x70,0x0,0x0}, {0x7,0x70,0x0,0x0},
  {0x78,0x0,0x0,0x0}, {0x78,0x1,0x0,0x0}, {0x78,0x0,0x0,0x0}, {0x78,0x3,0x0,0x0},
  {0x7c,0x0,0x0,0x0}, {0x7c,0x1,0x0,0x0}, {0x7e,0x0,0x0,0x0}, {0x7f,0x0,0x0,0x0},
  {0x0,0x0,0x0,0x0}, {0x81,0x0,0x0,0x0}, {0x0,0x0,0x0,0x0}, {0x83,0x0,0x0,0x0},
  {0x4,0x0,0x0,0x0}, {0x4,0x81,0x0,0x0}, {0x6,0x0,0x0,0x0}, {0x87,0x0,0x0,0x0},
  {0x0,0x0,0x0,0x0}, {0x81,0x0,0x0,0x0}, {0x0,0x0,0x0,0x0}, {0x83,0x0,0x0,0x0},
  {0xc,0x0,0x0,0x0}, {0xc,0x81,0x0,0x0}, {0xe,0x0,0x0,0x0}, {0x8f,0x0,0x0,0x0},
  {0x10,0x0,0x0,0x0}, {0x10,0x81,0x0,0x0}, {0x10,0x0,0x0,0x0}, {0x83,0x10,0x0,0x0},
  {0x4,0x10,0x0,0x0}, {0x4,0x10,0x81,0x0}, {0x6,0x10,0x0,0x0}, {0x87,0x10,0x0,0x0},
  {0x18,0x0,0x0,0x0}, {0x18,0x81,0x0,0x0}, {0x18,0x0,0x0,0x0}, {0x18,0x83,0x0,0x0},
  {0x1c,0x0,0x0,0x0}, {0x1c,0x81,0x0,0x0}, {0x1e,0x0,0x0,0x0}, {0x9f,0x0,0x0,0x0},
  {0x0,0x0,0x0,0x0}, {0x81,0x0,0x0,0x0}, {0x0,0x0,0x0,0x0}, {0x83,0x0,0x0,0x0},
  {0x4,0x0,0x0,0x0}, {0x4,0x81,0x0,0x0}, {0x6,0x0,0x0,0x0}, {0x87,0x0,0x0,0x0},
  {0x0,0x0,0x0,0x0}, {0x81,0x0,0x0,0x0}, {0x0,0x0,0x0,0x0}, {0x83,0x0,0x0,0x0},
  {0xc,0x0,0x0,0x0}, {0xc,0x81,0x0,0x0}, {0xe,0x0,0x0,0x0}, {0x8f,0x0,0x0,0x0},
  {0x30,0x0,0x0,0x0}, {0x30,0x81,0x0,0x0}, {0x30,0x0,0x0,0x0}, {0x83,0x30,0x0,0x0},
  {0x4,0x30,0x0,0x0}, {0x4,0x30,0x81,0x0}, {0x6,0x30,0x0,0x0}, {0x87,0x30,0x0,0x0},
  {0x38,0x0,0x0,0x0}, {0x38,0x81,0x0,0x0}, {0x38,0x0,0x0,0x0}, {0x38,0x83,0x0,0x0},
  {0x3c,0x0,0x0,0x0}, {0x3c,0x81,0x0,0x0}, {0x3e,0x0,0x0,0x0}, {0xbf,0x0,0x0,0x0},
  {0xc0,0x0,0x0,0x0}, {0xc1,0x0,0x0,0x0}, {0xc0,0x0,0x0,0x0}, {0xc3,0x0,0x0,0x0},
  {0x4,0xc0,0x0,0x0}, {0x4,0xc1,0x0,0x0}, {0x6,0xc0,0x0,0x0}, {0xc7,0x0,0x0,0x0},
  {0xc0,0x0,0x0,0x0}, {0xc1,0x0,0x0,0x0}, {0xc0,0x0,0x0,0x0}, {0xc3,0x0,0x0,0x0},
  {0xc,0xc0,0x0,0x0}, {0xc,0xc1,0x0,0x0}, {0xe,0xc0,0x0,0x0}, {0xcf,0x0,0x0,0x0},
  {0x10,0xc0,0x0,0x0}, {0x10,0xc1,0x0,0x0}, {0x10,0xc0,0x0,0x0}, {0xc3,0x10,0x0,0x0},
  {0x4,0x10,0xc0,0x0}, {0x4,0x10,0xc1,0x0}, {0x6,0x10,0xc0,0x0}, {0xc7,0x10,0x0,0x0},
  {0x18,0xc0,0x0,0x0}, {0x18,0xc1,0x0,0x0}, {0x18,0xc0,0x0,0x0}, {0x18,0xc3,0x0,0x0},
  {0x1c,0xc0,0x0,0x0}, {0x1c,0xc1,0x0,0x0}, {0x1e,0xc0,0x0,0x0}, {0xdf,0x0,0x0,0x0},
  {0xe0,0x0,0x0,0x0}, {0xe1,0x0,0x0,0x0}, {0xe0,0x0,0x0,0x0}, {0xe3,0x0,0x0,0x0},
  {0x4,0xe0,0x0,0x0}, {0x4,0xe1,0x0,0x0}, {0x6,0xe0,0x0,0x0}, {0xe7,0x0,0x0,0x0},
  {0xe0,0x0,0x0,0x0}, {0xe1,0x0,0x0,0x0}, {0xe0,0x0,0x0,0x0}, {0xe3,0x0,0x0,0x0},
  {0xc,0xe0,0x0,0x0}, {0xc,0xe1,0x0,0x0}, {0xe,0xe0,0x0,0x0}, {0xef,0x0,0x0,0x0},
  {0xf0,0x0,0x0,0x0}, {0xf1,0x0,0x0,0x0}, {0xf0,0x0,0x0,0x0}, {0xf3,0x0,0x0,0x0},
  {0x4,0xf0,0x0,0x0}, {0x4,0xf1,0x0,0x0}, {0x6,0xf0,0x0,0x0}, {0xf7,0x0,0x0,0x0},
  {0xf8,0x0,0x0,0x0}, {0xf9,0x0,0x0,0x0}, {0xf8,0x0,0x0,0x0}, {0xfb,0x0,0x0,0x0},
  {0xfc,0x0,0x0,0x0}, {0xfd,0x0,0x0,0x0}, {0xfe,0x0,0x0,0x0}, {0xff,0x0,0x0,0x0}
};

/* 
   Tableau des composantes 8-connexes 8-adjacentes au point central (C8C8) 
     - en global pour etre efficace.

   Ce tableau contient les codes des C8C8, pour toutes les
   configurations de 8-voisinage possibles.
*/

static int32_t Comp8Tab[256][4] = 
{
  {0x0,0x0,0x0,0x0}, {0x1,0x0,0x0,0x0}, {0x2,0x0,0x0,0x0}, {0x3,0x0,0x0,0x0},
  {0x4,0x0,0x0,0x0}, {0x5,0x0,0x0,0x0}, {0x6,0x0,0x0,0x0}, {0x7,0x0,0x0,0x0},
  {0x8,0x0,0x0,0x0}, {0x8,0x1,0x0,0x0}, {0x8,0x2,0x0,0x0}, {0x8,0x3,0x0,0x0},
  {0xc,0x0,0x0,0x0}, {0xd,0x0,0x0,0x0}, {0xe,0x0,0x0,0x0}, {0xf,0x0,0x0,0x0},
  {0x10,0x0,0x0,0x0}, {0x10,0x1,0x0,0x0}, {0x2,0x10,0x0,0x0}, {0x3,0x10,0x0,0x0},
  {0x14,0x0,0x0,0x0}, {0x15,0x0,0x0,0x0}, {0x16,0x0,0x0,0x0}, {0x17,0x0,0x0,0x0},
  {0x18,0x0,0x0,0x0}, {0x18,0x1,0x0,0x0}, {0x18,0x2,0x0,0x0}, {0x18,0x3,0x0,0x0},
  {0x1c,0x0,0x0,0x0}, {0x1d,0x0,0x0,0x0}, {0x1e,0x0,0x0,0x0}, {0x1f,0x0,0x0,0x0},
  {0x20,0x0,0x0,0x0}, {0x1,0x20,0x0,0x0}, {0x2,0x20,0x0,0x0}, {0x3,0x20,0x0,0x0},
  {0x4,0x20,0x0,0x0}, {0x5,0x20,0x0,0x0}, {0x6,0x20,0x0,0x0}, {0x7,0x20,0x0,0x0},
  {0x8,0x20,0x0,0x0}, {0x8,0x1,0x20,0x0}, {0x8,0x2,0x20,0x0}, {0x8,0x3,0x20,0x0},
  {0xc,0x20,0x0,0x0}, {0xd,0x20,0x0,0x0}, {0xe,0x20,0x0,0x0}, {0xf,0x20,0x0,0x0},
  {0x30,0x0,0x0,0x0}, {0x30,0x1,0x0,0x0}, {0x2,0x30,0x0,0x0}, {0x3,0x30,0x0,0x0},
  {0x34,0x0,0x0,0x0}, {0x35,0x0,0x0,0x0}, {0x36,0x0,0x0,0x0}, {0x37,0x0,0x0,0x0},
  {0x38,0x0,0x0,0x0}, {0x38,0x1,0x0,0x0}, {0x38,0x2,0x0,0x0}, {0x38,0x3,0x0,0x0},
  {0x3c,0x0,0x0,0x0}, {0x3d,0x0,0x0,0x0}, {0x3e,0x0,0x0,0x0}, {0x3f,0x0,0x0,0x0},
  {0x40,0x0,0x0,0x0}, {0x41,0x0,0x0,0x0}, {0x2,0x40,0x0,0x0}, {0x43,0x0,0x0,0x0},
  {0x4,0x40,0x0,0x0}, {0x45,0x0,0x0,0x0}, {0x6,0x40,0x0,0x0}, {0x47,0x0,0x0,0x0},
  {0x8,0x40,0x0,0x0}, {0x8,0x41,0x0,0x0}, {0x8,0x2,0x40,0x0}, {0x8,0x43,0x0,0x0},
  {0xc,0x40,0x0,0x0}, {0x4d,0x0,0x0,0x0}, {0xe,0x40,0x0,0x0}, {0x4f,0x0,0x0,0x0},
  {0x50,0x0,0x0,0x0}, {0x51,0x0,0x0,0x0}, {0x2,0x50,0x0,0x0}, {0x53,0x0,0x0,0x0},
  {0x54,0x0,0x0,0x0}, {0x55,0x0,0x0,0x0}, {0x56,0x0,0x0,0x0}, {0x57,0x0,0x0,0x0},
  {0x58,0x0,0x0,0x0}, {0x59,0x0,0x0,0x0}, {0x58,0x2,0x0,0x0}, {0x5b,0x0,0x0,0x0},
  {0x5c,0x0,0x0,0x0}, {0x5d,0x0,0x0,0x0}, {0x5e,0x0,0x0,0x0}, {0x5f,0x0,0x0,0x0},
  {0x60,0x0,0x0,0x0}, {0x61,0x0,0x0,0x0}, {0x2,0x60,0x0,0x0}, {0x63,0x0,0x0,0x0},
  {0x4,0x60,0x0,0x0}, {0x65,0x0,0x0,0x0}, {0x6,0x60,0x0,0x0}, {0x67,0x0,0x0,0x0},
  {0x8,0x60,0x0,0x0}, {0x8,0x61,0x0,0x0}, {0x8,0x2,0x60,0x0}, {0x8,0x63,0x0,0x0},
  {0xc,0x60,0x0,0x0}, {0x6d,0x0,0x0,0x0}, {0xe,0x60,0x0,0x0}, {0x6f,0x0,0x0,0x0},
  {0x70,0x0,0x0,0x0}, {0x71,0x0,0x0,0x0}, {0x2,0x70,0x0,0x0}, {0x73,0x0,0x0,0x0},
  {0x74,0x0,0x0,0x0}, {0x75,0x0,0x0,0x0}, {0x76,0x0,0x0,0x0}, {0x77,0x0,0x0,0x0},
  {0x78,0x0,0x0,0x0}, {0x79,0x0,0x0,0x0}, {0x78,0x2,0x0,0x0}, {0x7b,0x0,0x0,0x0},
  {0x7c,0x0,0x0,0x0}, {0x7d,0x0,0x0,0x0}, {0x7e,0x0,0x0,0x0}, {0x7f,0x0,0x0,0x0},
  {0x80,0x0,0x0,0x0}, {0x81,0x0,0x0,0x0}, {0x2,0x80,0x0,0x0}, {0x83,0x0,0x0,0x0},
  {0x4,0x80,0x0,0x0}, {0x85,0x0,0x0,0x0}, {0x6,0x80,0x0,0x0}, {0x87,0x0,0x0,0x0},
  {0x8,0x80,0x0,0x0}, {0x8,0x81,0x0,0x0}, {0x8,0x2,0x80,0x0}, {0x8,0x83,0x0,0x0},
  {0xc,0x80,0x0,0x0}, {0x8d,0x0,0x0,0x0}, {0xe,0x80,0x0,0x0}, {0x8f,0x0,0x0,0x0},
  {0x10,0x80,0x0,0x0}, {0x10,0x81,0x0,0x0}, {0x2,0x10,0x80,0x0}, {0x83,0x10,0x0,0x0},
  {0x14,0x80,0x0,0x0}, {0x95,0x0,0x0,0x0}, {0x16,0x80,0x0,0x0}, {0x97,0x0,0x0,0x0},
  {0x18,0x80,0x0,0x0}, {0x18,0x81,0x0,0x0}, {0x18,0x2,0x80,0x0}, {0x18,0x83,0x0,0x0},
  {0x1c,0x80,0x0,0x0}, {0x9d,0x0,0x0,0x0}, {0x1e,0x80,0x0,0x0}, {0x9f,0x0,0x0,0x0},
  {0x20,0x80,0x0,0x0}, {0x81,0x20,0x0,0x0}, {0x2,0x20,0x80,0x0}, {0x83,0x20,0x0,0x0},
  {0x4,0x20,0x80,0x0}, {0x85,0x20,0x0,0x0}, {0x6,0x20,0x80,0x0}, {0x87,0x20,0x0,0x0},
  {0x8,0x20,0x80,0x0}, {0x8,0x81,0x20,0x0}, {0x8,0x2,0x20,0x80}, {0x8,0x83,0x20,0x0},
  {0xc,0x20,0x80,0x0}, {0x8d,0x20,0x0,0x0}, {0xe,0x20,0x80,0x0}, {0x8f,0x20,0x0,0x0},
  {0x30,0x80,0x0,0x0}, {0x30,0x81,0x0,0x0}, {0x2,0x30,0x80,0x0}, {0x83,0x30,0x0,0x0},
  {0x34,0x80,0x0,0x0}, {0xb5,0x0,0x0,0x0}, {0x36,0x80,0x0,0x0}, {0xb7,0x0,0x0,0x0},
  {0x38,0x80,0x0,0x0}, {0x38,0x81,0x0,0x0}, {0x38,0x2,0x80,0x0}, {0x38,0x83,0x0,0x0},
  {0x3c,0x80,0x0,0x0}, {0xbd,0x0,0x0,0x0}, {0x3e,0x80,0x0,0x0}, {0xbf,0x0,0x0,0x0},
  {0xc0,0x0,0x0,0x0}, {0xc1,0x0,0x0,0x0}, {0x2,0xc0,0x0,0x0}, {0xc3,0x0,0x0,0x0},
  {0x4,0xc0,0x0,0x0}, {0xc5,0x0,0x0,0x0}, {0x6,0xc0,0x0,0x0}, {0xc7,0x0,0x0,0x0},
  {0x8,0xc0,0x0,0x0}, {0x8,0xc1,0x0,0x0}, {0x8,0x2,0xc0,0x0}, {0x8,0xc3,0x0,0x0},
  {0xc,0xc0,0x0,0x0}, {0xcd,0x0,0x0,0x0}, {0xe,0xc0,0x0,0x0}, {0xcf,0x0,0x0,0x0},
  {0xd0,0x0,0x0,0x0}, {0xd1,0x0,0x0,0x0}, {0x2,0xd0,0x0,0x0}, {0xd3,0x0,0x0,0x0},
  {0xd4,0x0,0x0,0x0}, {0xd5,0x0,0x0,0x0}, {0xd6,0x0,0x0,0x0}, {0xd7,0x0,0x0,0x0},
  {0xd8,0x0,0x0,0x0}, {0xd9,0x0,0x0,0x0}, {0xd8,0x2,0x0,0x0}, {0xdb,0x0,0x0,0x0},
  {0xdc,0x0,0x0,0x0}, {0xdd,0x0,0x0,0x0}, {0xde,0x0,0x0,0x0}, {0xdf,0x0,0x0,0x0},
  {0xe0,0x0,0x0,0x0}, {0xe1,0x0,0x0,0x0}, {0x2,0xe0,0x0,0x0}, {0xe3,0x0,0x0,0x0},
  {0x4,0xe0,0x0,0x0}, {0xe5,0x0,0x0,0x0}, {0x6,0xe0,0x0,0x0}, {0xe7,0x0,0x0,0x0},
  {0x8,0xe0,0x0,0x0}, {0x8,0xe1,0x0,0x0}, {0x8,0x2,0xe0,0x0}, {0x8,0xe3,0x0,0x0},
  {0xc,0xe0,0x0,0x0}, {0xed,0x0,0x0,0x0}, {0xe,0xe0,0x0,0x0}, {0xef,0x0,0x0,0x0},
  {0xf0,0x0,0x0,0x0}, {0xf1,0x0,0x0,0x0}, {0x2,0xf0,0x0,0x0}, {0xf3,0x0,0x0,0x0},
  {0xf4,0x0,0x0,0x0}, {0xf5,0x0,0x0,0x0}, {0xf6,0x0,0x0,0x0}, {0xf7,0x0,0x0,0x0},
  {0xf8,0x0,0x0,0x0}, {0xf9,0x0,0x0,0x0}, {0xf8,0x2,0x0,0x0}, {0xfb,0x0,0x0,0x0},
  {0xfc,0x0,0x0,0x0}, {0xfd,0x0,0x0,0x0}, {0xfe,0x0,0x0,0x0}, {0xff,0x0,0x0,0x0}
};

static inline int32_t is_on_frame(index_t p, index_t rs, index_t N)
{
  if ((p%rs==rs-1) || (p<rs) || (p%rs==0) || (p>=N-rs)) /* point de bord */
    return 1;
  else
    return 0;
}

/* ==================================== */
void veriftopo()
/* ==================================== */
{
  index_t i, j, n, m;
  for (i = 0; i < 256; i++)
  {
    m = ~i & 0xff;  /* complementaire de i */
    n = 0;          /* compte de nb de CC */
    for (j = 0; j < 4; j++) if (Comp4Tab[m][j]) n++;
    if (n != TopoTab[i][0]) 
#ifdef MC_64_BITS
      printf("ERREUR pour %lld\n", i);
#else
      printf("ERREUR pour %d\n", i);
#endif
  }
  printf("FIN TEST 4\n");

  for (i = 0; i < 256; i++)
  { 
    n = 0;          /* compte de nb de CC */
    for (j = 0; j < 4; j++) if (Comp8Tab[i][j]) n++;
    if (n != TopoTab[i][1]) 
#ifdef MC_64_BITS
      printf("ERREUR pour %lld\n", i);
#else
      printf("ERREUR pour %d\n", i);
#endif
  }
  printf("FIN TEST 8\n");
} /* veriftopo() */

/* ==================================== */
uint8_t mask(
  uint8_t *img,          /* pointeur base image */
  index_t p,                       /* index du point */
  index_t rs,                      /* taille rangee */
  index_t N)                       /* taille image */
/* 
  retourne un masque binaire representant le voisinage de p
*/
/* ==================================== */
{
	register uint8_t * ptr = img+p;
	register uint8_t v;
	assert(!is_on_frame(p, rs, N));
	if ( *(ptr+1)   ) v=1; else v=0;
 	if ( *(ptr+1-rs)) v|=2;
 	if ( *(ptr-rs)  ) v|=4;
 	if ( *(ptr-rs-1)) v|=8;
 	if ( *(ptr-1)   ) v|=16;
 	if ( *(ptr-1+rs)) v|=32;
 	if ( *(ptr+rs)  ) v|=64;
 	if ( *(ptr+rs+1)) v|=128;
        return v;
} /* mask() */

/* ==================================== */
uint8_t maskmm(
  uint8_t *img,          /* pointeur base image */
  index_t p,                       /* index du point */
  index_t rs,                      /* taille rangee */
  index_t N)                       /* taille image */
/* 
  retourne un masque binaire representant la coupe < au voisinage de p dans F
*/
/* ==================================== */
{
	register uint8_t val = *(img+p);
	register uint8_t * ptr = img+p;
	register uint8_t v0;
	register uint8_t v1;
	register uint8_t v2;
	register uint8_t v3;
	register uint8_t v4;
	register uint8_t v5;
	register uint8_t v6;
	register uint8_t v7;
	register uint8_t t;

	assert(!is_on_frame(p, rs, N));

        v0 = *(ptr+1)   ;
        v1 = *(ptr+1-rs);
        v2 = *(ptr-rs)  ;
        v3 = *(ptr-rs-1);
        v4 = *(ptr-1)   ;
        v5 = *(ptr-1+rs);
        v6 = *(ptr+rs)  ;
        v7 = *(ptr+rs+1);

	if (v0 < val) t=1; else t=0;
 	if (v1 < val) t|=2;
 	if (v2 < val) t|=4;
 	if (v3 < val) t|=8;
 	if (v4 < val) t|=16;
 	if (v5 < val) t|=32;
 	if (v6 < val) t|=64;
 	if (v7 < val) t|=128;

        return t;
} /* maskmm() */

/* ==================================== */
uint8_t maskpp(
  uint8_t *img,          /* pointeur base image */
  index_t p,                       /* index du point */
  index_t rs,                      /* taille rangee */
  index_t N)                       /* taille image */
/* 
  retourne un masque binaire representant la coupe > au voisinage de p dans F
*/
/* ==================================== */
{
	register uint8_t val = *(img+p);
	register uint8_t * ptr = img+p;
	register uint8_t v0;
	register uint8_t v1;
	register uint8_t v2;
	register uint8_t v3;
	register uint8_t v4;
	register uint8_t v5;
	register uint8_t v6;
	register uint8_t v7;
	register uint8_t t;

	assert(!is_on_frame(p, rs, N));

        v0 = *(ptr+1)   ;
        v1 = *(ptr+1-rs);
        v2 = *(ptr-rs)  ;
        v3 = *(ptr-rs-1);
        v4 = *(ptr-1)   ;
        v5 = *(ptr-1+rs);
        v6 = *(ptr+rs)  ;
        v7 = *(ptr+rs+1);

	if (v0 > val) t=1; else t=0;
 	if (v1 > val) t|=2;
 	if (v2 > val) t|=4;
 	if (v3 > val) t|=8;
 	if (v4 > val) t|=16;
 	if (v5 > val) t|=32;
 	if (v6 > val) t|=64;
 	if (v7 > val) t|=128;

        return t;
} /* maskpp() */

/* ==================================== */
uint8_t maskmmh(
  uint8_t *img,          /* pointeur base image */
  index_t p,                       /* index du point */
  uint8_t h,             /* hauteur de la coupe */
  index_t rs,                      /* taille rangee */
  index_t N)                       /* taille image */
/* 
  retourne un masque binaire representant la coupe <h au voisinage de p dans F
*/
/* ==================================== */
{
	register uint8_t val = h;
	register uint8_t * ptr = img+p;
	register uint8_t v0;
	register uint8_t v1;
	register uint8_t v2;
	register uint8_t v3;
	register uint8_t v4;
	register uint8_t v5;
	register uint8_t v6;
	register uint8_t v7;
	register uint8_t t;

	assert(!is_on_frame(p, rs, N));

        v0 = *(ptr+1)   ;
        v1 = *(ptr+1-rs);
        v2 = *(ptr-rs)  ;
        v3 = *(ptr-rs-1);
        v4 = *(ptr-1)   ;
        v5 = *(ptr-1+rs);
        v6 = *(ptr+rs)  ;
        v7 = *(ptr+rs+1);

	if (v0 < val) t=1; else t=0;
 	if (v1 < val) t|=2;
 	if (v2 < val) t|=4;
 	if (v3 < val) t|=8;
 	if (v4 < val) t|=16;
 	if (v5 < val) t|=32;
 	if (v6 < val) t|=64;
 	if (v7 < val) t|=128;

        return t;
} /* maskmmh() */

/* ==================================== */
uint8_t maskpph(
  uint8_t *img,          /* pointeur base image */
  index_t p,                       /* index du point */
  uint8_t h,             /* hauteur de la coupe */
  index_t rs,                      /* taille rangee */
  index_t N)                       /* taille image */
/* 
  retourne un masque binaire representant la coupe >h au voisinage de p dans F
*/
/* ==================================== */
{
	register uint8_t val = h;
	register uint8_t * ptr = img+p;
	register uint8_t v0;
	register uint8_t v1;
	register uint8_t v2;
	register uint8_t v3;
	register uint8_t v4;
	register uint8_t v5;
	register uint8_t v6;
	register uint8_t v7;
	register uint8_t t;

	assert(!is_on_frame(p, rs, N));

        v0 = *(ptr+1)   ;
        v1 = *(ptr+1-rs);
        v2 = *(ptr-rs)  ;
        v3 = *(ptr-rs-1);
        v4 = *(ptr-1)   ;
        v5 = *(ptr-1+rs);
        v6 = *(ptr+rs)  ;
        v7 = *(ptr+rs+1);

	if (v0 > val) t=1; else t=0;
 	if (v1 > val) t|=2;
 	if (v2 > val) t|=4;
 	if (v3 > val) t|=8;
 	if (v4 > val) t|=16;
 	if (v5 > val) t|=32;
 	if (v6 > val) t|=64;
 	if (v7 > val) t|=128;

        return t;
} /* maskpph() */

/* ==================================== */
void nbtopo(        /* nombres topologiques pour minima 4-connexes */
  uint8_t *img,          /* pointeur base image */
  index_t p,                       /* index du point */
  index_t rs,                      /* taille rangee */
  index_t N,                       /* taille image */
  int32_t *t4mm,
  int32_t *t4m,
  int32_t *t8p,
  int32_t *t8pp)                   /* resultats */
/* ==================================== */
{
	register uint8_t val = *(img+p);
	register uint8_t * ptr = img+p;
	register uint8_t v0;
	register uint8_t v1;
	register uint8_t v2;
	register uint8_t v3;
	register uint8_t v4;
	register uint8_t v5;
	register uint8_t v6;
	register uint8_t v7;
	register int32_t t;

	assert(!is_on_frame(p, rs, N));

        v0 = *(ptr+1)   ;
        v1 = *(ptr+1-rs);
        v2 = *(ptr-rs)  ;
        v3 = *(ptr-rs-1);
        v4 = *(ptr-1)   ;
        v5 = *(ptr-1+rs);
        v6 = *(ptr+rs)  ;
        v7 = *(ptr+rs+1);

	if (v0 >= val) t=1; else t=0;
 	if (v1 >= val) t|=2;
 	if (v2 >= val) t|=4;
 	if (v3 >= val) t|=8;
 	if (v4 >= val) t|=16;
 	if (v5 >= val) t|=32;
 	if (v6 >= val) t|=64;
 	if (v7 >= val) t|=128;

	*t4mm=TopoTab[t][0];
	*t8p=TopoTab[t][1];

	if (v0 > val) t=1; else t=0;
 	if (v1 > val) t|=2;
 	if (v2 > val) t|=4;
 	if (v3 > val) t|=8;
 	if (v4 > val) t|=16;
 	if (v5 > val) t|=32;
 	if (v6 > val) t|=64;
 	if (v7 > val) t|=128;

	*t4m=TopoTab[t][0];
	*t8pp=TopoTab[t][1];
} /* nbtopo() */

/* ==================================== */
int32_t t4mm(        /* nombre topologique mm pour minima 4-connexes */
  uint8_t *img,          /* pointeur base image */
  index_t p,                       /* index du point */
  index_t rs,                      /* taille rangee */
  index_t N)                       /* taille image */
/* ==================================== */
{
	register uint8_t val = *(img+p);
	register uint8_t * ptr = img+p;
	register uint8_t v0;
	register uint8_t v1;
	register uint8_t v2;
	register uint8_t v3;
	register uint8_t v4;
	register uint8_t v5;
	register uint8_t v6;
	register uint8_t v7;
	register int32_t t;

	assert(!is_on_frame(p, rs, N));

        v0 = *(ptr+1)   ;
        v1 = *(ptr+1-rs);
        v2 = *(ptr-rs)  ;
        v3 = *(ptr-rs-1);
        v4 = *(ptr-1)   ;
        v5 = *(ptr-1+rs);
        v6 = *(ptr+rs)  ;
        v7 = *(ptr+rs+1);

	if (v0 >= val) t=1; else t=0;
 	if (v1 >= val) t|=2;
 	if (v2 >= val) t|=4;
 	if (v3 >= val) t|=8;
 	if (v4 >= val) t|=16;
 	if (v5 >= val) t|=32;
 	if (v6 >= val) t|=64;
 	if (v7 >= val) t|=128;

	return TopoTab[t][0];
} /* t4mm() */

/* ==================================== */
int32_t t4m(        /* nombre topologique m pour minima 4-connexes */
  uint8_t *img,          /* pointeur base image */
  index_t p,                       /* index du point */
  index_t rs,                      /* taille rangee */
  index_t N)                       /* taille image */
/* ==================================== */
{
	register uint8_t val = *(img+p);
	register uint8_t * ptr = img+p;
	register uint8_t v0;
	register uint8_t v1;
	register uint8_t v2;
	register uint8_t v3;
	register uint8_t v4;
	register uint8_t v5;
	register uint8_t v6;
	register uint8_t v7;
	register int32_t t;

	assert(!is_on_frame(p, rs, N));

        v0 = *(ptr+1)   ;
        v1 = *(ptr+1-rs);
        v2 = *(ptr-rs)  ;
        v3 = *(ptr-rs-1);
        v4 = *(ptr-1)   ;
        v5 = *(ptr-1+rs);
        v6 = *(ptr+rs)  ;
        v7 = *(ptr+rs+1);

	if (v0 > val) t=1; else t=0;
 	if (v1 > val) t|=2;
 	if (v2 > val) t|=4;
 	if (v3 > val) t|=8;
 	if (v4 > val) t|=16;
 	if (v5 > val) t|=32;
 	if (v6 > val) t|=64;
 	if (v7 > val) t|=128;

	return TopoTab[t][0];
} /* t4m() */

/* ==================================== */
int32_t t8p(        /* nombre topologique p pour minima 4-connexes */
  uint8_t *img,          /* pointeur base image */
  index_t p,                       /* index du point */
  index_t rs,                      /* taille rangee */
  index_t N)                       /* taille image */
/* ==================================== */
{
	register uint8_t val = *(img+p);
	register uint8_t * ptr = img+p;
	register uint8_t v0;
	register uint8_t v1;
	register uint8_t v2;
	register uint8_t v3;
	register uint8_t v4;
	register uint8_t v5;
	register uint8_t v6;
	register uint8_t v7;
	register int32_t t;

	assert(!is_on_frame(p, rs, N));

        v0 = *(ptr+1)   ;
        v1 = *(ptr+1-rs);
        v2 = *(ptr-rs)  ;
        v3 = *(ptr-rs-1);
        v4 = *(ptr-1)   ;
        v5 = *(ptr-1+rs);
        v6 = *(ptr+rs)  ;
        v7 = *(ptr+rs+1);

	if (v0 >= val) t=1; else t=0;
 	if (v1 >= val) t|=2;
 	if (v2 >= val) t|=4;
 	if (v3 >= val) t|=8;
 	if (v4 >= val) t|=16;
 	if (v5 >= val) t|=32;
 	if (v6 >= val) t|=64;
 	if (v7 >= val) t|=128;

	return TopoTab[t][1];
} /* t8p() */

/* ==================================== */
int32_t t8pp(        /* nombre topologique pp pour minima 4-connexes */
  uint8_t *img,          /* pointeur base image */
  index_t p,                       /* index du point */
  index_t rs,                      /* taille rangee */
  index_t N)                       /* taille image */
/* ==================================== */
{
	register uint8_t val = *(img+p);
	register uint8_t * ptr = img+p;
	register uint8_t v0;
	register uint8_t v1;
	register uint8_t v2;
	register uint8_t v3;
	register uint8_t v4;
	register uint8_t v5;
	register uint8_t v6;
	register uint8_t v7;
	register int32_t t;

	assert(!is_on_frame(p, rs, N));

        v0 = *(ptr+1)   ;
        v1 = *(ptr+1-rs);
        v2 = *(ptr-rs)  ;
        v3 = *(ptr-rs-1);
        v4 = *(ptr-1)   ;
        v5 = *(ptr-1+rs);
        v6 = *(ptr+rs)  ;
        v7 = *(ptr+rs+1);

	if (v0 > val) t=1; else t=0;
 	if (v1 > val) t|=2;
 	if (v2 > val) t|=4;
 	if (v3 > val) t|=8;
 	if (v4 > val) t|=16;
 	if (v5 > val) t|=32;
 	if (v6 > val) t|=64;
 	if (v7 > val) t|=128;

	return TopoTab[t][1];
} /* t8pp() */

/* ==================================== */
int32_t t8pp_l(        /* nombre topologique pp pour minima 4-connexes (images type int32_t) */
  int32_t *img,          /* pointeur base image */
  index_t p,                       /* index du point */
  index_t rs,                      /* taille rangee */
  index_t N)                       /* taille image */
/* ==================================== */
{
	register int32_t val = *(img+p);
	register int32_t * ptr = img+p;
	register int32_t v0;
	register int32_t v1;
	register int32_t v2;
	register int32_t v3;
	register int32_t v4;
	register int32_t v5;
	register int32_t v6;
	register int32_t v7;
	register int32_t t;

	assert(!is_on_frame(p, rs, N));

        v0 = *(ptr+1)   ;
        v1 = *(ptr+1-rs);
        v2 = *(ptr-rs)  ;
        v3 = *(ptr-rs-1);
        v4 = *(ptr-1)   ;
        v5 = *(ptr-1+rs);
        v6 = *(ptr+rs)  ;
        v7 = *(ptr+rs+1);

	if (v0 > val) t=1; else t=0;
 	if (v1 > val) t|=2;
 	if (v2 > val) t|=4;
 	if (v3 > val) t|=8;
 	if (v4 > val) t|=16;
 	if (v5 > val) t|=32;
 	if (v6 > val) t|=64;
 	if (v7 > val) t|=128;

	return TopoTab[t][1];
} /* t8pp_l() */

/* ==================================== */
void nbtopo2(        /* nombres topologiques pour minima 8-connexes */
  uint8_t *img,          /* pointeur base image */
  index_t p,                       /* index du point */
  index_t rs,                      /* taille rangee */
  index_t N,                       /* taille image */
  int32_t *t8mm,
  int32_t *t8m,
  int32_t *t4p,
  int32_t *t4pp)                   /* resultats */
/* ==================================== */
{
	register uint8_t val = *(img+p);
	register uint8_t * ptr = img+p;
	register uint8_t v0;
	register uint8_t v1;
	register uint8_t v2;
	register uint8_t v3;
	register uint8_t v4;
	register uint8_t v5;
	register uint8_t v6;
	register uint8_t v7;
	register int32_t t;

	assert(!is_on_frame(p, rs, N));

        v0 = *(ptr+1)   ;
        v1 = *(ptr+1-rs);
        v2 = *(ptr-rs)  ;
        v3 = *(ptr-rs-1);
        v4 = *(ptr-1)   ;
        v5 = *(ptr-1+rs);
        v6 = *(ptr+rs)  ;
        v7 = *(ptr+rs+1);

	if (v0 <= val) t=1; else t=0;
 	if (v1 <= val) t|=2;
 	if (v2 <= val) t|=4;
 	if (v3 <= val) t|=8;
 	if (v4 <= val) t|=16;
 	if (v5 <= val) t|=32;
 	if (v6 <= val) t|=64;
 	if (v7 <= val) t|=128;

	*t4pp=TopoTab[t][0];
	*t8m=TopoTab[t][1];

	if (v0 < val) t=1; else t=0;
 	if (v1 < val) t|=2;
 	if (v2 < val) t|=4;
 	if (v3 < val) t|=8;
 	if (v4 < val) t|=16;
 	if (v5 < val) t|=32;
 	if (v6 < val) t|=64;
 	if (v7 < val) t|=128;

	*t4p=TopoTab[t][0];
	*t8mm=TopoTab[t][1];
} /* nbtopo2() */

/* ==================================== */
int32_t t8mm(        /* nombre topologique mm pour minima 8-connexes */
  uint8_t *img,          /* pointeur base image */
  index_t p,                       /* index du point */
  index_t rs,                      /* taille rangee */
  index_t N)                       /* taille image */
/* ==================================== */
{
	register uint8_t val = *(img+p);
	register uint8_t * ptr = img+p;
	register uint8_t v0;
	register uint8_t v1;
	register uint8_t v2;
	register uint8_t v3;
	register uint8_t v4;
	register uint8_t v5;
	register uint8_t v6;
	register uint8_t v7;
	register int32_t t;

	assert(!is_on_frame(p, rs, N));

        v0 = *(ptr+1)   ;
        v1 = *(ptr+1-rs);
        v2 = *(ptr-rs)  ;
        v3 = *(ptr-rs-1);
        v4 = *(ptr-1)   ;
        v5 = *(ptr-1+rs);
        v6 = *(ptr+rs)  ;
        v7 = *(ptr+rs+1);

	if (v0 < val) t=1; else t=0;
 	if (v1 < val) t|=2;
 	if (v2 < val) t|=4;
 	if (v3 < val) t|=8;
 	if (v4 < val) t|=16;
 	if (v5 < val) t|=32;
 	if (v6 < val) t|=64;
 	if (v7 < val) t|=128;

	return TopoTab[t][1];
} /* t8mm() */

/* ==================================== */
int32_t t8m(        /* nombre topologique m pour minima 8-connexes */
  uint8_t *img,          /* pointeur base image */
  index_t p,                       /* index du point */
  index_t rs,                      /* taille rangee */
  index_t N)                       /* taille image */
/* ==================================== */
{
	register uint8_t val = *(img+p);
	register uint8_t * ptr = img+p;
	register uint8_t v0;
	register uint8_t v1;
	register uint8_t v2;
	register uint8_t v3;
	register uint8_t v4;
	register uint8_t v5;
	register uint8_t v6;
	register uint8_t v7;
	register int32_t t;

	assert(!is_on_frame(p, rs, N));

        v0 = *(ptr+1)   ;
        v1 = *(ptr+1-rs);
        v2 = *(ptr-rs)  ;
        v3 = *(ptr-rs-1);
        v4 = *(ptr-1)   ;
        v5 = *(ptr-1+rs);
        v6 = *(ptr+rs)  ;
        v7 = *(ptr+rs+1);

	if (v0 <= val) t=1; else t=0;
 	if (v1 <= val) t|=2;
 	if (v2 <= val) t|=4;
 	if (v3 <= val) t|=8;
 	if (v4 <= val) t|=16;
 	if (v5 <= val) t|=32;
 	if (v6 <= val) t|=64;
 	if (v7 <= val) t|=128;

	return TopoTab[t][1];
} /* t8m() */

/* ==================================== */
int32_t t4p(        /* nombre topologique p pour minima 8-connexes */
  uint8_t *img,          /* pointeur base image */
  index_t p,                       /* index du point */
  index_t rs,                      /* taille rangee */
  index_t N)                       /* taille image */
/* ==================================== */
{
	register uint8_t val = *(img+p);
	register uint8_t * ptr = img+p;
	register uint8_t v0;
	register uint8_t v1;
	register uint8_t v2;
	register uint8_t v3;
	register uint8_t v4;
	register uint8_t v5;
	register uint8_t v6;
	register uint8_t v7;
	register int32_t t;

	assert(!is_on_frame(p, rs, N));

        v0 = *(ptr+1)   ;
        v1 = *(ptr+1-rs);
        v2 = *(ptr-rs)  ;
        v3 = *(ptr-rs-1);
        v4 = *(ptr-1)   ;
        v5 = *(ptr-1+rs);
        v6 = *(ptr+rs)  ;
        v7 = *(ptr+rs+1);

	if (v0 < val) t=1; else t=0;
 	if (v1 < val) t|=2;
 	if (v2 < val) t|=4;
 	if (v3 < val) t|=8;
 	if (v4 < val) t|=16;
 	if (v5 < val) t|=32;
 	if (v6 < val) t|=64;
 	if (v7 < val) t|=128;

	return TopoTab[t][0];
} /* t4p() */

/* ==================================== */
int32_t t4pp(        /* nombre topologique pp pour minima 8-connexes */
  uint8_t *img,          /* pointeur base image */
  index_t p,                       /* index du point */
  index_t rs,                      /* taille rangee */
  index_t N)                       /* taille image */
/* ==================================== */
{
	register uint8_t val = *(img+p);
	register uint8_t * ptr = img+p;
	register uint8_t v0;
	register uint8_t v1;
	register uint8_t v2;
	register uint8_t v3;
	register uint8_t v4;
	register uint8_t v5;
	register uint8_t v6;
	register uint8_t v7;
	register int32_t t;

	assert(!is_on_frame(p, rs, N));

        v0 = *(ptr+1)   ;
        v1 = *(ptr+1-rs);
        v2 = *(ptr-rs)  ;
        v3 = *(ptr-rs-1);
        v4 = *(ptr-1)   ;
        v5 = *(ptr-1+rs);
        v6 = *(ptr+rs)  ;
        v7 = *(ptr+rs+1);

	if (v0 <= val) t=1; else t=0;
 	if (v1 <= val) t|=2;
 	if (v2 <= val) t|=4;
 	if (v3 <= val) t|=8;
 	if (v4 <= val) t|=16;
 	if (v5 <= val) t|=32;
 	if (v6 <= val) t|=64;
 	if (v7 <= val) t|=128;

	return TopoTab[t][0];
} /* t4pp() */

/* ==================================== */
int32_t t4pp_l(        /* nombre topologique pp pour minima 8-connexes (images type int32_t) */
  int32_t *img,          /* pointeur base image */
  index_t p,                       /* index du point */
  index_t rs,                      /* taille rangee */
  index_t N)                       /* taille image */
/* ==================================== */
{
	register int32_t val = *(img+p);
	register int32_t * ptr = img+p;
	register int32_t v0;
	register int32_t v1;
	register int32_t v2;
	register int32_t v3;
	register int32_t v4;
	register int32_t v5;
	register int32_t v6;
	register int32_t v7;
	register int32_t t;

	assert(!is_on_frame(p, rs, N));

        v0 = *(ptr+1)   ;
        v1 = *(ptr+1-rs);
        v2 = *(ptr-rs)  ;
        v3 = *(ptr-rs-1);
        v4 = *(ptr-1)   ;
        v5 = *(ptr-1+rs);
        v6 = *(ptr+rs)  ;
        v7 = *(ptr+rs+1);

	if (v0 <= val) t=1; else t=0;
 	if (v1 <= val) t|=2;
 	if (v2 <= val) t|=4;
 	if (v3 <= val) t|=8;
 	if (v4 <= val) t|=16;
 	if (v5 <= val) t|=32;
 	if (v6 <= val) t|=64;
 	if (v7 <= val) t|=128;

	return TopoTab[t][0];
} /* t4pp_l() */

/* ==================================== */
void nbtopoh(    /* minima 4-connexes - version avec hauteur de la coupe variable */
  uint8_t *img,          /* pointeur base image */
  index_t p,                       /* index du point */
  uint8_t h,             /* hauteur de la coupe */
  index_t rs,                      /* taille rangee */
  index_t N,                       /* taille image */
  int32_t *t4mm,
  int32_t *t4m,
  int32_t *t8p,
  int32_t *t8pp)                   /* resultats */
/* ==================================== */
{
	register uint8_t val = h;
	register uint8_t * ptr = img+p;
	register uint8_t v0;
	register uint8_t v1;
	register uint8_t v2;
	register uint8_t v3;
	register uint8_t v4;
	register uint8_t v5;
	register uint8_t v6;
	register uint8_t v7;
	register int32_t t;

	assert(!is_on_frame(p, rs, N));

        v0 = *(ptr+1)   ;
        v1 = *(ptr+1-rs);
        v2 = *(ptr-rs)  ;
        v3 = *(ptr-rs-1);
        v4 = *(ptr-1)   ;
        v5 = *(ptr-1+rs);
        v6 = *(ptr+rs)  ;
        v7 = *(ptr+rs+1);

	if (v0 >= val) t=1; else t=0;
 	if (v1 >= val) t|=2;
 	if (v2 >= val) t|=4;
 	if (v3 >= val) t|=8;
 	if (v4 >= val) t|=16;
 	if (v5 >= val) t|=32;
 	if (v6 >= val) t|=64;
 	if (v7 >= val) t|=128;

	*t4mm=TopoTab[t][0];
	*t8p=TopoTab[t][1];

	if (v0 > val) t=1; else t=0;
 	if (v1 > val) t|=2;
 	if (v2 > val) t|=4;
 	if (v3 > val) t|=8;
 	if (v4 > val) t|=16;
 	if (v5 > val) t|=32;
 	if (v6 > val) t|=64;
 	if (v7 > val) t|=128;

	*t4m=TopoTab[t][0];
	*t8pp=TopoTab[t][1];
} /* nbtopoh() */

/* ==================================== */
void nbtopoh2(        /* minima 8-connexes - version avec hauteur de la coupe variable */
  uint8_t *img,          /* pointeur base image */
  index_t p,                       /* index du point */
  uint8_t h,             /* hauteur de la coupe */
  index_t rs,                      /* taille rangee */
  index_t N,                       /* taille image */
  int32_t *t8mm,
  int32_t *t8m,
  int32_t *t4p,
  int32_t *t4pp)                   /* resultats */
/* ==================================== */
{
	register uint8_t val = h;
	register uint8_t * ptr = img+p;
	register uint8_t v0;
	register uint8_t v1;
	register uint8_t v2;
	register uint8_t v3;
	register uint8_t v4;
	register uint8_t v5;
	register uint8_t v6;
	register uint8_t v7;
	register int32_t t;

	assert(!is_on_frame(p, rs, N));

        v0 = *(ptr+1)   ;
        v1 = *(ptr+1-rs);
        v2 = *(ptr-rs)  ;
        v3 = *(ptr-rs-1);
        v4 = *(ptr-1)   ;
        v5 = *(ptr-1+rs);
        v6 = *(ptr+rs)  ;
        v7 = *(ptr+rs+1);

	if (v0 <= val) t=1; else t=0;
 	if (v1 <= val) t|=2;
 	if (v2 <= val) t|=4;
 	if (v3 <= val) t|=8;
 	if (v4 <= val) t|=16;
 	if (v5 <= val) t|=32;
 	if (v6 <= val) t|=64;
 	if (v7 <= val) t|=128;

	*t4pp=TopoTab[t][0];
	*t8m=TopoTab[t][1];

	if (v0 < val) t=1; else t=0;
 	if (v1 < val) t|=2;
 	if (v2 < val) t|=4;
 	if (v3 < val) t|=8;
 	if (v4 < val) t|=16;
 	if (v5 < val) t|=32;
 	if (v6 < val) t|=64;
 	if (v7 < val) t|=128;

	*t4p=TopoTab[t][0];
	*t8mm=TopoTab[t][1];
} /* nbtopoh2() */

/* ==================================== */
void nbtopoh_l(    /* minima 4-connexes - version avec hauteur de la coupe variable */
  int32_t *img,          /* pointeur base image */
  index_t p,                       /* index du point */
  int32_t h,             /* hauteur de la coupe */
  index_t rs,                      /* taille rangee */
  index_t N,                       /* taille image */
  int32_t *t4mm,
  int32_t *t4m,
  int32_t *t8p,
  int32_t *t8pp)                   /* resultats */
/* ==================================== */
{
	register int32_t val = h;
	register int32_t * ptr = img+p;
	register int32_t v0;
	register int32_t v1;
	register int32_t v2;
	register int32_t v3;
	register int32_t v4;
	register int32_t v5;
	register int32_t v6;
	register int32_t v7;
	register int32_t t;

	assert(!is_on_frame(p, rs, N));

        v0 = *(ptr+1)   ;
        v1 = *(ptr+1-rs);
        v2 = *(ptr-rs)  ;
        v3 = *(ptr-rs-1);
        v4 = *(ptr-1)   ;
        v5 = *(ptr-1+rs);
        v6 = *(ptr+rs)  ;
        v7 = *(ptr+rs+1);

	if (v0 >= val) t=1; else t=0;
 	if (v1 >= val) t|=2;
 	if (v2 >= val) t|=4;
 	if (v3 >= val) t|=8;
 	if (v4 >= val) t|=16;
 	if (v5 >= val) t|=32;
 	if (v6 >= val) t|=64;
 	if (v7 >= val) t|=128;

	*t4mm=TopoTab[t][0];
	*t8p=TopoTab[t][1];

	if (v0 > val) t=1; else t=0;
 	if (v1 > val) t|=2;
 	if (v2 > val) t|=4;
 	if (v3 > val) t|=8;
 	if (v4 > val) t|=16;
 	if (v5 > val) t|=32;
 	if (v6 > val) t|=64;
 	if (v7 > val) t|=128;

	*t4m=TopoTab[t][0];
	*t8pp=TopoTab[t][1];
} /* nbtopoh_l() */

/* ==================================== */
void nbtopoh2_l(        /* minima 8-connexes - version avec hauteur de la coupe variable */
  int32_t *img,          /* pointeur base image */
  index_t p,                       /* index du point */
  int32_t h,             /* hauteur de la coupe */
  index_t rs,                      /* taille rangee */
  index_t N,                       /* taille image */
  int32_t *t8mm,
  int32_t *t8m,
  int32_t *t4p,
  int32_t *t4pp)                   /* resultats */
/* ==================================== */
{
	register int32_t val = h;
	register int32_t * ptr = img+p;
	register int32_t v0;
	register int32_t v1;
	register int32_t v2;
	register int32_t v3;
	register int32_t v4;
	register int32_t v5;
	register int32_t v6;
	register int32_t v7;
	register int32_t t;

	assert(!is_on_frame(p, rs, N));

        v0 = *(ptr+1)   ;
        v1 = *(ptr+1-rs);
        v2 = *(ptr-rs)  ;
        v3 = *(ptr-rs-1);
        v4 = *(ptr-1)   ;
        v5 = *(ptr-1+rs);
        v6 = *(ptr+rs)  ;
        v7 = *(ptr+rs+1);

	if (v0 <= val) t=1; else t=0;
 	if (v1 <= val) t|=2;
 	if (v2 <= val) t|=4;
 	if (v3 <= val) t|=8;
 	if (v4 <= val) t|=16;
 	if (v5 <= val) t|=32;
 	if (v6 <= val) t|=64;
 	if (v7 <= val) t|=128;

	*t4pp=TopoTab[t][0];
	*t8m=TopoTab[t][1];

	if (v0 < val) t=1; else t=0;
 	if (v1 < val) t|=2;
 	if (v2 < val) t|=4;
 	if (v3 < val) t|=8;
 	if (v4 < val) t|=16;
 	if (v5 < val) t|=32;
 	if (v6 < val) t|=64;
 	if (v7 < val) t|=128;

	*t4p=TopoTab[t][0];
	*t8mm=TopoTab[t][1];
} /* nbtopoh2_l() */

/* ==================================== */
int32_t t8ph(
  uint8_t *img,          /* pointeur base image */
  index_t p,                       /* index du point */
  uint8_t h,             /* hauteur de la coupe */
  index_t rs,                      /* taille rangee */
  index_t N)                       /* taille image */
/* retourne le T8plus de p a la coupe h */
/* ==================================== */
{
	register uint8_t val = h;
	register uint8_t * ptr = img+p;
	register uint8_t v0;
	register uint8_t v1;
	register uint8_t v2;
	register uint8_t v3;
	register uint8_t v4;
	register uint8_t v5;
	register uint8_t v6;
	register uint8_t v7;
	register int32_t t;

	assert(!is_on_frame(p, rs, N));

        v0 = *(ptr+1)   ;
        v1 = *(ptr+1-rs);
        v2 = *(ptr-rs)  ;
        v3 = *(ptr-rs-1);
        v4 = *(ptr-1)   ;
        v5 = *(ptr-1+rs);
        v6 = *(ptr+rs)  ;
        v7 = *(ptr+rs+1);

	if (v0 >= val) t=1; else t=0;
 	if (v1 >= val) t|=2;
 	if (v2 >= val) t|=4;
 	if (v3 >= val) t|=8;
 	if (v4 >= val) t|=16;
 	if (v5 >= val) t|=32;
 	if (v6 >= val) t|=64;
 	if (v7 >= val) t|=128;

	return TopoTab[t][1];
} /* t8ph() */

/* ==================================== */
uint8_t alpha8m(
  uint8_t *img,          /* pointeur base image */
  index_t p,                       /* index du point */
  index_t rs,                      /* taille rangee */
  index_t N)                       /* taille image */
/* retourne le sup des valeurs < img[p] dans le 8-voisinage de p, */
/* ou img[p] si pas de telles valeurs */
/* ==================================== */
{
	register uint8_t val = *(img+p);
	register uint8_t * ptr = img+p;
	register uint8_t v;
	register int32_t alpha = NDG_MIN - 1;

        v = (p%rs!=rs-1)             ? *(ptr+1)    : val;
        if ((v < val) && ((int32_t)v > alpha)) alpha = (int32_t)v;
        v = ((p%rs!=rs-1)&&(p>=rs))  ? *(ptr+1-rs) : val;
        if ((v < val) && ((int32_t)v > alpha)) alpha = (int32_t)v;
        v = (p>=rs)                  ? *(ptr-rs)   : val;
        if ((v < val) && ((int32_t)v > alpha)) alpha = (int32_t)v;
        v = ((p>=rs)&&(p%rs!=0))     ? *(ptr-rs-1) : val;
        if ((v < val) && ((int32_t)v > alpha)) alpha = (int32_t)v;
        v = (p%rs!=0)                ? *(ptr-1)    : val;
        if ((v < val) && ((int32_t)v > alpha)) alpha = (int32_t)v;
        v = ((p%rs!=0)&&(p<N-rs))    ? *(ptr-1+rs) : val;
        if ((v < val) && ((int32_t)v > alpha)) alpha = (int32_t)v;
        v = (p<N-rs)                 ? *(ptr+rs)   : val;
        if ((v < val) && ((int32_t)v > alpha)) alpha = (int32_t)v;
        v = ((p<N-rs)&&(p%rs!=rs-1)) ? *(ptr+rs+1) : val;
        if ((v < val) && ((int32_t)v > alpha)) alpha = (int32_t)v;
        if (alpha == NDG_MIN - 1) 
          return val;
        else
          return (uint8_t)alpha;
} /* alpha8m() */

/* ==================================== */
uint8_t alpha4m(
  uint8_t *img,          /* pointeur base image */
  index_t p,                       /* index du point */
  index_t rs,                      /* taille rangee */
  index_t N)                       /* taille image */
/* retourne le sup des valeurs < img[p] dans le 4-voisinage de p, */
/* ou img[p] si pas de telles valeurs */
/* ==================================== */
{
	register uint8_t val = *(img+p);
	register uint8_t * ptr = img+p;
	register uint8_t v;
	register int32_t alpha = NDG_MIN - 1;

        v = (p%rs!=rs-1)             ? *(ptr+1)    : val;
        if ((v < val) && ((int32_t)v > alpha)) alpha = (int32_t)v;
        v = (p>=rs)                  ? *(ptr-rs)   : val;
        if ((v < val) && ((int32_t)v > alpha)) alpha = (int32_t)v;
        v = (p%rs!=0)                ? *(ptr-1)    : val;
        if ((v < val) && ((int32_t)v > alpha)) alpha = (int32_t)v;
        v = (p<N-rs)                 ? *(ptr+rs)   : val;
        if ((v < val) && ((int32_t)v > alpha)) alpha = (int32_t)v;

        if (alpha == NDG_MIN - 1) 
          return val;
        else
          return (uint8_t)alpha;
} /* alpha4m() */

/* ==================================== */
uint8_t alpha8p(
  uint8_t *img,          /* pointeur base image */
  index_t p,                       /* index du point */
  index_t rs,                      /* taille rangee */
  index_t N)                       /* taille image */
/* retourne l'inf des valeurs > img[p] dans le 8-voisinage de p, */
/* ou img[p] si pas de telles valeurs */
/* ==================================== */
{
	register uint8_t val = *(img+p);
	register uint8_t * ptr = img+p;
	register uint8_t v;
	register int32_t alpha = NDG_MAX + 1;

        v = (p%rs!=rs-1)             ? *(ptr+1)    : val;
        if ((v > val) && ((int32_t)v < alpha)) alpha = (int32_t)v;
        v = ((p%rs!=rs-1)&&(p>=rs))  ? *(ptr+1-rs) : val;
        if ((v > val) && ((int32_t)v < alpha)) alpha = (int32_t)v;
        v = (p>=rs)                  ? *(ptr-rs)   : val;
        if ((v > val) && ((int32_t)v < alpha)) alpha = (int32_t)v;
        v = ((p>=rs)&&(p%rs!=0))     ? *(ptr-rs-1) : val;
        if ((v > val) && ((int32_t)v < alpha)) alpha = (int32_t)v;
        v = (p%rs!=0)                ? *(ptr-1)    : val;
        if ((v > val) && ((int32_t)v < alpha)) alpha = (int32_t)v;
        v = ((p%rs!=0)&&(p<N-rs))    ? *(ptr-1+rs) : val;
        if ((v > val) && ((int32_t)v < alpha)) alpha = (int32_t)v;
        v = (p<N-rs)                 ? *(ptr+rs)   : val;
        if ((v > val) && ((int32_t)v < alpha)) alpha = (int32_t)v;
        v = ((p<N-rs)&&(p%rs!=rs-1)) ? *(ptr+rs+1) : val;
        if ((v > val) && ((int32_t)v < alpha)) alpha = (int32_t)v;
        if (alpha == NDG_MAX + 1) 
          return val;
        else
          return (uint8_t)alpha;
} /* alpha8p() */

/* ==================================== */
uint8_t alpha4p(
  uint8_t *img,          /* pointeur base image */
  index_t p,                       /* index du point */
  index_t rs,                      /* taille rangee */
  index_t N)                       /* taille image */
/* retourne l'inf des valeurs > img[p] dans le 4-voisinage de p, */
/* ou img[p] si pas de telles valeurs */
/* ==================================== */
{
	register uint8_t val = *(img+p);
	register uint8_t * ptr = img+p;
	register uint8_t v;
	register int32_t alpha = NDG_MAX + 1;

        v = (p%rs!=rs-1)             ? *(ptr+1)    : val;
        if ((v > val) && ((int32_t)v < alpha)) alpha = (int32_t)v;
        v = (p>=rs)                  ? *(ptr-rs)   : val;
        if ((v > val) && ((int32_t)v < alpha)) alpha = (int32_t)v;
        v = (p%rs!=0)                ? *(ptr-1)    : val;
        if ((v > val) && ((int32_t)v < alpha)) alpha = (int32_t)v;
        v = (p<N-rs)                 ? *(ptr+rs)   : val;
        if ((v > val) && ((int32_t)v < alpha)) alpha = (int32_t)v;
        if (alpha == NDG_MAX + 1) 
          return val;
        else
          return (uint8_t)alpha;
} /* alpha4p() */

/* ==================================== */
uint8_t calpha8m(
  uint8_t *img,          /* pointeur base image */
  uint8_t *cnd,          /* pointeur base image condition */
  index_t p,                       /* index du point */
  index_t rs,                      /* taille rangee */
  index_t N)                       /* taille image */
/* retourne le sup des valeurs img[q] < img[p] dans le 8-voisinage de p, */
/* telles que cnd[q] == 0, */  
/* ou img[p] si pas de telles valeurs */
/* ==================================== */
{
	register uint8_t val = *(img+p);
	register uint8_t * ptr = img+p;
	register uint8_t * ntr = cnd+p;
	register uint8_t v;
	register int32_t alpha = NDG_MIN - 1;

        v = (p%rs!=rs-1)             ? *(ptr+1)    : val;
        if ((*(ntr+1) == 0) && (v < val) && ((int32_t)v > alpha)) alpha = (int32_t)v;
        v = ((p%rs!=rs-1)&&(p>=rs))  ? *(ptr+1-rs) : val;
        if ((*(ntr+1-rs) == 0) && (v < val) && ((int32_t)v > alpha)) alpha = (int32_t)v;
        v = (p>=rs)                  ? *(ptr-rs)   : val;
        if ((*(ntr-rs) == 0) && (v < val) && ((int32_t)v > alpha)) alpha = (int32_t)v;
        v = ((p>=rs)&&(p%rs!=0))     ? *(ptr-rs-1) : val;
        if ((*(ntr-rs-1) == 0) && (v < val) && ((int32_t)v > alpha)) alpha = (int32_t)v;
        v = (p%rs!=0)                ? *(ptr-1)    : val;
        if ((*(ntr-1) == 0) && (v < val) && ((int32_t)v > alpha)) alpha = (int32_t)v;
        v = ((p%rs!=0)&&(p<N-rs))    ? *(ptr-1+rs) : val;
        if ((*(ntr-1+rs) == 0) && (v < val) && ((int32_t)v > alpha)) alpha = (int32_t)v;
        v = (p<N-rs)                 ? *(ptr+rs)   : val;
        if ((*(ntr+rs) == 0) && (v < val) && ((int32_t)v > alpha)) alpha = (int32_t)v;
        v = ((p<N-rs)&&(p%rs!=rs-1)) ? *(ptr+rs+1) : val;
        if ((*(ntr+rs+1) == 0) && (v < val) && ((int32_t)v > alpha)) alpha = (int32_t)v;
        if (alpha == NDG_MIN - 1) 
          return val;
        else
          return (uint8_t)alpha;
} /* calpha8m() */

/* ==================================== */
uint8_t calpha4m(
  uint8_t *img,          /* pointeur base image */
  uint8_t *cnd,          /* pointeur base image condition */
  index_t p,                       /* index du point */
  index_t rs,                      /* taille rangee */
  index_t N)                       /* taille image */
/* retourne le sup des valeurs < img[p] dans le 4-voisinage de p, */
/* telles que cnd[q] == 0, */  
/* ou img[p] si pas de telles valeurs */
/* ==================================== */
{
	register uint8_t val = *(img+p);
	register uint8_t * ptr = img+p;
	register uint8_t * ntr = cnd+p;
	register uint8_t v;
	register int32_t alpha = NDG_MIN - 1;

        v = (p%rs!=rs-1)             ? *(ptr+1)    : val;
        if ((*(ntr+1) == 0) && (v < val) && ((int32_t)v > alpha)) alpha = (int32_t)v;
        v = (p>=rs)                  ? *(ptr-rs)   : val;
        if ((*(ntr-rs) == 0) && (v < val) && ((int32_t)v > alpha)) alpha = (int32_t)v;
        v = (p%rs!=0)                ? *(ptr-1)    : val;
        if ((*(ntr-1) == 0) && (v < val) && ((int32_t)v > alpha)) alpha = (int32_t)v;
        v = (p<N-rs)                 ? *(ptr+rs)   : val;
        if ((*(ntr+rs) == 0) && (v < val) && ((int32_t)v > alpha)) alpha = (int32_t)v;

        if (alpha == NDG_MIN - 1) 
          return val;
        else
          return (uint8_t)alpha;
} /* calpha4m() */

/* ==================================== */
uint8_t calpha8p(
  uint8_t *img,          /* pointeur base image */
  uint8_t *cnd,          /* pointeur base image condition */
  index_t p,                       /* index du point */
  index_t rs,                      /* taille rangee */
  index_t N)                       /* taille image */
/* retourne l'inf des valeurs > img[p] dans le 8-voisinage de p, */
/* telles que cnd[q] == 0, */  
/* ou img[p] si pas de telles valeurs */
/* ==================================== */
{
	register uint8_t val = *(img+p);
	register uint8_t * ptr = img+p;
	register uint8_t * ntr = cnd+p;
	register uint8_t v;
	register int32_t alpha = NDG_MAX + 1;

        v = (p%rs!=rs-1)             ? *(ptr+1)    : val;
        if ((*(ntr+1) == 0) && (v > val) && ((int32_t)v < alpha)) alpha = (int32_t)v;
        v = ((p%rs!=rs-1)&&(p>=rs))  ? *(ptr+1-rs) : val;
        if ((*(ntr+1-rs) == 0) && (v > val) && ((int32_t)v < alpha)) alpha = (int32_t)v;
        v = (p>=rs)                  ? *(ptr-rs)   : val;
        if ((*(ntr-rs) == 0) && (v > val) && ((int32_t)v < alpha)) alpha = (int32_t)v;
        v = ((p>=rs)&&(p%rs!=0))     ? *(ptr-rs-1) : val;
        if ((*(ntr-rs-1) == 0) && (v > val) && ((int32_t)v < alpha)) alpha = (int32_t)v;
        v = (p%rs!=0)                ? *(ptr-1)    : val;
        if ((*(ntr-1) == 0) && (v > val) && ((int32_t)v < alpha)) alpha = (int32_t)v;
        v = ((p%rs!=0)&&(p<N-rs))    ? *(ptr-1+rs) : val;
        if ((*(ntr-1+rs) == 0) && (v > val) && ((int32_t)v < alpha)) alpha = (int32_t)v;
        v = (p<N-rs)                 ? *(ptr+rs)   : val;
        if ((*(ntr+rs) == 0) && (v > val) && ((int32_t)v < alpha)) alpha = (int32_t)v;
        v = ((p<N-rs)&&(p%rs!=rs-1)) ? *(ptr+rs+1) : val;
        if ((*(ntr+rs+1) == 0) && (v > val) && ((int32_t)v < alpha)) alpha = (int32_t)v;
        if (alpha == NDG_MAX + 1) 
          return val;
        else
          return (uint8_t)alpha;
} /* calpha8p() */

/* ==================================== */
uint8_t calpha4p(
  uint8_t *img,          /* pointeur base image */
  uint8_t *cnd,          /* pointeur base image condition */
  index_t p,                       /* index du point */
  index_t rs,                      /* taille rangee */
  index_t N)                       /* taille image */
/* retourne l'inf des valeurs > img[p] dans le 4-voisinage de p, */
/* telles que cnd[q] == 0, */  
/* ou img[p] si pas de telles valeurs */
/* ==================================== */
{
	register uint8_t val = *(img+p);
	register uint8_t * ptr = img+p;
	register uint8_t * ntr = cnd+p;
	register uint8_t v;
	register int32_t alpha = NDG_MAX + 1;

        v = (p%rs!=rs-1)             ? *(ptr+1)    : val;
        if ((*(ntr+1) == 0) && (v > val) && ((int32_t)v < alpha)) alpha = (int32_t)v;
        v = (p>=rs)                  ? *(ptr-rs)   : val;
        if ((*(ntr-rs) == 0) && (v > val) && ((int32_t)v < alpha)) alpha = (int32_t)v;
        v = (p%rs!=0)                ? *(ptr-1)    : val;
        if ((*(ntr-1) == 0) && (v > val) && ((int32_t)v < alpha)) alpha = (int32_t)v;
        v = (p<N-rs)                 ? *(ptr+rs)   : val;
        if ((*(ntr+rs) == 0) && (v > val) && ((int32_t)v < alpha)) alpha = (int32_t)v;
        if (alpha == NDG_MAX + 1) 
          return val;
        else
          return (uint8_t)alpha;
} /* calpha4p() */

/* ==================================== */
uint8_t halpha8m(
  uint8_t *img,          /* pointeur base image */
  index_t p,                       /* index du point */
  uint8_t h,             /* valeur de seuil */
  index_t rs,                      /* taille rangee */
  index_t N)                       /* taille image */
/* retourne le sup des valeurs < h dans le 8-voisinage de p, */
/* ou h si pas de telles valeurs */
/* ==================================== */
{
	register uint8_t val = h;
	register uint8_t * ptr = img+p;
	register uint8_t v;
	register int32_t alpha = NDG_MIN - 1;

        v = (p%rs!=rs-1)             ? *(ptr+1)    : val;
        if ((v < val) && ((int32_t)v > alpha)) alpha = (int32_t)v;
        v = ((p%rs!=rs-1)&&(p>=rs))  ? *(ptr+1-rs) : val;
        if ((v < val) && ((int32_t)v > alpha)) alpha = (int32_t)v;
        v = (p>=rs)                  ? *(ptr-rs)   : val;
        if ((v < val) && ((int32_t)v > alpha)) alpha = (int32_t)v;
        v = ((p>=rs)&&(p%rs!=0))     ? *(ptr-rs-1) : val;
        if ((v < val) && ((int32_t)v > alpha)) alpha = (int32_t)v;
        v = (p%rs!=0)                ? *(ptr-1)    : val;
        if ((v < val) && ((int32_t)v > alpha)) alpha = (int32_t)v;
        v = ((p%rs!=0)&&(p<N-rs))    ? *(ptr-1+rs) : val;
        if ((v < val) && ((int32_t)v > alpha)) alpha = (int32_t)v;
        v = (p<N-rs)                 ? *(ptr+rs)   : val;
        if ((v < val) && ((int32_t)v > alpha)) alpha = (int32_t)v;
        v = ((p<N-rs)&&(p%rs!=rs-1)) ? *(ptr+rs+1) : val;
        if ((v < val) && ((int32_t)v > alpha)) alpha = (int32_t)v;
        if (alpha == NDG_MIN - 1) 
          return val;
        else
          return (uint8_t)alpha;
} /* halpha8m() */

/* ==================================== */
uint8_t halpha4m(
  uint8_t *img,          /* pointeur base image */
  index_t p,                       /* index du point */
  uint8_t h,             /* valeur de seuil */
  index_t rs,                      /* taille rangee */
  index_t N)                       /* taille image */
/* retourne le sup des valeurs < img[p] dans le 4-voisinage de p, */
/* ou h si pas de telles valeurs */
/* ==================================== */
{
	register uint8_t val = h;
	register uint8_t * ptr = img+p;
	register uint8_t v;
	register int32_t alpha = NDG_MIN - 1;

        v = (p%rs!=rs-1)             ? *(ptr+1)    : val;
        if ((v < val) && ((int32_t)v > alpha)) alpha = (int32_t)v;
        v = (p>=rs)                  ? *(ptr-rs)   : val;
        if ((v < val) && ((int32_t)v > alpha)) alpha = (int32_t)v;
        v = (p%rs!=0)                ? *(ptr-1)    : val;
        if ((v < val) && ((int32_t)v > alpha)) alpha = (int32_t)v;
        v = (p<N-rs)                 ? *(ptr+rs)   : val;
        if ((v < val) && ((int32_t)v > alpha)) alpha = (int32_t)v;

        if (alpha == NDG_MIN - 1) 
          return val;
        else
          return (uint8_t)alpha;
} /* halpha4m() */

/* ==================================== */
uint8_t halpha8p(
  uint8_t *img,          /* pointeur base image */
  index_t p,                       /* index du point */
  uint8_t h,             /* valeur de seuil */
  index_t rs,                      /* taille rangee */
  index_t N)                       /* taille image */
/* retourne l'inf des valeurs > img[p] dans le 8-voisinage de p, */
/* ou h si pas de telles valeurs */
/* ==================================== */
{
	register uint8_t val = h;
	register uint8_t * ptr = img+p;
	register uint8_t v;
	register int32_t alpha = NDG_MAX + 1;

        v = (p%rs!=rs-1)             ? *(ptr+1)    : val;
        if ((v > val) && ((int32_t)v < alpha)) alpha = (int32_t)v;
        v = ((p%rs!=rs-1)&&(p>=rs))  ? *(ptr+1-rs) : val;
        if ((v > val) && ((int32_t)v < alpha)) alpha = (int32_t)v;
        v = (p>=rs)                  ? *(ptr-rs)   : val;
        if ((v > val) && ((int32_t)v < alpha)) alpha = (int32_t)v;
        v = ((p>=rs)&&(p%rs!=0))     ? *(ptr-rs-1) : val;
        if ((v > val) && ((int32_t)v < alpha)) alpha = (int32_t)v;
        v = (p%rs!=0)                ? *(ptr-1)    : val;
        if ((v > val) && ((int32_t)v < alpha)) alpha = (int32_t)v;
        v = ((p%rs!=0)&&(p<N-rs))    ? *(ptr-1+rs) : val;
        if ((v > val) && ((int32_t)v < alpha)) alpha = (int32_t)v;
        v = (p<N-rs)                 ? *(ptr+rs)   : val;
        if ((v > val) && ((int32_t)v < alpha)) alpha = (int32_t)v;
        v = ((p<N-rs)&&(p%rs!=rs-1)) ? *(ptr+rs+1) : val;
        if ((v > val) && ((int32_t)v < alpha)) alpha = (int32_t)v;
        if (alpha == NDG_MAX + 1) 
          return val;
        else
          return (uint8_t)alpha;
} /* halpha8p() */

/* ==================================== */
uint8_t halpha4p(
  uint8_t *img,          /* pointeur base image */
  index_t p,                       /* index du point */
  uint8_t h,             /* valeur de seuil */
  index_t rs,                      /* taille rangee */
  index_t N)                       /* taille image */
/* retourne l'inf des valeurs > img[p] dans le 4-voisinage de p, */
/* ou h si pas de telles valeurs */
/* ==================================== */
{
	register uint8_t val = h;
	register uint8_t * ptr = img+p;
	register uint8_t v;
	register int32_t alpha = NDG_MAX + 1;

        v = (p%rs!=rs-1)             ? *(ptr+1)    : val;
        if ((v > val) && ((int32_t)v < alpha)) alpha = (int32_t)v;
        v = (p>=rs)                  ? *(ptr-rs)   : val;
        if ((v > val) && ((int32_t)v < alpha)) alpha = (int32_t)v;
        v = (p%rs!=0)                ? *(ptr-1)    : val;
        if ((v > val) && ((int32_t)v < alpha)) alpha = (int32_t)v;
        v = (p<N-rs)                 ? *(ptr+rs)   : val;
        if ((v > val) && ((int32_t)v < alpha)) alpha = (int32_t)v;
        if (alpha == NDG_MAX + 1) 
          return val;
        else
          return (uint8_t)alpha;
} /* halpha4p() */

/* ==================================== */
int32_t alpha8m_l(
  int32_t *img,          /* pointeur base image */
  index_t p,                       /* index du point */
  index_t rs,                      /* taille rangee */
  index_t N)                       /* taille image */
/* retourne le sup des valeurs < img[p] dans le 8-voisinage de p, */
/* ou img[p] si pas de telles valeurs */
/* ==================================== */
{
	register int32_t val = *(img+p);
	register int32_t * ptr = img+p;
	register int32_t v;
	register int32_t alpha = - 1;

        v = (p%rs!=rs-1)             ? *(ptr+1)    : val;
        if ((v < val) && ((int32_t)v > alpha)) alpha = (int32_t)v;
        v = ((p%rs!=rs-1)&&(p>=rs))  ? *(ptr+1-rs) : val;
        if ((v < val) && ((int32_t)v > alpha)) alpha = (int32_t)v;
        v = (p>=rs)                  ? *(ptr-rs)   : val;
        if ((v < val) && ((int32_t)v > alpha)) alpha = (int32_t)v;
        v = ((p>=rs)&&(p%rs!=0))     ? *(ptr-rs-1) : val;
        if ((v < val) && ((int32_t)v > alpha)) alpha = (int32_t)v;
        v = (p%rs!=0)                ? *(ptr-1)    : val;
        if ((v < val) && ((int32_t)v > alpha)) alpha = (int32_t)v;
        v = ((p%rs!=0)&&(p<N-rs))    ? *(ptr-1+rs) : val;
        if ((v < val) && ((int32_t)v > alpha)) alpha = (int32_t)v;
        v = (p<N-rs)                 ? *(ptr+rs)   : val;
        if ((v < val) && ((int32_t)v > alpha)) alpha = (int32_t)v;
        v = ((p<N-rs)&&(p%rs!=rs-1)) ? *(ptr+rs+1) : val;
        if ((v < val) && ((int32_t)v > alpha)) alpha = (int32_t)v;
        if (alpha == - 1) 
          return val;
        else
          return (int32_t)alpha;
} /* alpha8m_l() */

/* ==================================== */
int32_t alpha4m_l(
  int32_t *img,          /* pointeur base image */
  index_t p,                       /* index du point */
  index_t rs,                      /* taille rangee */
  index_t N)                       /* taille image */
/* retourne le sup des valeurs < img[p] dans le 4-voisinage de p, */
/* ou img[p] si pas de telles valeurs */
/* ==================================== */
{
	register int32_t val = *(img+p);
	register int32_t * ptr = img+p;
	register int32_t v;
	register int32_t alpha = - 1;

        v = (p%rs!=rs-1)             ? *(ptr+1)    : val;
        if ((v < val) && ((int32_t)v > alpha)) alpha = (int32_t)v;
        v = (p>=rs)                  ? *(ptr-rs)   : val;
        if ((v < val) && ((int32_t)v > alpha)) alpha = (int32_t)v;
        v = (p%rs!=0)                ? *(ptr-1)    : val;
        if ((v < val) && ((int32_t)v > alpha)) alpha = (int32_t)v;
        v = (p<N-rs)                 ? *(ptr+rs)   : val;
        if ((v < val) && ((int32_t)v > alpha)) alpha = (int32_t)v;

        if (alpha == - 1) 
          return val;
        else
          return (int32_t)alpha;
} /* alpha4m_l() */

/* ==================================== */
int32_t alpha8p_l(
  int32_t *img,          /* pointeur base image */
  index_t p,                       /* index du point */
  index_t rs,                      /* taille rangee */
  index_t N)                       /* taille image */
/* retourne l'inf des valeurs > img[p] dans le 8-voisinage de p, */
/* ou img[p] si pas de telles valeurs */
/* ==================================== */
{
	register int32_t val = *(img+p);
	register int32_t * ptr = img+p;
	register int32_t v;
	register int32_t alpha = INT32_MAX;

        v = (p%rs!=rs-1)             ? *(ptr+1)    : val;
        if ((v > val) && ((int32_t)v < alpha)) alpha = (int32_t)v;
        v = ((p%rs!=rs-1)&&(p>=rs))  ? *(ptr+1-rs) : val;
        if ((v > val) && ((int32_t)v < alpha)) alpha = (int32_t)v;
        v = (p>=rs)                  ? *(ptr-rs)   : val;
        if ((v > val) && ((int32_t)v < alpha)) alpha = (int32_t)v;
        v = ((p>=rs)&&(p%rs!=0))     ? *(ptr-rs-1) : val;
        if ((v > val) && ((int32_t)v < alpha)) alpha = (int32_t)v;
        v = (p%rs!=0)                ? *(ptr-1)    : val;
        if ((v > val) && ((int32_t)v < alpha)) alpha = (int32_t)v;
        v = ((p%rs!=0)&&(p<N-rs))    ? *(ptr-1+rs) : val;
        if ((v > val) && ((int32_t)v < alpha)) alpha = (int32_t)v;
        v = (p<N-rs)                 ? *(ptr+rs)   : val;
        if ((v > val) && ((int32_t)v < alpha)) alpha = (int32_t)v;
        v = ((p<N-rs)&&(p%rs!=rs-1)) ? *(ptr+rs+1) : val;
        if ((v > val) && ((int32_t)v < alpha)) alpha = (int32_t)v;
        if (alpha == INT32_MAX) 
          return val;
        else
          return (int32_t)alpha;
} /* alpha8p_l() */

/* ==================================== */
int32_t alpha4p_l(
  int32_t *img,          /* pointeur base image */
  index_t p,                       /* index du point */
  index_t rs,                      /* taille rangee */
  index_t N)                       /* taille image */
/* retourne l'inf des valeurs > img[p] dans le 4-voisinage de p, */
/* ou img[p] si pas de telles valeurs */
/* ==================================== */
{
	register int32_t val = *(img+p);
	register int32_t * ptr = img+p;
	register int32_t v;
	register int32_t alpha = INT32_MAX;

        v = (p%rs!=rs-1)             ? *(ptr+1)    : val;
        if ((v > val) && ((int32_t)v < alpha)) alpha = (int32_t)v;
        v = (p>=rs)                  ? *(ptr-rs)   : val;
        if ((v > val) && ((int32_t)v < alpha)) alpha = (int32_t)v;
        v = (p%rs!=0)                ? *(ptr-1)    : val;
        if ((v > val) && ((int32_t)v < alpha)) alpha = (int32_t)v;
        v = (p<N-rs)                 ? *(ptr+rs)   : val;
        if ((v > val) && ((int32_t)v < alpha)) alpha = (int32_t)v;
        if (alpha == INT32_MAX) 
          return val;
        else
          return (int32_t)alpha;
} /* alpha4p_l() */

/* ==================================== */
uint8_t beta8m(
  uint8_t *img,          /* pointeur base image */
  index_t p,                       /* index du point */
  index_t rs,                      /* taille rangee */
  index_t N)                       /* taille image */
/* retourne l'inf des valeurs du (8-voisinage de p) U p */
/* ou img[p] si pas de telles valeurs */
/* ==================================== */
{
	register uint8_t val = *(img+p);
	register uint8_t * ptr = img+p;
	register uint8_t v;
	register uint8_t beta = val;

        v = (p%rs!=rs-1)             ? *(ptr+1)    : val;
        if (v < beta) beta = (int32_t)v;
        v = ((p%rs!=rs-1)&&(p>=rs))  ? *(ptr+1-rs) : val;
        if (v < beta) beta = (int32_t)v;
        v = (p>=rs)                  ? *(ptr-rs)   : val;
        if (v < beta) beta = (int32_t)v;
        v = ((p>=rs)&&(p%rs!=0))     ? *(ptr-rs-1) : val;
        if (v < beta) beta = (int32_t)v;
        v = (p%rs!=0)                ? *(ptr-1)    : val;
        if (v < beta) beta = (int32_t)v;
        v = ((p%rs!=0)&&(p<N-rs))    ? *(ptr-1+rs) : val;
        if (v < beta) beta = (int32_t)v;
        v = (p<N-rs)                 ? *(ptr+rs)   : val;
        if (v < beta) beta = (int32_t)v;
        v = ((p<N-rs)&&(p%rs!=rs-1)) ? *(ptr+rs+1) : val;
        if (v < beta) beta = (int32_t)v;
        return beta;
} /* beta8m() */

/* ==================================== */
uint8_t beta8p(
  uint8_t *img,          /* pointeur base image */
  index_t p,                       /* index du point */
  index_t rs,                      /* taille rangee */
  index_t N)                       /* taille image */
/* retourne le sup des valeurs du (8-voisinage de p) U p */
/* ou img[p] si pas de telles valeurs */
/* ==================================== */
{
	register uint8_t val = *(img+p);
	register uint8_t * ptr = img+p;
	register uint8_t v;
	register uint8_t beta = val;

        v = (p%rs!=rs-1)             ? *(ptr+1)    : val;
        if (v > beta) beta = (int32_t)v;
        v = ((p%rs!=rs-1)&&(p>=rs))  ? *(ptr+1-rs) : val;
        if (v > beta) beta = (int32_t)v;
        v = (p>=rs)                  ? *(ptr-rs)   : val;
        if (v > beta) beta = (int32_t)v;
        v = ((p>=rs)&&(p%rs!=0))     ? *(ptr-rs-1) : val;
        if (v > beta) beta = (int32_t)v;
        v = (p%rs!=0)                ? *(ptr-1)    : val;
        if (v > beta) beta = (int32_t)v;
        v = ((p%rs!=0)&&(p<N-rs))    ? *(ptr-1+rs) : val;
        if (v > beta) beta = (int32_t)v;
        v = (p<N-rs)                 ? *(ptr+rs)   : val;
        if (v > beta) beta = (int32_t)v;
        v = ((p<N-rs)&&(p%rs!=rs-1)) ? *(ptr+rs+1) : val;
        if (v > beta) beta = (int32_t)v;
        return beta;
} /* beta8p() */

/* ==================================== */
uint8_t beta4m(
  uint8_t *img,          /* pointeur base image */
  index_t p,                       /* index du point */
  index_t rs,                      /* taille rangee */
  index_t N)                       /* taille image */
/* retourne l'inf des valeurs du (4-voisinage de p) U p */
/* ou img[p] si pas de telles valeurs */
/* ==================================== */
{
	register uint8_t val = *(img+p);
	register uint8_t * ptr = img+p;
	register uint8_t v;
	register uint8_t beta = val;

        v = (p%rs!=rs-1)             ? *(ptr+1)    : val;
        if (v < beta) beta = (int32_t)v;
        v = (p>=rs)                  ? *(ptr-rs)   : val;
        if (v < beta) beta = (int32_t)v;
        v = (p%rs!=0)                ? *(ptr-1)    : val;
        if (v < beta) beta = (int32_t)v;
        v = (p<N-rs)                 ? *(ptr+rs)   : val;
        if (v < beta) beta = (int32_t)v;
        return beta;
} /* beta4m() */

/* ==================================== */
uint8_t beta4p(
  uint8_t *img,          /* pointeur base image */
  index_t p,                       /* index du point */
  index_t rs,                      /* taille rangee */
  index_t N)                       /* taille image */
/* retourne le sup des valeurs du (4-voisinage de p) U p */
/* ou img[p] si pas de telles valeurs */
/* ==================================== */
{
	register uint8_t val = *(img+p);
	register uint8_t * ptr = img+p;
	register uint8_t v;
	register uint8_t beta = val;

        v = (p%rs!=rs-1)             ? *(ptr+1)    : val;
        if (v > beta) beta = (int32_t)v;
        v = (p>=rs)                  ? *(ptr-rs)   : val;
        if (v > beta) beta = (int32_t)v;
        v = (p%rs!=0)                ? *(ptr-1)    : val;
        if (v > beta) beta = (int32_t)v;
        v = (p<N-rs)                 ? *(ptr+rs)   : val;
        if (v > beta) beta = (int32_t)v;
        return beta;
} /* beta4p() */

/* ==================================== */
int32_t typetopo(                  /* types topologiques avec la 4-connexite pour les minima */
  uint8_t *img,          /* pointeur base image */
  index_t p,                       /* index du point */
  index_t rs,                      /* taille rangee */
  index_t N)                       /* taille image */
/* ==================================== */
{
	int32_t t4mm,t4m,t8p,t8pp;

        if ((p%rs==rs-1) || (p<rs) || (p%rs==0) || (p>=N-rs)) /* point de bord */
                return UNDEFINED;

        nbtopo(img, p, rs, N, &t4mm, &t4m, &t8p, &t8pp);

	if ((t8p==0) && (t4mm==1) && (t8pp==0) && (t4m==1))
		return PEAK;
	
	if ((t8p==1) && (t4mm==0))
		{
		if ((t8pp==0) && (t4m==1)) return PLATEAU;
		if ((t8pp==1) && (t4m==0)) return WELL;
		if ((t8pp==1) && (t4m==1)) return CONST_FLOOR;
		return CONV_FLOOR;
		}

	if ((t8p==1) && (t4mm==1))
		{
		if ((t8pp==0) && (t4m==1)) return DEST_CEILING;
		if ((t8pp==1) && (t4m==1)) return CONST_DEST;
		return CONV_DEST;
		}		

	if ((t8pp==0) && (t4m==1)) return DIV_CEILING;
	if ((t8pp==1) && (t4m==1)) return DIV_CONST;
	return SADDLE;
} /* typetopo() */

/* ==================================== */
int32_t typetopo8(  /* types topologiques avec la 8-connexite pour les minima */
  uint8_t *img,          /* pointeur base image */
  index_t p,                       /* index du point */
  index_t rs,                      /* taille rangee */
  index_t N)                       /* taille image */
/* ==================================== */
{
	int32_t t8mm,t8m,t4p,t4pp;

        if ((p%rs==rs-1) || (p<rs) || (p%rs==0) || (p>=N-rs)) /* point de bord */
                return UNDEFINED;

        nbtopo2(img, p, rs, N, &t8mm, &t8m, &t4p, &t4pp);

	if ((t4p==0) && (t8mm==1) && (t4pp==0) && (t8m==1))
		return PEAK;
	
	if ((t4p==1) && (t8mm==0))
        {
          if ((t4pp==0) && (t8m==1)) return PLATEAU;
          if ((t4pp==1) && (t8m==0)) return WELL;
          if ((t4pp==1) && (t8m==1)) return CONST_FLOOR;
          return CONV_FLOOR;
        }

	if ((t4p==1) && (t8mm==1))
	{
          if ((t4pp==0) && (t8m==1)) return DEST_CEILING;
          if ((t4pp==1) && (t8m==1)) return CONST_DEST;
          return CONV_DEST;
	}		

	if ((t4pp==0) && (t8m==1)) return DIV_CEILING;
	if ((t4pp==1) && (t8m==1)) return DIV_CONST;
	return SADDLE;
} /* typetopo8() */

/* ==================================== */
int32_t pdestr( /* teste si un point est destructible - minima 4-connexes */
            /* ancienne version - utiliser maintenant pdestr4() */
  uint8_t *img,          /* pointeur base image */
  index_t p,                       /* index du point */
  index_t rs,                      /* taille rangee */
  index_t N)                       /* taille image */
/* ==================================== */
{	
	int32_t t4mm,t8p,b,n;
	
        if ((p%rs==rs-1) || (p<rs) || (p%rs==0) || (p>=N-rs)) /* point de bord */
                return 0;

        nbtopo(img, p, rs, N, &t4mm, &b, &t8p, &n);
	if ((t4mm==1) && (t8p==1))
		return 1;
	return 0;
} /* pdestr() */

/* ==================================== */
int32_t pdestr4( /* teste si un point est destructible - minima 4-connexes */
  uint8_t *img,          /* pointeur base image */
  index_t p,                       /* index du point */
  index_t rs,                      /* taille rangee */
  index_t N)                       /* taille image */
/* ==================================== */
{	
	int32_t t4mm,t8p,b,n;
	
        if ((p%rs==rs-1) || (p<rs) || (p%rs==0) || (p>=N-rs)) /* point de bord */
                return 0;

        nbtopo(img, p, rs, N, &t4mm, &b, &t8p, &n);
	if ((t4mm==1) && (t8p==1))
		return 1;
	return 0;
} /* pdestr4() */

/* ==================================== */
int32_t pdestr8( /* teste si un point est destructible - minima 8-connexes */
  uint8_t *img,          /* pointeur base image */
  index_t p,                       /* index du point */
  index_t rs,                      /* taille rangee */
  index_t N)                       /* taille image */
/* ==================================== */
{	
	int32_t t8mm,t4p,b,n;
	
        if ((p%rs==rs-1)||(p<rs)||(p%rs==0)||(p>=N-rs)) /* point de bord */
                return 0;

        nbtopo2(img, p, rs, N, &t8mm, &b, &t4p, &n);
	if ((t8mm==1) && (t4p==1))
		return 1;
	return 0;
} /* pdestr8() */

/* ==================================== */
int32_t pconstr(   /* teste si un point est constructible - minima 4-connexes */
               /* ancienne version - utiliser maintenant pconstr4() */
  uint8_t *img,          /* pointeur base image */
  index_t p,                       /* index du point */
  index_t rs,                      /* taille rangee */
  index_t N)                       /* taille image */
/* ==================================== */
{	
	int32_t t4m,t8pp,b,n;

        if ((p%rs==rs-1)||(p<rs)||(p%rs==0)||(p>=N-rs)) /* point de bord */
                return 0;

        nbtopo(img, p, rs, N, &b, &t4m, &n, &t8pp);
	if ((t4m==1) && (t8pp==1))
		return 1;
	return 0;
} /* pconstr() */

/* ==================================== */
int32_t pconstr4(  /* teste si un point est constructible - minima 4-connexes */
  uint8_t *img,          /* pointeur base image */
  index_t p,                       /* index du point */
  index_t rs,                      /* taille rangee */
  index_t N)                       /* taille image */
/* ==================================== */
{	
	int32_t t4m,t8pp,b,n;

        if ((p%rs==rs-1)||(p<rs)||(p%rs==0)||(p>=N-rs)) /* point de bord */
                return 0;

        nbtopo(img, p, rs, N, &b, &t4m, &n, &t8pp);
	if ((t4m==1) && (t8pp==1))
		return 1;
	return 0;
} /* pconstr4() */

/* ==================================== */
int32_t pconstr8(   /* teste si un point est constructible - minima 8-connexes */
  uint8_t *img,          /* pointeur base image */
  index_t p,                       /* index du point */
  index_t rs,                      /* taille rangee */
  index_t N)                       /* taille image */
/* ==================================== */
{	
	int32_t t8m,t4pp,b,n;

        if ((p%rs==rs-1)||(p<rs)||(p%rs==0)||(p>=N-rs)) /* point de bord */
                return 0;

        nbtopo2(img, p, rs, N, &b, &t8m, &n, &t4pp);
	if ((t8m==1) && (t4pp==1))
		return 1;
	return 0;
} /* pconstr8() */

/* ==================================== */
int32_t peakordestr4( /* teste si un point est pic ou destructible - minima 4-connexes */
  uint8_t *img,          /* pointeur base image */
  index_t p,                       /* index du point */
  index_t rs,                      /* taille rangee */
  index_t N)                       /* taille image */
/* ==================================== */
{	
	int32_t t4mm,t8p,b,n;
	
        if ((p%rs==rs-1) || (p<rs) || (p%rs==0) || (p>=N-rs)) /* point de bord */
                return 0;

        nbtopo(img, p, rs, N, &t4mm, &b, &t8p, &n);
	if (t4mm==1) return 1;
	return 0;
} /* peakordestr4() */

/* ==================================== */
int32_t peakordestr8( /* teste si un point est pic ou destructible - minima 8-connexes */
  uint8_t *img,          /* pointeur base image */
  index_t p,                       /* index du point */
  index_t rs,                      /* taille rangee */
  index_t N)                       /* taille image */
/* ==================================== */
{	
	int32_t t8mm,t4p,b,n;
	
        if ((p%rs==rs-1)||(p<rs)||(p%rs==0)||(p>=N-rs)) /* point de bord */
                return 0;

        nbtopo2(img, p, rs, N, &t8mm, &b, &t4p, &n);
	if (t8mm==1) return 1;
	return 0;
} /* peakordestr8() */

/* ==================================== */
int32_t wellorconstr4(  /* teste si un point est puits ou constructible - minima 4-connexes */
  uint8_t *img,          /* pointeur base image */
  index_t p,                       /* index du point */
  index_t rs,                      /* taille rangee */
  index_t N)                       /* taille image */
/* ==================================== */
{	
	int32_t t4m,t8pp,b,n;

        if ((p%rs==rs-1)||(p<rs)||(p%rs==0)||(p>=N-rs)) /* point de bord */
                return 0;

        nbtopo(img, p, rs, N, &b, &t4m, &n, &t8pp);
	if (t8pp==1) return 1;
	return 0;
} /* wellorconstr4() */

/* ==================================== */
int32_t wellorconstr8(   /* teste si un point est puits ou constructible - minima 8-connexes */
  uint8_t *img,          /* pointeur base image */
  index_t p,                       /* index du point */
  index_t rs,                      /* taille rangee */
  index_t N)                       /* taille image */
/* ==================================== */
{	
	int32_t t8m,t4pp,b,n;

        if ((p%rs==rs-1)||(p<rs)||(p%rs==0)||(p>=N-rs)) /* point de bord */
                return 0;

        nbtopo2(img, p, rs, N, &b, &t8m, &n, &t4pp);
	if (t4pp==1) return 1;
	return 0;
} /* wellorconstr8() */

/* ==================================== */
int32_t peak4( /* teste si un point est pic - minima 4-connexes */
  uint8_t *img,          /* pointeur base image */
  index_t p,                       /* index du point */
  index_t rs,                      /* taille rangee */
  index_t N)                       /* taille image */
/* ==================================== */
{	
	int32_t t4mm,t8p,b,n;
	
        if ((p%rs==rs-1) || (p<rs) || (p%rs==0) || (p>=N-rs)) /* point de bord */
                return 0;

        nbtopo(img, p, rs, N, &t4mm, &b, &t8p, &n);
	if ((t4mm==1) && (t8p==0)) return 1;
	return 0;
} /* peak4() */

/* ==================================== */
int32_t peak8( /* teste si un point est pic - minima 8-connexes */
  uint8_t *img,          /* pointeur base image */
  index_t p,                       /* index du point */
  index_t rs,                      /* taille rangee */
  index_t N)                       /* taille image */
/* ==================================== */
{	
	int32_t t8mm,t4p,b,n;
	
        if ((p%rs==rs-1)||(p<rs)||(p%rs==0)||(p>=N-rs)) /* point de bord */
                return 0;

        nbtopo2(img, p, rs, N, &t8mm, &b, &t4p, &n);
	if ((t8mm==1) && (t4p==0)) return 1;
	return 0;
} /* peak8() */

/* ==================================== */
int32_t well4(  /* teste si un point est puits - minima 4-connexes */
  uint8_t *img,          /* pointeur base image */
  index_t p,                       /* index du point */
  index_t rs,                      /* taille rangee */
  index_t N)                       /* taille image */
/* ==================================== */
{	
	int32_t t4m,t8pp,b,n;

        if ((p%rs==rs-1)||(p<rs)||(p%rs==0)||(p>=N-rs)) /* point de bord */
                return 0;

        nbtopo(img, p, rs, N, &b, &t4m, &n, &t8pp);
	if ((t8pp==1) && (t4m==0)) return 1;
	return 0;
} /* well4() */

/* ==================================== */
int32_t well8(   /* teste si un point est puits - minima 8-connexes */
  uint8_t *img,          /* pointeur base image */
  index_t p,                       /* index du point */
  index_t rs,                      /* taille rangee */
  index_t N)                       /* taille image */
/* ==================================== */
{	
	int32_t t8m,t4pp,b,n;

        if ((p%rs==rs-1)||(p<rs)||(p%rs==0)||(p>=N-rs)) /* point de bord */
                return 0;

        nbtopo2(img, p, rs, N, &b, &t8m, &n, &t4pp);
	if ((t4pp==1) && (t8m==0)) return 1;
	return 0;
} /* well8() */

/* ==================================== */
uint8_t delta4m( /* retourne la valeur max. a laquelle p est destructible - minima 4-connexes */
  uint8_t *img,          /* pointeur base image */
  index_t p,                       /* index du point */
  index_t rs,                      /* taille rangee */
  index_t N)                       /* taille image */
/* ==================================== */
{	
  uint8_t ret, sav = img[p];
  while (pdestr4(img, p, rs, N)) img[p] = alpha8m(img, p, rs, N);
  ret = img[p];
  img[p] = sav;
  return ret;
} /* delta4m() */

/* ==================================== */
uint8_t delta8m( /* retourne la valeur max. a laquelle p est destructible - minima 8-connexes */
  uint8_t *img,          /* pointeur base image */
  index_t p,                       /* index du point */
  index_t rs,                      /* taille rangee */
  index_t N)                       /* taille image */
/* ==================================== */
{	
  uint8_t ret, sav = img[p];
  while (pdestr8(img, p, rs, N)) img[p] = alpha8m(img, p, rs, N);
  ret = img[p];
  img[p] = sav;
  return ret;
} /* delta8m() */

/* ==================================== */
uint8_t delta4p( /* retourne la valeur max. a laquelle p est constructible - minima 4-connexes */
  uint8_t *img,          /* pointeur base image */
  index_t p,                       /* index du point */
  index_t rs,                      /* taille rangee */
  index_t N)                       /* taille image */
/* ==================================== */
{	
  uint8_t ret, sav = img[p];
  while (pconstr4(img, p, rs, N)) img[p] = alpha8p(img, p, rs, N);
  ret = img[p];
  img[p] = sav;
  return ret;
} /* delta4p() */

/* ==================================== */
uint8_t delta8p( /* retourne la valeur max. a laquelle p est constructible - minima 8-connexes */
  uint8_t *img,          /* pointeur base image */
  index_t p,                       /* index du point */
  index_t rs,                      /* taille rangee */
  index_t N)                       /* taille image */
/* ==================================== */
{	
  uint8_t ret, sav = img[p];
  while (pconstr8(img, p, rs, N)) img[p] = alpha8p(img, p, rs, N);
  ret = img[p];
  img[p] = sav;
  return ret;
} /* delta8p() */

/* ==================================== */
int32_t pconv4(  /* teste si un point est convergent - minima 4-connexes */
  uint8_t *img,          /* pointeur base image */
  index_t p,                       /* index du point */
  index_t rs,                      /* taille rangee */
  index_t N)                       /* taille image */
/* ==================================== */
{	
	int32_t t4mm, t4m, t8p, t8pp;

        if ((p%rs==rs-1)||(p<rs)||(p%rs==0)||(p>=N-rs)) /* point de bord */
                return 0;

        nbtopo(img, p, rs, N, &t4mm, &t4m, &t8p, &t8pp);
	if (t8pp >= 2)
		return 1;
	return 0;
} /* pconv4() */

/* ==================================== */
int32_t pconv8(   /* teste si un point est convergent - minima 8-connexes */
  uint8_t *img,          /* pointeur base image */
  index_t p,                       /* index du point */
  index_t rs,                      /* taille rangee */
  index_t N)                       /* taille image */
/* ==================================== */
{	
	int32_t t8mm, t8m, t4p, t4pp;

        if ((p%rs==rs-1)||(p<rs)||(p%rs==0)||(p>=N-rs)) /* point de bord */
                return 0;

        nbtopo2(img, p, rs, N, &t8mm, &t8m, &t4p, &t4pp);
	if (t4pp >= 2)
		return 1;
	return 0;
} /* pconv8() */

/* ==================================== */
int32_t pdiv4(  /* teste si un point est divergent - minima 4-connexes */
  uint8_t *img,          /* pointeur base image */
  index_t p,                       /* index du point */
  index_t rs,                      /* taille rangee */
  index_t N)                       /* taille image */
/* ==================================== */
{	
	int32_t t4mm, t4m, t8p, t8pp;

        if ((p%rs==rs-1)||(p<rs)||(p%rs==0)||(p>=N-rs)) /* point de bord */
                return 0;

        nbtopo(img, p, rs, N, &t4mm, &t4m, &t8p, &t8pp);
	if (t4mm >= 2)
		return 1;
	return 0;
} /* pdiv4() */

/* ==================================== */
int32_t pdiv8(   /* teste si un point est divergent - minima 8-connexes */
  uint8_t *img,          /* pointeur base image */
  index_t p,                       /* index du point */
  index_t rs,                      /* taille rangee */
  index_t N)                       /* taille image */
/* ==================================== */
{	
	int32_t t8mm, t8m, t4p, t4pp;

        if ((p%rs==rs-1)||(p<rs)||(p%rs==0)||(p>=N-rs)) /* point de bord */
                return 0;

        nbtopo2(img, p, rs, N, &t8mm, &t8m, &t4p, &t4pp);
	if (t8mm >= 2)
		return 1;
	return 0;
} /* pdiv8() */

/* ==================================== */
int32_t saddle4(                   /* point selle avec la 4-connexite pour les minima */
  uint8_t *img,          /* pointeur base image */
  index_t p,                       /* index du point */
  index_t rs,                      /* taille rangee */
  index_t N)                       /* taille image */
/* ==================================== */
{
	int32_t t4mm,t4m,t8p,t8pp;

        if ((p%rs==rs-1) || (p<rs) || (p%rs==0) || (p>=N-rs)) /* point de bord */
                return 0;

        nbtopo(img, p, rs, N, &t4mm, &t4m, &t8p, &t8pp);
	if ((t8pp > 1) && (t4mm > 1)) return t8pp;
	return 0;
} /* saddle4() */

/* ==================================== */
int32_t saddle8(                   /* point selle avec la 8-connexite pour les minima */
  uint8_t *img,          /* pointeur base image */
  index_t p,                       /* index du point */
  index_t rs,                      /* taille rangee */
  index_t N)                       /* taille image */
/* ==================================== */
{
	int32_t t8mm,t8m,t4p,t4pp;

        if ((p%rs==rs-1) || (p<rs) || (p%rs==0) || (p>=N-rs)) /* point de bord */
                return 0;

        nbtopo2(img, p, rs, N, &t8mm, &t8m, &t4p, &t4pp);
	if ((t4pp > 1) && (t8mm > 1)) return t4pp;
	return 0;
} /* saddle8() */

/* ==================================== */
int32_t lambdadestr4( /* teste si un point est lambda-destructible - minima 4-connexes */
  uint8_t *F,            /* pointeur base image */
  index_t p,                       /* index du point */
  int32_t lambda,                  /* elevation admissible */
  index_t rs,                      /* taille rangee */
  index_t N)                       /* taille image */
/* ==================================== */
/* 
  un point p est lambda-destructible si
    p est destructible OU
    p est un pic ET F(p)-alpha8-(F,p) <= lambda OU
    ( p est k-divergent (ie- t4mm(p) = k >=2) ET
      k - 1 composantes de F4mm sont a une distance 
      inferieure ou egale a lambda de p
    )
*/
{	
  int32_t t4mm, t8p, b, n, i, k, m, mi, q, destr;

  if ((p%rs==rs-1) || (p<rs) || (p%rs==0) || (p>=N-rs)) /* bord */
    return 0;

  nbtopo(F, p, rs, N, &t4mm, &b, &t8p, &n);
  if ((t4mm == 1) && (t8p == 1)) return 1;

#define DETRUIT_LAMBDA_PICS 
#ifdef DETRUIT_LAMBDA_PICS 
  if ((t4mm == 1) && (t8p == 0)) return ((F[p] - alpha8m(F,p,rs,N)) <= lambda);
#endif
  destr = 0;
  if (t4mm >= 2)
  {
    n = 0;
    m = maskmm(F, p, rs, N);
    for (i = 0; i < t4mm; i++) 
    { 
      mi = Comp4Tab[m][i];
      if (mi == 0) break;
      for (k = 0; k < 8; k++)
      {
        if ((mi % 2) != 0)
	{
          q = voisin(p, k, rs, N);
          if ((F[p] - F[q]) > lambda) break; /* composante eloignee */
	}
        mi = mi >> 1;
      } /* for (k = 0; k < 8; k++) */
      if (k == 8) n++;   /* compte le nb de composantes proches */
    } /* for (i = 0; i < t4mm; i++) */
    if (n >= (t4mm - 1)) return 1;
  } /* if (t4mm >= 2) */
  return 0;
} /* lambdadestr4() */

/* ==================================== */
int32_t lambdaconstr4( /* teste si un point est lambda-constructible - minima 4-connexes */
  uint8_t *F,            /* pointeur base image */
  index_t p,                       /* index du point */
  int32_t lambda,                  /* elevation admissible */
  index_t rs,                      /* taille rangee */
  index_t N)                       /* taille image */
/* ==================================== */
/* 
  un point p est lambda-constructible si
    p est constructible OU
    p est un puits ET alpha8+(F,p)-F(p) <= lambda OU
    ( p est k-convergent (ie- t8pp(p) = k >=2) ET
      k - 1 composantes de F8pp sont a une distance 
      inferieure ou egale a lambda de p
    )
*/
{	
  int32_t t4mm, t8p, t4m, t8pp, i, k, n, m, mi, q, constr;

  if ((p%rs==rs-1) || (p<rs) || (p%rs==0) || (p>=N-rs)) /* bord */
    return 0;

  nbtopo(F, p, rs, N, &t4mm, &t4m, &t8p, &t8pp);
  if ((t4m == 1) && (t8pp == 1)) return 1;

#define DETRUIT_LAMBDA_PUITS 
#ifdef DETRUIT_LAMBDA_PUITS 
  if ((t8pp == 1) && (t4m == 0)) return ((alpha8p(F,p,rs,N) - F[p]) <= lambda);
#endif
  constr = 0;
  if (t8pp >= 2)
  {
    n = 0;
    m = maskpp(F, p, rs, N);
    for (i = 0; i < t8pp; i++) 
    { 
      mi = Comp8Tab[m][i];
      if (mi == 0) break;
      for (k = 0; k < 8; k++)
      {
        if ((mi % 2) != 0)
	{
          q = voisin(p, k, rs, N);
          if ((F[q] - F[p]) > lambda) break; /* composante eloignee */
	}
        mi = mi >> 1;
      } /* for (k = 0; k < 8; k++) */
      if (k == 8) n++;   /* compte le nb de composantes proches */
    } /* for (i = 0; i < t8pp; i++) */
    if (n >= (t8pp - 1)) return 1;
  } /* if (t8pp >= 2) */
  return 0;
} /* lambdaconstr4() */

/* ==================================== */
int32_t lambdadestr8( /* teste si un point est lambda-destructible - minima 8-connexes */
  uint8_t *F,            /* pointeur base image */
  index_t p,                       /* index du point */
  int32_t lambda,                  /* elevation admissible */
  index_t rs,                      /* taille rangee */
  index_t N)                       /* taille image */
/* ==================================== */
/* 
  un point p est lambda-destructible si
    p est destructible OU
    p est un pic ET F(p)-alpha8-(F,p) <= lambda OU
    ( p est k-divergent (ie- t8mm(p) = k >=2) ET
      k - 1 composantes de F8mm sont a une distance 
      inferieure ou egale a lambda de p
    )
*/
{	
  int32_t t8mm, t4p, b, n, i, k, m, mi, q, destr;

  if ((p%rs==rs-1) || (p<rs) || (p%rs==0) || (p>=N-rs)) /* bord */
    return 0;

  nbtopo2(F, p, rs, N, &t8mm, &b, &t4p, &n);
  if ((t8mm == 1) && (t4p == 1)) return 1;

#ifdef DETRUIT_LAMBDA_PICS 
  if ((t8mm == 1) && (t4p == 0)) return ((F[p] - alpha8m(F,p,rs,N)) <= lambda);
#endif
  destr = 0;
  if (t8mm >= 2)
  {
    n = 0;
    m = maskmm(F, p, rs, N);
    for (i = 0; i < t8mm; i++) 
    { 
      mi = Comp8Tab[m][i];
      if (mi == 0) break;
      for (k = 0; k < 8; k++)
      {
        if ((mi % 2) != 0)
	{
          q = voisin(p, k, rs, N);
          if ((F[p] - F[q]) > lambda) break; /* composante eloignee */
	}
        mi = mi >> 1;
      } /* for (k = 0; k < 8; k++) */
      if (k == 8) n++;   /* compte le nb de composantes proches */
    } /* for (i = 0; i < t8mm; i++) */
    if (n >= (t8mm - 1)) return 1;
  } /* if (t8mm >= 2) */
  return 0;
} /* lambdadestr8() */

/* ==================================== */
int32_t lambdaconstr8( /* teste si un point est lambda-constructible - minima 8-connexes */
  uint8_t *F,            /* pointeur base image */
  index_t p,                       /* index du point */
  int32_t lambda,                  /* elevation admissible */
  index_t rs,                      /* taille rangee */
  index_t N)                       /* taille image */
/* ==================================== */
/* 
  un point p est lambda-constructible si
    p est constructible OU
    p est un puits ET alpha8+(F,p)-F(p) <= lambda OU
    ( p est k-convergent (ie- t4pp(p) = k >=2) ET
      k - 1 composantes de F4pp sont a une distance 
      inferieure ou egale a lambda de p
    )
*/
{	
  int32_t t8mm, t4p, t8m, t4pp, i, k, n, m, mi, q, constr;

  if ((p%rs==rs-1) || (p<rs) || (p%rs==0) || (p>=N-rs)) /* bord */
    return 0;

  nbtopo2(F, p, rs, N, &t8mm, &t8m, &t4p, &t4pp);
  if ((t8m == 1) && (t4pp == 1)) return 1;

#ifdef DETRUIT_LAMBDA_PUITS 
  if ((t4pp == 1) && (t8m == 0)) return ((alpha8p(F,p,rs,N) - F[p]) <= lambda);
#endif
  constr = 0;
  if (t4pp >= 2)
  {
    n = 0;
    m = maskpp(F, p, rs, N);
    for (i = 0; i < t4pp; i++) 
    { 
      mi = Comp4Tab[m][i];
      if (mi == 0) break;
      for (k = 0; k < 8; k++)
      {
        if ((mi % 2) != 0)
	{
          q = voisin(p, k, rs, N);
          if ((F[q] - F[p]) > lambda) break; /* composante eloignee */
	}
        mi = mi >> 1;
      } /* for (k = 0; k < 8; k++) */
      if (k == 8) n++;   /* compte le nb de composantes proches */
    } /* for (i = 0; i < t4pp; i++) */
    if (n >= (t4pp - 1)) return 1;
  } /* if (t4pp >= 2) */
  return 0;
} /* lambdaconstr8() */

/* ==================================================================== */
/*                    Nombres topologiques en binaire                   */
/* ==================================================================== */

/* ==================================== */
void top8(                     /* pour un objet en 8-connexite */
  uint8_t *img,          /* pointeur base image */
  index_t p,                       /* index du point */
  index_t rs,                      /* taille rangee */
  index_t N,                       /* taille image */
  int32_t *t,
  int32_t *tb)                     /* resultats */
/* ==================================== */
{
	register uint8_t * ptr = img+p;
	register int32_t v;

	assert(!is_on_frame(p, rs, N));

	if ( *(ptr+1)   ) v=1; else v=0;
 	if ( *(ptr+1-rs)) v|=2;
 	if ( *(ptr-rs)  ) v|=4;
 	if ( *(ptr-rs-1)) v|=8;
 	if ( *(ptr-1)   ) v|=16;
 	if ( *(ptr-1+rs)) v|=32;
 	if ( *(ptr+rs)  ) v|=64;
 	if ( *(ptr+rs+1)) v|=128;

	*tb=TopoTab[v][0];
	*t=TopoTab[v][1];
} /* top8() */

/* ==================================== */
void top4(                     /* pour un objet en 4-connexite */
  uint8_t *img,          /* pointeur base image */
  index_t p,                       /* index du point */
  index_t rs,                      /* taille rangee */
  index_t N,                       /* taille image */
  int32_t *t,
  int32_t *tb)                     /* resultats */
/* ==================================== */
{
	register uint8_t * ptr = img+p;
	register int32_t v;

	assert(!is_on_frame(p, rs, N));

	if ( *(ptr+1)   ) v=1; else v=0;
 	if ( *(ptr+1-rs)) v|=2;
 	if ( *(ptr-rs)  ) v|=4;
 	if ( *(ptr-rs-1)) v|=8;
 	if ( *(ptr-1)   ) v|=16;
 	if ( *(ptr-1+rs)) v|=32;
 	if ( *(ptr+rs)  ) v|=64;
 	if ( *(ptr+rs+1)) v|=128;
        v = ~v & 0xff;

	*t=TopoTab[v][0];
	*tb=TopoTab[v][1];
} /* top4() */

/* ==================================== */
void top8_l(                   /* pour un objet en 8-connexite */
  int32_t *img,          /* pointeur base image */
  index_t p,                       /* index du point */
  index_t rs,                      /* taille rangee */
  index_t N,                       /* taille image */
  int32_t *t,
  int32_t *tb)                     /* resultats */
/* ==================================== */
{
	register int32_t * ptr = img+p;
	register int32_t v;

	assert(!is_on_frame(p, rs, N));

	if ( *(ptr+1)   ) v=1; else v=0;
 	if ( *(ptr+1-rs)) v|=2;
 	if ( *(ptr-rs)  ) v|=4;
 	if ( *(ptr-rs-1)) v|=8;
 	if ( *(ptr-1)   ) v|=16;
 	if ( *(ptr-1+rs)) v|=32;
 	if ( *(ptr+rs)  ) v|=64;
 	if ( *(ptr+rs+1)) v|=128;

	*tb=TopoTab[v][0];
	*t=TopoTab[v][1];
} /* top8_l() */

/* ==================================== */
void top4_l(                   /* pour un objet en 4-connexite */
  int32_t *img,          /* pointeur base image */
  index_t p,                       /* index du point */
  index_t rs,                      /* taille rangee */
  index_t N,                       /* taille image */
  int32_t *t,
  int32_t *tb)                     /* resultats */
/* ==================================== */
{
	register int32_t * ptr = img+p;
	register int32_t v;

	assert(!is_on_frame(p, rs, N));

	if ( *(ptr+1)   ) v=1; else v=0;
 	if ( *(ptr+1-rs)) v|=2;
 	if ( *(ptr-rs)  ) v|=4;
 	if ( *(ptr-rs-1)) v|=8;
 	if ( *(ptr-1)   ) v|=16;
 	if ( *(ptr-1+rs)) v|=32;
 	if ( *(ptr+rs)  ) v|=64;
 	if ( *(ptr+rs+1)) v|=128;
        v = ~v & 0xff;

	*t=TopoTab[v][0];
	*tb=TopoTab[v][1];
} /* top4_l() */

/* ==================================== */
int32_t simple8(                   /* pour un objet en 8-connexite */
  uint8_t *img,          /* pointeur base image */
  index_t p,                       /* index du point */
  index_t rs,                      /* taille rangee */
  index_t N)                       /* taille image */
/* ==================================== */
{
        int32_t t, tb;
        if ((p%rs==rs-1) || (p<rs) || (p%rs==0) || (p>=N-rs)) return(0);
        top8(img, p, rs, N, &t, &tb);
        return (t == 1) && (tb == 1);
} /* simple8() */

/* ==================================== */
int32_t simple4(                   /* pour un objet en 4-connexite */
  uint8_t *img,          /* pointeur base image */
  index_t p,                       /* index du point */
  index_t rs,                      /* taille rangee */
  index_t N)                       /* taille image */
/* ==================================== */
{
        int32_t t, tb;
        if ((p%rs==rs-1) || (p<rs) || (p%rs==0) || (p>=N-rs)) return(0);
        top4(img, p, rs, N, &t, &tb);
        return (t == 1) && (tb == 1);
} /* simple4() */

/* ==================================== */
int32_t typetopobin( /* types topo. en binaire et en 4-connexite */
  uint8_t *img,          /* pointeur base image */
  index_t p,                       /* index du point */
  index_t rs,                      /* taille rangee */
  index_t N)                       /* taille image */
/* ==================================== */
{
  int32_t t, tb;
  if (img[p] == 0) return 0;
  if ((p%rs==rs-1) || (p<rs) || (p%rs==0) || (p>=N-rs)) return(0);
  top4(img, p, rs, N, &t, &tb);
  if ((t == 1) && (tb == 1)) return SIMPLE;
  if (t > 2) return MULTIPLE;
  if (t > 1) return SEPARANT;
  if (t == 0) return ISOLE;
  if (tb == 0) return INTERIEUR;
  return 0;
} /* typetopobin() */

/* ==================================== */
int32_t typetopobin8( /* types topo. en binaire et en 8-connexite */
  uint8_t *img,          /* pointeur base image */
  index_t p,                       /* index du point */
  index_t rs,                      /* taille rangee */
  index_t N)                       /* taille image */
/* ==================================== */
{
  int32_t t, tb;
  if (img[p] == 0) return 0;
  if ((p%rs==rs-1) || (p<rs) || (p%rs==0) || (p>=N-rs)) return(0);
  top8(img, p, rs, N, &t, &tb);
  if ((t == 1) && (tb == 1)) return SIMPLE;
  if (t > 2) return MULTIPLE;
  if (t > 1) return SEPARANT;
  if (t == 0) return ISOLE;
  if (tb == 0) return INTERIEUR;
  return 0;
} /* typetopobin8() */

/* ==================================== */
int32_t t8(int32_t v) /* pour un objet en 8-connexite - v est le masque binaire du voisinage */
/* ==================================== */
{
  return TopoTab[v][1];
}

/* ==================================== */
int32_t t8b(int32_t v) /* pour un objet en 8-connexite - v est le masque binaire du voisinage */
/* ==================================== */
{
  v = ~v & 0xff;
  return TopoTab[v][1];
}

/* ==================================== */
int32_t t4(int32_t v) /* pour un objet en 4-connexite - v est le masque binaire du voisinage */
/* ==================================== */
{
  v = ~v & 0xff;
  return TopoTab[v][0];
}

/* ==================================== */
int32_t t4b(int32_t v) /* pour un objet en 4-connexite - v est le masque binaire du voisinage */
/* ==================================== */
{
  return TopoTab[v][0];
}

/* ==================================== */
int32_t nbvois8(
  uint8_t *img,          /* pointeur base image */
  index_t p,                       /* index du point */
  index_t rs,                      /* taille rangee */
  index_t N)                       /* taille image */
/* retourne le nombre de 8-voisins de niveau non nul */
/* ==================================== */
{
	register uint8_t * ptr = img+p;
	register int32_t n = 0;

        if ((p%rs!=rs-1) && (*(ptr+1))) n++;
        if (((p%rs!=rs-1)&&(p>=rs)) && (*(ptr+1-rs))) n++;
        if ((p>=rs) && (*(ptr-rs))) n++;
        if (((p>=rs)&&(p%rs!=0)) && (*(ptr-rs-1))) n++;
        if ((p%rs!=0) && (*(ptr-1))) n++;
        if (((p%rs!=0)&&(p<N-rs)) && (*(ptr-1+rs))) n++;
        if ((p<N-rs) && (*(ptr+rs))) n++;
        if (((p<N-rs)&&(p%rs!=rs-1)) && (*(ptr+rs+1))) n++;
        return n;
} /* nbvois8() */

/* ==================================== */
int32_t nbvois4(
  uint8_t *img,          /* pointeur base image */
  index_t p,                       /* index du point */
  index_t rs,                      /* taille rangee */
  index_t N)                       /* taille image */
/* retourne le nombre de 4-voisins de niveau non nul */
/* ==================================== */
{
	register uint8_t * ptr = img+p;
	register int32_t n = 0;

        if ((p%rs!=rs-1) && (*(ptr+1))) n++;
        if ((p>=rs) && (*(ptr-rs))) n++;
        if ((p%rs!=0) && (*(ptr-1))) n++;
        if ((p<N-rs) && (*(ptr+rs))) n++;
        return n;
} /* nbvois4() */

/* ==================================== */
int32_t nbvoisc8(
  uint8_t *img,          /* pointeur base image */
  index_t p,                       /* index du point */
  index_t rs,                      /* taille rangee */
  index_t N)                       /* taille image */
/* retourne le nombre de 8-voisins de niveau nul */
/* ==================================== */
{
	register uint8_t * ptr = img+p;
	register int32_t n = 0;

        if ((p%rs!=rs-1) && (*(ptr+1)==0)) n++;
        if (((p%rs!=rs-1)&&(p>=rs)) && (*(ptr+1-rs)==0)) n++;
        if ((p>=rs) && (*(ptr-rs)==0)) n++;
        if (((p>=rs)&&(p%rs!=0)) && (*(ptr-rs-1)==0)) n++;
        if ((p%rs!=0) && (*(ptr-1)==0)) n++;
        if (((p%rs!=0)&&(p<N-rs)) && (*(ptr-1+rs)==0)) n++;
        if ((p<N-rs) && (*(ptr+rs)==0)) n++;
        if (((p<N-rs)&&(p%rs!=rs-1)) && (*(ptr+rs+1)==0)) n++;
        return n;
} /* nbvoisc8() */

/* ==================================== */
int32_t nbvoisc4(
  uint8_t *img,          /* pointeur base image */
  index_t p,                       /* index du point */
  index_t rs,                      /* taille rangee */
  index_t N)                       /* taille image */
/* retourne le nombre de 4-voisins de niveau nul */
/* ==================================== */
{
	register uint8_t * ptr = img+p;
	register int32_t n = 0;

        if ((p%rs!=rs-1) && (*(ptr+1)==0)) n++;
        if ((p>=rs) && (*(ptr-rs)==0)) n++;
        if ((p%rs!=0) && (*(ptr-1)==0)) n++;
        if ((p<N-rs) && (*(ptr+rs)==0)) n++;
        return n;
} /* nbvoisc4() */

/* ==================================== */
int32_t nbvois8neq(
  int32_t *img,          /* pointeur base image */
  index_t p,                       /* index du point */
  index_t rs,                      /* taille rangee */
  index_t N)                       /* taille image */
/* retourne le nombre de 8-voisins de niveau different de celui du point p */
/* ==================================== */
{
	register int32_t * ptr = img+p;
        register int32_t pval = *ptr;
	register int32_t n = 0;

        if (!((p%rs!=rs-1) && (*(ptr+1) == pval))) n++;
        if (!(((p%rs!=rs-1)&&(p>=rs)) && (*(ptr+1-rs) == pval))) n++;
        if (!((p>=rs) && (*(ptr-rs) == pval))) n++;
        if (!(((p>=rs)&&(p%rs!=0)) && (*(ptr-rs-1) == pval))) n++;
        if (!((p%rs!=0) && (*(ptr-1) == pval))) n++;
        if (!(((p%rs!=0)&&(p<N-rs)) && (*(ptr-1+rs) == pval))) n++;
        if (!((p<N-rs) && (*(ptr+rs) == pval))) n++;
        if (!(((p<N-rs)&&(p%rs!=rs-1)) && (*(ptr+rs+1) == pval))) n++;
        return n;
} /* nbvois8neq() */

/* ==================================== */
int32_t nbvois4neq(
  int32_t *img,          /* pointeur base image */
  index_t p,                       /* index du point */
  index_t rs,                      /* taille rangee */
  index_t N)                       /* taille image */
/* retourne le nombre de 4-voisins de niveau different de celui du point p */
/* ==================================== */
{
	register int32_t * ptr = img+p;
        register int32_t pval = *ptr;
	register int32_t n = 0;

        if (!((p%rs!=rs-1) && (*(ptr+1) == pval))) n++;
        if (!((p>=rs) && (*(ptr-rs) == pval))) n++;
        if (!((p%rs!=0) && (*(ptr-1) == pval))) n++;
        if (!((p<N-rs) && (*(ptr+rs) == pval))) n++;
        return n;
} /* nbvois4neq() */

/* ==================================== */
int32_t nbvoisp8(
  uint8_t *img,          /* pointeur base image */
  index_t p,                       /* index du point */
  index_t rs,                      /* taille rangee */
  index_t N)                       /* taille image */
/* retourne le nb de 8-voisins de niveau superieur ou egal a celui du point p */
/* ==================================== */
{
	register uint8_t val = *(img+p);
	register uint8_t * ptr = img+p;
	register int32_t n = 0;

        if ((p%rs!=rs-1) && (*(ptr+1) >= val)) n++;
        if (((p%rs!=rs-1)&&(p>=rs)) && (*(ptr+1-rs) >= val)) n++;
        if ((p>=rs) && (*(ptr-rs) >= val)) n++;
        if (((p>=rs)&&(p%rs!=0)) && (*(ptr-rs-1) >= val)) n++;
        if ((p%rs!=0) && (*(ptr-1) >= val)) n++;
        if (((p%rs!=0)&&(p<N-rs)) && (*(ptr-1+rs) >= val)) n++;
        if ((p<N-rs) && (*(ptr+rs) >= val)) n++;
        if (((p<N-rs)&&(p%rs!=rs-1)) && (*(ptr+rs+1) >= val)) n++;
        return n;
} /* nbvoisp8() */

/* ==================================== */
int32_t nbvoisp4(
  uint8_t *img,          /* pointeur base image */
  index_t p,                       /* index du point */
  index_t rs,                      /* taille rangee */
  index_t N)                       /* taille image */
/* retourne le nb de 4-voisins de niveau superieur ou egal a celui du point p */
/* ==================================== */
{
	register uint8_t val = *(img+p);
	register uint8_t * ptr = img+p;
	register int32_t n = 0;

        if ((p%rs!=rs-1) && (*(ptr+1) >= val)) n++;
        if ((p>=rs) && (*(ptr-rs) >= val)) n++;
        if ((p%rs!=0) && (*(ptr-1) >= val)) n++;
        if ((p<N-rs) && (*(ptr+rs) >= val)) n++;
        return n;
} /* nbvoisp4() */

/* ==================================== */
int32_t nbvoism8(
  uint8_t *img,          /* pointeur base image */
  index_t p,                       /* index du point */
  index_t rs,                      /* taille rangee */
  index_t N)                       /* taille image */
/* retourne le nb de 8-voisins de niveau inferieur ou egal a celui du point p */
/* ==================================== */
{
	register uint8_t val = *(img+p);
	register uint8_t * ptr = img+p;
	register int32_t n = 0;

        if ((p%rs!=rs-1) && (*(ptr+1) <= val)) n++;
        if (((p%rs!=rs-1)&&(p>=rs)) && (*(ptr+1-rs) <= val)) n++;
        if ((p>=rs) && (*(ptr-rs) <= val)) n++;
        if (((p>=rs)&&(p%rs!=0)) && (*(ptr-rs-1) <= val)) n++;
        if ((p%rs!=0) && (*(ptr-1) <= val)) n++;
        if (((p%rs!=0)&&(p<N-rs)) && (*(ptr-1+rs) <= val)) n++;
        if ((p<N-rs) && (*(ptr+rs) <= val)) n++;
        if (((p<N-rs)&&(p%rs!=rs-1)) && (*(ptr+rs+1) <= val)) n++;
        return n;
} /* nbvoism8() */

/* ==================================== */
int32_t nbvoism4(
  uint8_t *img,          /* pointeur base image */
  index_t p,                       /* index du point */
  index_t rs,                      /* taille rangee */
  index_t N)                       /* taille image */
/* retourne le nb de 4-voisins de niveau inferieur ou egal a celui du point p */
/* ==================================== */
{
	register uint8_t val = *(img+p);
	register uint8_t * ptr = img+p;
	register int32_t n = 0;

        if ((p%rs!=rs-1) && (*(ptr+1) <= val)) n++;
        if ((p>=rs) && (*(ptr-rs) <= val)) n++;
        if ((p%rs!=0) && (*(ptr-1) <= val)) n++;
        if ((p<N-rs) && (*(ptr+rs) <= val)) n++;
        return n;
} /* nbvoism4() */

/* ==================================== */
int32_t extremite8(
  uint8_t *img,          /* pointeur base image */
  index_t p,                       /* index du point */
  index_t rs,                      /* taille rangee */
  index_t N)                       /* taille image */
/* retourne 1 si le point p est un point extremite de 8-courbe, 0 sinon */
/* un point extremite de 8-courbe est un point p qui a extactement */
/* un 8-voisin de niveau superieur ou egal a celui du point p */
/* ==================================== */
{
	register uint8_t val = *(img+p);
	register uint8_t * ptr = img+p;
	register int32_t n = 0;

        if ((p%rs!=rs-1) && (*(ptr+1) >= val)) n++;
        if (((p%rs!=rs-1)&&(p>=rs)) && (*(ptr+1-rs) >= val)) n++;
        if (n > 1) return 0;
        if ((p>=rs) && (*(ptr-rs) >= val)) n++;
        if (n > 1) return 0;
        if (((p>=rs)&&(p%rs!=0)) && (*(ptr-rs-1) >= val)) n++;
        if (n > 1) return 0;
        if ((p%rs!=0) && (*(ptr-1) >= val)) n++;
        if (n > 1) return 0;
        if (((p%rs!=0)&&(p<N-rs)) && (*(ptr-1+rs) >= val)) n++;
        if (n > 1) return 0;
        if ((p<N-rs) && (*(ptr+rs) >= val)) n++;
        if (n > 1) return 0;
        if (((p<N-rs)&&(p%rs!=rs-1)) && (*(ptr+rs+1) >= val)) n++;
        if (n > 1) return 0;
        if (n < 1) return 0;
        return 1;
} /* extremite8() */

/* ==================================== */
int32_t extremite4(
  uint8_t *img,          /* pointeur base image */
  index_t p,                       /* index du point */
  index_t rs,                      /* taille rangee */
  index_t N)                       /* taille image */
/* retourne 1 si le point p est un point extremite de 4-courbe, 0 sinon */
/* un point extremite de 4-courbe est un point p qui a extactement */
/* un 4-voisin de niveau superieur ou egal a celui du point p */
/* ==================================== */
{
	register uint8_t val = *(img+p);
	register uint8_t * ptr = img+p;
	register int32_t n = 0;

        if ((p%rs!=rs-1) && (*(ptr+1) >= val)) n++;
        if ((p>=rs) && (*(ptr-rs) >= val)) n++;
        if (n > 1) return 0;
        if ((p%rs!=0) && (*(ptr-1) >= val)) n++;
        if (n > 1) return 0;
        if ((p<N-rs) && (*(ptr+rs) >= val)) n++;
        if (n > 1) return 0;
        if (n < 1) return 0;
        return 1;
} /* extremite4() */

/* ==================================== */
int32_t extremite8m(
  uint8_t *img,          /* pointeur base image */
  index_t p,                       /* index du point */
  index_t rs,                      /* taille rangee */
  index_t N)                       /* taille image */
/* retourne 1 si le point p est un point extremite de 8-courbe, 0 sinon */
/* un point extremite de 8-courbe est un point p qui a extactement */
/* un 8-voisin de niveau inferieur ou egal a celui du point p */
/* ==================================== */
{
	register uint8_t val = *(img+p);
	register uint8_t * ptr = img+p;
	register int32_t n = 0;

        if ((p%rs!=rs-1) && (*(ptr+1) <= val)) n++;
        if (((p%rs!=rs-1)&&(p>=rs)) && (*(ptr+1-rs) <= val)) n++;
        if (n > 1) return 0;
        if ((p>=rs) && (*(ptr-rs) <= val)) n++;
        if (n > 1) return 0;
        if (((p>=rs)&&(p%rs!=0)) && (*(ptr-rs-1) <= val)) n++;
        if (n > 1) return 0;
        if ((p%rs!=0) && (*(ptr-1) <= val)) n++;
        if (n > 1) return 0;
        if (((p%rs!=0)&&(p<N-rs)) && (*(ptr-1+rs) <= val)) n++;
        if (n > 1) return 0;
        if ((p<N-rs) && (*(ptr+rs) <= val)) n++;
        if (n > 1) return 0;
        if (((p<N-rs)&&(p%rs!=rs-1)) && (*(ptr+rs+1) <= val)) n++;
        if (n > 1) return 0;
        if (n < 1) return 0;
        return 1;
} /* extremite8m() */

/* ==================================== */
int32_t extremite4m(
  uint8_t *img,          /* pointeur base image */
  index_t p,                       /* index du point */
  index_t rs,                      /* taille rangee */
  index_t N)                       /* taille image */
/* retourne 1 si le point p est un point extremite de 4-courbe, 0 sinon */
/* un point extremite de 4-courbe est un point p qui a extactement */
/* un 4-voisin de niveau inferieur ou egal a celui du point p */
/* ==================================== */
{
	register uint8_t val = *(img+p);
	register uint8_t * ptr = img+p;
	register int32_t n = 0;

        if ((p%rs!=rs-1) && (*(ptr+1) <= val)) n++;
        if ((p>=rs) && (*(ptr-rs) <= val)) n++;
        if (n > 1) return 0;
        if ((p%rs!=0) && (*(ptr-1) <= val)) n++;
        if (n > 1) return 0;
        if ((p<N-rs) && (*(ptr+rs) <= val)) n++;
        if (n > 1) return 0;
        if (n < 1) return 0;
        return 1;
} /* extremite4m() */

/* ==================================== */
int32_t ridge(
  uint8_t *img,          /* pointeur base image */
  index_t p,                       /* index du point */
  index_t rs,                      /* taille rangee */
  index_t N)                       /* taille image */
/* 
  detection des points "ridge" (d'apres Arcelli)  
*/
/* ==================================== */
{
	register uint8_t val = *(img+p);
	register uint8_t * ptr = img+p;
	register uint8_t v0;
	register uint8_t v1;
	register uint8_t v2;
	register uint8_t v3;
	register uint8_t v4;
	register uint8_t v5;
	register uint8_t v6;
	register uint8_t v7;

	assert(!is_on_frame(p, rs, N));

        v0 = *(ptr+1)   ;
        v1 = *(ptr+1-rs);
        v2 = *(ptr-rs)  ;
        v3 = *(ptr-rs-1);
        v4 = *(ptr-1)   ;
        v5 = *(ptr-1+rs);
        v6 = *(ptr+rs)  ;
        v7 = *(ptr+rs+1);

        if (mcmin(v1,val) > mcmax(v0,v2)) return 1; 
        if (mcmin(v3,val) > mcmax(v2,v4)) return 1; 
        if (mcmin(v5,val) > mcmax(v4,v6)) return 1; 
        if (mcmin(v7,val) > mcmax(v6,v0)) return 1; 
        if (mcmin(v0,mcmin(v4,val)) > mcmax(v2,v6)) return 1; 
        if (mcmin(v2,mcmin(v6,val)) > mcmax(v0,v4)) return 1; 

        return 0;
} /* ridge() */

/* ==================================== */
int32_t bordext4(uint8_t *F, index_t x, index_t rs, index_t N)
/* ==================================== */
/* teste si x a un 4-voisin a 0 */
{
  index_t k, y;
  for (k = 0; k < 8; k += 2) /* parcourt les voisins en 4-connexite */
  {
    y = voisin(x, k, rs, N);
    if ((y != -1) && (F[y] == 0)) return 1;
  } /* for k */      
  return 0;
} /* bordext4() */

/* ==================================== */
int32_t bordext8(uint8_t *F, index_t x, index_t rs, index_t N)
/* ==================================== */
/* teste si x a un 8-voisin a 0 */
{
  index_t k, y;
  for (k = 0; k < 8; k += 1) /* parcourt les voisins en 8-connexite */
  {
    y = voisin(x, k, rs, N);
    if ((y != -1) && (F[y] == 0)) return 1;
  } /* for k */      
  return 0;
} /* bordext8() */

/* ==================================== */
int32_t curve4( /* point de courbe en 4-connexite */
  uint8_t *img,          /* pointeur base image */
  index_t p,                       /* index du point */
  index_t rs,                      /* taille rangee */
  index_t N)                       /* taille image */
/* ==================================== */
{
  int32_t t, tb;
  if (img[p] == 0) return 0;
  if ((p%rs==rs-1) || (p<rs) || (p%rs==0) || (p>=N-rs)) return(0);
  top4(img, p, rs, N, &t, &tb);
  if ((t == 2) && (nbvois4(img, p, rs, N) == 2)) return 1;
  return 0;
} /* curve4() */

/* ==================================== */
int32_t curve8( /* point de courbe en 8-connexite */
  uint8_t *img,          /* pointeur base image */
  index_t p,                       /* index du point */
  index_t rs,                      /* taille rangee */
  index_t N)                       /* taille image */
/* ==================================== */
{
  int32_t t, tb;
  if (img[p] == 0) return 0;
  if ((p%rs==rs-1) || (p<rs) || (p%rs==0) || (p>=N-rs)) return(0);
  top8(img, p, rs, N, &t, &tb);
  if ((t == 2) && (nbvois8(img, p, rs, N) == 2)) return 1;
  return 0;
} /* curve8() */

#ifdef PSIMPLE
/* ========================================== */
uint8_t P_simple4(uint8_t *X, uint8_t *P)
/* ========================================== */
/*
  d'apres: "Some topological properties of surfaces in Z3", G. Bertrand & R. Malgouyres
           Theoreme 6
*/
{
  int32_t n;
  uint8_t R[9];

  /* teste la condition 2 (theoreme 6) */
  m = mask(X, 4, 3, 9);
  if (t8b(m) != 1) return 0; 

  /* teste la condition 4 (theoreme 6) */
      for (n = 0; n < x->n26v; n++)
      {
        yp = xp->v26[n];
        if (yp->val)
        {
          yc = xc->v26[n];
          v = yc->val;
          yc->val = 1;
          if (T26(cubec) != 1) return 0;
          yc->val = v;
        } /* if (yp->val) */
      } /* for (n = 0; n < x->n26v; n++) */
  
  for (n = 0; n < 27; n++) /* calcule et range dans cubec l'ensemble R = X - P  */
  {
    y = &(cube[n]);
    yp = &(cubep[n]);
    yc = &(cubec[n]);
    if (y->val && !yp->val) yc->val = 1; else yc->val = 0;
  } /* for (n = 0; n < 27; n++) */

  /* teste la condition 1 (theoreme 6) */
  if (T6(cubec) != 1) return 0;
  
  /* teste la condition 3 (theoreme 6) */
      for (n = 0; n < x->n6v; n++)
      {
        yp = xp->v6[n];
        if (yp->val)
        {
          yc = xc->v6[n];
          v = yc->val;
          yc->val = 1;
          if (T6(cubec) != 1) return 0;
          yc->val = v;
        } /* if (yp->val) */
      } /* for (n = 0; n < x->n6v; n++) */
  return 1;
} /* P_simple4() */
#endif

/* ==================================== */
int32_t separant4(  /* teste si un point est separant - minima 4-connexes
      ie- s'il est separant pour une coupe c telle que 0 < c <= img[p] 
      retourne c, ou 0 si le point n'est pas separant */
  uint8_t *img,          /* pointeur base image */
  index_t p,                       /* index du point */
  index_t rs,                      /* taille rangee */
  index_t N)                       /* taille image */
/* ==================================== */
{	
  int32_t t4mm, t4m, t8p, t8pp;
  index_t k, q;

  if ((p%rs==rs-1)||(p<rs)||(p%rs==0)||(p>=N-rs)) /* point de bord */
     return 0;
  nbtopo(img, p, rs, N, &t4mm, &t4m, &t8p, &t8pp);
  if (t4mm >= 2) return img[p];
  for (k = 0; k < 8; k += 1)
  {
    q = voisin(p, k, rs, N);
    if ((q != -1) && (img[q] < img[p]))
    {
      nbtopoh(img, p, img[q], rs, N, &t4mm, &t4m, &t8p, &t8pp);
      if (t4mm >= 2) return img[q];
    }
  }	
  return 0;
} /* separant4() */

/* ==================================== */
int32_t separant4h(  /* teste si un point est separant pour la coupe h - minima 4-connexes */
  uint8_t *img,          /* pointeur base image */
  index_t p,                       /* index du point */
  uint8_t h,             /* hauteur de la coupe */
  index_t rs,                      /* taille rangee */
  index_t N)                       /* taille image */
/* ==================================== */
{	
  int32_t t4mm, t4m, t8p, t8pp;

  if ((p%rs==rs-1)||(p<rs)||(p%rs==0)||(p>=N-rs)) /* point de bord */
     return 0;
  nbtopoh(img, p, h, rs, N, &t4mm, &t4m, &t8p, &t8pp);
  if (t4mm >= 2) return 1;
  return 0;
} /* separant4h() */

/* ==================================== */
int32_t hseparant4(  /* teste si un point est h-separant - minima 4-connexes
	         ie- s'il est separant pour une coupe c telle que h < c <= img[p] */
  uint8_t *img,          /* pointeur base image */
  index_t p,                       /* index du point */
  int32_t h,                       /* parametre */
  index_t rs,                      /* taille rangee */
  index_t N)                       /* taille image */
/* ==================================== */
{	
  int32_t t4mm, t4m, t8p, t8pp;
  index_t k, q;

  if ((p%rs==rs-1)||(p<rs)||(p%rs==0)||(p>=N-rs)) /* point de bord */
     return 0;
  if (img[p] <= h) return 0;
  nbtopo(img, p, rs, N, &t4mm, &t4m, &t8p, &t8pp);
  if (t4mm >= 2) return 1;
  for (k = 0; k < 8; k += 1)
  {
    q = voisin(p, k, rs, N);
    if ((q != -1) && (img[q] > h) && (img[q] < img[p]))
    {
      nbtopoh(img, p, img[q], rs, N, &t4mm, &t4m, &t8p, &t8pp);
      if (t4mm >= 2) return 1;
    }
  }	
  return 0;
} /* hseparant4() */

/* ==================================== */
int32_t separant8(  /* teste si un point est separant - minima 8-connexes
      ie- s'il est separant pour une coupe c telle que 0 < c <= img[p] 
      retourne c, ou 0 si le point n'est pas separant */
  uint8_t *img,          /* pointeur base image */
  index_t p,                       /* index du point */
  index_t rs,                      /* taille rangee */
  index_t N)                       /* taille image */
/* ==================================== */
{
  int32_t t8mm, t8m, t4p, t4pp;
  index_t k, q;

  if ((p%rs==rs-1)||(p<rs)||(p%rs==0)||(p>=N-rs)) /* point de bord */
     return 0;
  nbtopo2(img, p, rs, N, &t8mm, &t8m, &t4p, &t4pp);
  if (t8mm >= 2) return img[p];
  for (k = 0; k < 8; k += 1)
  {
    q = voisin(p, k, rs, N);
    if ((q != -1) && (img[q] < img[p]))
    {
      nbtopoh2(img, p, img[q], rs, N, &t8mm, &t8m, &t4p, &t4pp);
      if (t8mm >= 2) return img[q];
    }
  }	
  return 0;
} /* separant8() */

/* ==================================== */
int32_t museparant4(  
  uint8_t *F,            /* pointeur base image */
  index_t p,                       /* index du point */
  index_t rs,                      /* taille rangee */
  index_t N,                       /* taille image */ 
  int32_t mu)
/* ==================================== */
/* 
   teste si un point est mu-separant - minima 4-connexes
   ie- s'il est separant pour une coupe c telle que c <= F(p) 
   et si pour au moins 2 des k composantes C1..Ck de FcBar, 
   |F(p) - F(Ci)| >= mu
*/
{	
  int32_t t4mm, t4m, t8p, t8pp;
  int32_t i, k, c, m, mi, d, dmax, n;
  index_t q;

  if ((p%rs==rs-1)||(p<rs)||(p%rs==0)||(p>=N-rs)) /* point de bord */
     return 0;
  nbtopo(F, p, rs, N, &t4mm, &t4m, &t8p, &t8pp);
  c = F[p];
  if (t4mm >= 2) goto step2;
  for (k = 0; k < 8; k += 1)
  {
    q = voisin(p, k, rs, N);
    c = F[q];
    if ((q != -1) && (c < F[p]))
    {
      nbtopoh(F, p, c, rs, N, &t4mm, &t4m, &t8p, &t8pp);
      if (t4mm >= 2) goto step2;
    }
  }	
  return 0;
step2:
  n = 0;
  m = maskmmh(F, p, c, rs, N);
  for (i = 0; i < t4mm; i++) 
  { 
    mi = Comp4Tab[m][i];
    if (mi == 0) break;
    dmax = 0;
    for (k = 0; k < 8; k++)
    {
      if (mi % 2)
      {
        q = voisin(p, k, rs, N);
        d = F[p] - F[q];
        if (dmax < d) dmax = d;
      }
      mi = mi >> 1;
    } /* for (k = 0; k < 8; k++) */
    if (dmax >= mu) n++;
  } /* for (i = 0; i < t4mm; i++) */
  if (n >= 2) return 1;
  return 0;
} /* museparant4() */

/* ==================================== */
int32_t nbtrans8(  
  uint8_t *F,            /* pointeur base image */
  index_t p,                       /* index du point */
  index_t rs,                      /* taille rangee */
  index_t N)                       /* taille image */ 
/* ==================================== */
/* 
  compte le nombre de transitions noir-blanc en parcourant les 8 voisins de p
  retourne -1 en cas de point de bord
*/
{
  int32_t k, r, n = 0;
  index_t q;
  q = voisin(p, 0, rs, N);
  if (q == -1) return -1;
  for (k = 1; k < 8; k += 1)
  {
    r = voisin(p, k, rs, N);
    if (r == -1) return -1;
    if (F[r] && !F[q]) n++;
    q = r;
  }	
  r = voisin(p, 0, rs, N);
  if (F[r] && !F[q]) n++;
  return n;
}

#ifdef GENTABSIMPLES
int32_t main()
{
  int32_t i, v, t, tb, n;

#define CONNEX4
#define CONNEX8
#ifdef CONNEX4
  n = 0;
  printf("4-connexite: \n");
  for (i=0; i < 256; i++)
  {
    v = ~i & 0xff;
    t=TopoTab[v][0];
    tb=TopoTab[v][1];
    if ((t == 1) && (tb == 1))
    { printf("%x : simple\n",i); n++; }
    else
      printf("%x : non simple\n",i);
  }
  printf("nb. 4-simples : %d\n", n);
#endif

#ifdef CONNEX8
  n = 0;
  printf("8-connexite: \n");
  for (i=0; i < 256; i++)
  {
    t=TopoTab[i][0];
    tb=TopoTab[i][1];
    if ((t == 1) && (tb == 1))
    { printf("%x : simple\n",i); n++; }
    else
      printf("%x : non simple\n",i);
  }
  printf("nb. 8-simples : %d\n", n);
#endif
}
#endif
