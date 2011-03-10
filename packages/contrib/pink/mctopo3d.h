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
#ifdef __cplusplus
extern "C" {
#endif

#ifndef _MCIMAGE_H
#include <mcimage.h>
#endif

typedef enum {centre, face, arete, coin} typevoxel;

typedef struct VOXEL {
  /* nombres de voisins selon le type d'adjacence */
  uint8_t n6v;       /* les faces */
  uint8_t n12v;      /* les aretes */
  uint8_t n8v;       /* les coins */
  uint8_t n18v;      /* les faces et les aretes */
  uint8_t n26v;      /* les faces et les aretes et les coins */
  /* pointeurs sur les voxels voisins selon le type d'adjacence */
  struct VOXEL * v6[6];     /* les 6-voisins */
  struct VOXEL * v12[12];   /* les purs 18-voisins */
  struct VOXEL * v8[8];     /* les purs 26-voisins */
  struct VOXEL * v18[18];   /* les 18-voisins au sens large */
  struct VOXEL * v26[26];   /* les 26-voisins au sens large */
  /* champs de donnees */
  uint8_t val;
  uint8_t lab;
  uint8_t lab2;
  uint8_t lab3;
  uint8_t lab4;
  uint8_t lab5;
  /* divers */
  typevoxel type; /* peut prendre les valeurs : centre, face, arete, coin */
  uint8_t x,y,z;    /* coordonnes du point */
  index_t n;        /* indice du point dans le tableau */
} voxel;

typedef voxel * pvoxel;

#define encode(x,y,z) ((x)+(y)*3+(z)*9)

extern void mctopo3d_init_topo3d();
extern void mctopo3d_termine_topo3d();
extern uint32_t mctopo3d_encodecube();
extern void mctopo3d_construitcube(voxel * cube);
extern void mctopo3d_top6(
  uint8_t *img,          /* pointeur base image */
  index_t p,                       /* index du point */
  index_t rs,                      /* taille rangee */
  index_t ps,                      /* taille plan */
  index_t N,                       /* taille image */
  int32_t *t,
  int32_t *tb);                    /* resultats */
extern void mctopo3d_top18(
  uint8_t *img,          /* pointeur base image */
  index_t p,                       /* index du point */
  index_t rs,                      /* taille rangee */
  index_t ps,                      /* taille plan */
  index_t N,                       /* taille image */
  int32_t *t,
  int32_t *tb);                    /* resultats */
extern void mctopo3d_top26(
  uint8_t *img,          /* pointeur base image */
  index_t p,                       /* index du point */
  index_t rs,                      /* taille rangee */
  index_t ps,                      /* taille plan */
  index_t N,                       /* taille image */
  int32_t *t,
  int32_t *tb);                    /* resultats */
extern int32_t mctopo3d_simple6(               /* pour un objet en 6-connexite */
  uint8_t *img,          /* pointeur base image */
  index_t p,                       /* index du point */
  index_t rs,                      /* taille rangee */
  index_t ps,                      /* taille plan */
  index_t N);                      /* taille image */
extern int32_t mctopo3d_simple18(              /* pour un objet en 18-connexite */
  uint8_t *img,          /* pointeur base image */
  index_t p,                       /* index du point */
  index_t rs,                      /* taille rangee */
  index_t ps,                      /* taille plan */
  index_t N);                      /* taille image */
extern int32_t mctopo3d_simple26(              /* pour un objet en 26-connexite */
  uint8_t *img,          /* pointeur base image */
  index_t p,                       /* index du point */
  index_t rs,                      /* taille rangee */
  index_t ps,                      /* taille plan */
  index_t N);                      /* taille image */
extern int32_t mctopo3d_simple6h(              /* pour un objet en 6-connexite */
  uint8_t *img,          /* pointeur base image */
  index_t p,                       /* index du point */
  int32_t h,                       /* seuil */
  index_t rs,                      /* taille rangee */
  index_t ps,                      /* taille plan */
  index_t N);                      /* taille image */
extern int32_t mctopo3d_simple18h(             /* pour un objet en 18-connexite */
  uint8_t *img,          /* pointeur base image */
  index_t p,                       /* index du point */
  int32_t h,                       /* seuil */
  index_t rs,                      /* taille rangee */
  index_t ps,                      /* taille plan */
  index_t N);                      /* taille image */
extern int32_t mctopo3d_simple26h(             /* pour un objet en 26-connexite */
  uint8_t *img,          /* pointeur base image */
  index_t p,                       /* index du point */
  int32_t h,                       /* seuil */
  index_t rs,                      /* taille rangee */
  index_t ps,                      /* taille plan */
  index_t N);                      /* taille image */
extern int32_t mctopo3d_simple6lab(                  /* pour un objet en 26-connexite */
  int32_t *img,          /* pointeur base image */
  index_t p,                       /* index du point */
  index_t rs,                      /* taille rangee */
  index_t ps,                      /* taille plan */
  index_t N);                      /* taille image */
extern int32_t mctopo3d_simple18lab(                  /* pour un objet en 26-connexite */
  int32_t *img,          /* pointeur base image */
  index_t p,                       /* index du point */
  index_t rs,                      /* taille rangee */
  index_t ps,                      /* taille plan */
  index_t N);                      /* taille image */
extern int32_t mctopo3d_simple26lab(                  /* pour un objet en 26-connexite */
  int32_t *img,          /* pointeur base image */
  index_t p,                       /* index du point */
  index_t rs,                      /* taille rangee */
  index_t ps,                      /* taille plan */
  index_t N);                      /* taille image */
extern int32_t mctopo3d_tbar6h(               /* pour un objet en 6-connexite */
  uint8_t *img,          /* pointeur base image */
  index_t p,                       /* index du point */
  int32_t h,                       /* seuil */
  index_t rs,                      /* taille rangee */
  index_t ps,                      /* taille plan */
  index_t N);                      /* taille image */
extern int32_t mctopo3d_tbar26h(              /* pour un objet en 26-connexite */
  uint8_t *img,          /* pointeur base image */
  index_t p,                       /* index du point */
  int32_t h,                       /* seuil */
  index_t rs,                      /* taille rangee */
  index_t ps,                      /* taille plan */
  index_t N);                      /* taille image */
extern uint8_t mctopo3d_P_simple(voxel * cube, voxel * cubep, voxel * cubec, uint8_t connex);




extern int32_t mctopo3d_pdestr6(                   /* pour des minima en 6-connexite */
  uint8_t *img,          /* pointeur base image */
  index_t p,                       /* index du point */
  index_t rs,                      /* taille rangee */
  index_t ps,                      /* taille plan */
  index_t N);                      /* taille image */
extern int32_t mctopo3d_pdestr18(                  /* pour des minima en 18-connexite */
  uint8_t *img,          /* pointeur base image */
  index_t p,                       /* index du point */
  index_t rs,                      /* taille rangee */
  index_t ps,                      /* taille plan */
  index_t N);                      /* taille image */
extern int32_t mctopo3d_pdestr26(                  /* pour des minima en 26-connexite */
  uint8_t *img,          /* pointeur base image */
  index_t p,                       /* index du point */
  index_t rs,                      /* taille rangee */
  index_t ps,                      /* taille plan */
  index_t N);                      /* taille image */
extern int32_t mctopo3d_plevdestr6(                   /* pour des minima en 6-connexite */
  uint8_t *img,          /* pointeur base image */
  index_t p,                       /* index du point */
  index_t rs,                      /* taille rangee */
  index_t ps,                      /* taille plan */
  index_t N);                      /* taille image */
extern int32_t mctopo3d_plevdestr18(                  /* pour des minima en 18-connexite */
  uint8_t *img,          /* pointeur base image */
  index_t p,                       /* index du point */
  index_t rs,                      /* taille rangee */
  index_t ps,                      /* taille plan */
  index_t N);                      /* taille image */
extern int32_t mctopo3d_plevdestr26(                  /* pour des minima en 26-connexite */
  uint8_t *img,          /* pointeur base image */
  index_t p,                       /* index du point */
  index_t rs,                      /* taille rangee */
  index_t ps,                      /* taille plan */
  index_t N);                      /* taille image */
extern int32_t mctopo3d_pconstr6(                   /* pour des minima en 6-connexite */
  uint8_t *img,          /* pointeur base image */
  index_t p,                       /* index du point */
  index_t rs,                      /* taille rangee */
  index_t ps,                      /* taille plan */
  index_t N);                       /* taille image */
extern int32_t mctopo3d_pconstr18(                  /* pour des minima en 18-connexite */
  uint8_t *img,          /* pointeur base image */
  index_t p,                       /* index du point */
  index_t rs,                      /* taille rangee */
  index_t ps,                      /* taille plan */
  index_t N);                       /* taille image */
extern int32_t mctopo3d_pconstr26(                  /* pour des minima en 26-connexite */
  uint8_t *img,          /* pointeur base image */
  index_t p,                       /* index du point */
  index_t rs,                      /* taille rangee */
  index_t ps,                      /* taille plan */
  index_t N);                       /* taille image */
extern int32_t mctopo3d_plevconstr6(                   /* pour des minima en 6-connexite */
  uint8_t *img,          /* pointeur base image */
  index_t p,                       /* index du point */
  index_t rs,                      /* taille rangee */
  index_t ps,                      /* taille plan */
  index_t N);                       /* taille image */
extern int32_t mctopo3d_plevconstr18(                  /* pour des minima en 18-connexite */
  uint8_t *img,          /* pointeur base image */
  index_t p,                       /* index du point */
  index_t rs,                      /* taille rangee */
  index_t ps,                      /* taille plan */
  index_t N);                       /* taille image */
extern int32_t mctopo3d_plevconstr26(                  /* pour des minima en 26-connexite */
  uint8_t *img,          /* pointeur base image */
  index_t p,                       /* index du point */
  index_t rs,                      /* taille rangee */
  index_t ps,                      /* taille plan */
  index_t N);                       /* taille image */
extern int32_t mctopo3d_peak6(                   /* pour des minima en 6-connexite */
  uint8_t *img,          /* pointeur base image */
  index_t p,                       /* index du point */
  index_t rs,                      /* taille rangee */
  index_t ps,                      /* taille plan */
  index_t N);                       /* taille image */
extern int32_t mctopo3d_peak26(                   /* pour des minima en 26-connexite */
  uint8_t *img,          /* pointeur base image */
  index_t p,                       /* index du point */
  index_t rs,                      /* taille rangee */
  index_t ps,                      /* taille plan */
  index_t N);                       /* taille image */
extern int32_t mctopo3d_well6(                   /* pour des minima en 6-connexite */
  uint8_t *img,          /* pointeur base image */
  index_t p,                       /* index du point */
  index_t rs,                      /* taille rangee */
  index_t ps,                      /* taille plan */
  index_t N);                       /* taille image */
extern int32_t mctopo3d_well26(                   /* pour des minima en 26-connexite */
  uint8_t *img,          /* pointeur base image */
  index_t p,                       /* index du point */
  index_t rs,                      /* taille rangee */
  index_t ps,                      /* taille plan */
  index_t N);                       /* taille image */
extern uint8_t mctopo3d_alpha26m(
  uint8_t *img,          /* pointeur base image */
  index_t p,                       /* index du point */
  index_t rs,                      /* taille rangee */
  index_t ps,                      /* taille plan */
  index_t N);                       /* taille image */
extern int32_t mctopo3d_alpha26m_l(
  int32_t *img,          /* pointeur base image */
  index_t p,                       /* index du point */
  index_t rs,                      /* taille rangee */
  index_t ps,                      /* taille plan */
  index_t N);                       /* taille image */
extern uint8_t mctopo3d_alpha6m(
  uint8_t *img,          /* pointeur base image */
  index_t p,                       /* index du point */
  index_t rs,                      /* taille rangee */
  index_t ps,                      /* taille plan */
  index_t N);                       /* taille image */
extern uint8_t mctopo3d_alpha26p(
  uint8_t *img,          /* pointeur base image */
  index_t p,                       /* index du point */
  index_t rs,                      /* taille rangee */
  index_t ps,                      /* taille plan */
  index_t N);                       /* taille image */
extern uint8_t mctopo3d_alpha6p(
  uint8_t *img,          /* pointeur base image */
  index_t p,                       /* index du point */
  index_t rs,                      /* taille rangee */
  index_t ps,                      /* taille plan */
  index_t N);                       /* taille image */
extern uint8_t mctopo3d_delta6m( 
/* retourne la valeur max. a laquelle p est destructible - minima 6-connexes */
  uint8_t *img,          /* pointeur base image */
  index_t p,                       /* index du point */
  index_t rs,                      /* taille rangee */
  index_t ps,                      /* taille plan */
  index_t N);                      /* taille image */
extern uint8_t mctopo3d_delta26m( 
/* retourne la valeur max. a laquelle p est destructible - minima 26-connexes */
  uint8_t *img,          /* pointeur base image */
  index_t p,                       /* index du point */
  index_t rs,                      /* taille rangee */
  index_t ps,                      /* taille plan */
  index_t N);                      /* taille image */
extern uint8_t mctopo3d_delta6p( 
/* retourne la valeur min. a laquelle p est constructible - minima 6-connexes */
  uint8_t *img,          /* pointeur base image */
  index_t p,                       /* index du point */
  index_t rs,                      /* taille rangee */
  index_t ps,                      /* taille plan */
  index_t N);                      /* taille image */
extern uint8_t mctopo3d_delta26p( 
/* retourne la valeur min. a laquelle p est constructible - minima 26-connexes */
  uint8_t *img,          /* pointeur base image */
  index_t p,                       /* index du point */
  index_t rs,                      /* taille rangee */
  index_t ps,                      /* taille plan */
  index_t N);                      /* taille image */
extern int32_t mctopo3d_separant6(  /* teste si un point est separant - minima 6-connexes
	         ie- s'il est separant pour une coupe <= img[p] */
  uint8_t *img,          /* pointeur base image */
  index_t p,                       /* index du point */
  index_t rs,                      /* taille rangee */
  index_t ps,                      /* taille plan */
  index_t N);                      /* taille image */
extern int32_t mctopo3d_hseparant6(  /* teste si un point est hseparant - minima 6-connexes
	         ie- s'il est separant pour la coupe h */
  uint8_t *img,          /* pointeur base image */
  index_t p,                       /* index du point */
  int32_t h,                       /* parametre */
  index_t rs,                      /* taille rangee */
  index_t ps,                      /* taille plan */
  index_t N);                      /* taille image */
extern int32_t mctopo3d_hfseparant6(  /* teste si un point est hfseparant - minima 6-connexes
	         ie- s'il est separant pour une coupe c telle que h < c <= img[p] */
  uint8_t *img,          /* pointeur base image */
  index_t p,                       /* index du point */
  int32_t h,                       /* parametre */
  index_t rs,                      /* taille rangee */
  index_t ps,                      /* taille plan */
  index_t N);                      /* taille image */
extern int32_t mctopo3d_filsombre6(                /* pour des minima en 6-connexite */
  uint8_t *img,          /* pointeur base image */
  index_t p,                       /* index du point */
  index_t rs,                      /* taille rangee */
  index_t ps,                      /* taille plan */
  index_t N);                      /* taille image */
extern int32_t mctopo3d_filsombre26(               /* pour des minima en 26-connexite */
  uint8_t *img,          /* pointeur base image */
  index_t p,                       /* index du point */
  index_t rs,                      /* taille rangee */
  index_t ps,                      /* taille plan */
  index_t N);                      /* taille image */
extern int32_t mctopo3d_filclair6(                 /* pour des minima en 6-connexite */
  uint8_t *img,          /* pointeur base image */
  index_t p,                       /* index du point */
  index_t rs,                      /* taille rangee */
  index_t ps,                      /* taille plan */
  index_t N);                      /* taille image */
extern int32_t mctopo3d_filclair26(                /* pour des minima en 26-connexite */
  uint8_t *img,          /* pointeur base image */
  index_t p,                       /* index du point */
  index_t rs,                      /* taille rangee */
  index_t ps,                      /* taille plan */
  index_t N);                      /* taille image */
extern int32_t mctopo3d_t6mm(uint8_t *img, index_t p, index_t rs, index_t ps, index_t N);
extern int32_t mctopo3d_t6m(uint8_t *img, index_t p, index_t rs, index_t ps, index_t N);
extern int32_t mctopo3d_t6pp(uint8_t *img, index_t p, index_t rs, index_t ps, index_t N);
extern int32_t mctopo3d_t6p(uint8_t *img, index_t p, index_t rs, index_t ps, index_t N);
extern int32_t mctopo3d_t26mm(uint8_t *img, index_t p, index_t rs, index_t ps, index_t N);
extern int32_t mctopo3d_t26m(uint8_t *img, index_t p, index_t rs, index_t ps, index_t N);
extern int32_t mctopo3d_t26pp(uint8_t *img, index_t p, index_t rs, index_t ps, index_t N);
extern int32_t mctopo3d_t26p(uint8_t *img, index_t p, index_t rs, index_t ps, index_t N);
extern int32_t mctopo3d_t26pp_l(int32_t *img, index_t p, index_t rs, index_t ps, index_t N);
extern int32_t mctopo3d_t6pp_l(int32_t *img, index_t p, index_t rs, index_t ps, index_t N);
extern int32_t mctopo3d_bordext6(uint8_t *F, index_t x, index_t rs, index_t ps, index_t N);
extern int32_t mctopo3d_bordext26(uint8_t *F, index_t x, index_t rs, index_t ps, index_t N);
extern void mctopo3d_nbtopoh3d26_l( /* pour les minima en 26-connexite */ 
  int32_t *img,          /* pointeur base image */
  index_t p,                       /* index du point */
  int32_t h,
  index_t rs,                      /* taille rangee */
  index_t ps,                      /* taille plan */
  index_t N,                       /* taille image */
  int32_t *t6p,
  int32_t *t26mm);
extern void mctopo3d_nbtopoh3d6_l( /* pour les minima en 26-connexite */ 
  int32_t *img,          /* pointeur base image */
  index_t p,                       /* index du point */
  int32_t h,
  index_t rs,                      /* taille rangee */
  index_t ps,                      /* taille plan */
  index_t N,                       /* taille image */
  int32_t *t26p,
  int32_t *t6mm);
extern int32_t mctopo3d_nbvoisc6(
  uint8_t *B,            /* pointeur base image */
  index_t i,                       /* index du point */
  index_t rs,                      /* taille rangee */
  index_t ps,                      /* taille plan */
  index_t N);                      /* taille image */
extern int32_t mctopo3d_nbvoisc18(
  uint8_t *B,            /* pointeur base image */
  index_t i,                       /* index du point */
  index_t rs,                      /* taille rangee */
  index_t ps,                      /* taille plan */
  index_t N);                      /* taille image */
extern int32_t mctopo3d_nbvoisc26(
  uint8_t *B,            /* pointeur base image */
  index_t i,                       /* index du point */
  index_t rs,                      /* taille rangee */
  index_t ps,                      /* taille plan */
  index_t N);                      /* taille image */
extern int32_t mctopo3d_nbvoiso6(
  uint8_t *B,            /* pointeur base image */
  index_t i,                       /* index du point */
  index_t rs,                      /* taille rangee */
  index_t ps,                      /* taille plan */
  index_t N);                      /* taille image */
extern int32_t mctopo3d_nbvoiso18(
  uint8_t *B,            /* pointeur base image */
  index_t i,                       /* index du point */
  index_t rs,                      /* taille rangee */
  index_t ps,                      /* taille plan */
  index_t N);                      /* taille image */
extern int32_t mctopo3d_nbvoiso26(
  uint8_t *B,            /* pointeur base image */
  index_t i,                       /* index du point */
  index_t rs,                      /* taille rangee */
  index_t ps,                      /* taille plan */
  index_t N);                      /* taille image */
extern int32_t mctopo3d_nbvoislab6(
  int32_t *B,            /* pointeur base image */
  index_t i,                       /* index du point */
  index_t rs,                      /* taille rangee */
  index_t ps,                      /* taille plan */
  index_t N);                       /* taille image */
extern int32_t mctopo3d_nbvoislab18(
  int32_t *B,            /* pointeur base image */
  index_t i,                       /* index du point */
  index_t rs,                      /* taille rangee */
  index_t ps,                      /* taille plan */
  index_t N);                       /* taille image */
extern int32_t mctopo3d_nbvoislab26(
  int32_t *B,            /* pointeur base image */
  index_t i,                       /* index du point */
  index_t rs,                      /* taille rangee */
  index_t ps,                      /* taille plan */
  index_t N);                       /* taille image */
extern int32_t mctopo3d_nbvoislabc6(
  int32_t *B,            /* pointeur base image */
  index_t i,                       /* index du point */
  index_t rs,                      /* taille rangee */
  index_t ps,                      /* taille plan */
  index_t N);                       /* taille image */
extern int32_t mctopo3d_nbvoislabc18(
  int32_t *B,            /* pointeur base image */
  index_t i,                       /* index du point */
  index_t rs,                      /* taille rangee */
  index_t ps,                      /* taille plan */
  index_t N);                       /* taille image */
extern int32_t mctopo3d_nbvoislabc26(
  int32_t *B,            /* pointeur base image */
  index_t i,                       /* index du point */
  index_t rs,                      /* taille rangee */
  index_t ps,                      /* taille plan */
  index_t N);                       /* taille image */
extern int32_t mctopo3d_curve6(uint8_t *img, index_t p, index_t rs, index_t ps, index_t N);
extern int32_t mctopo3d_curve18(uint8_t *img, index_t p, index_t rs, index_t ps, index_t N);
extern int32_t mctopo3d_curve26(uint8_t *img, index_t p, index_t rs, index_t ps, index_t N);
#ifdef __cplusplus
}
#endif
