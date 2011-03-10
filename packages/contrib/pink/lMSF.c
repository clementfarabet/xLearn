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
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/types.h>
#include <math.h>
#include <mccodimage.h>
#include <jccodimage.h>
#include <mcimage.h>
#include <jcimage.h>
#include <mcutil.h>
#include <mcindic.h>
#include <lMSF.h>
#include <mcrbt.h>

#define TRUE 1
#define FALSE 0

/* INPUT: 
         1. a graphs (V,E), F: un graphe, une fonction de valuation des 
	 aretes (embarquees ds structure ga).
	 2. Une fonction de labels pour les points de E (images marqueurs).
	 Les labels sont supposés connexes.
   OUTPUT: 
         La carte de labels des sommets de E correspondant à une MSF de marqueurs 
	 ds (V,E,F): résulatat ds marqueurs
 */

int32_t MSF(struct xvimage *ga, struct xvimage *marqueurs) 
#undef F_NAME
#define F_NAME "MSF"
{
  int32_t i;
  int32_t u,v;                                    /* une arete */
  int32_t x,y,z, x_1,y_1;                                  /* des sommets */
  int32_t rs = rowsize(ga);                     /* taille ligne */
  int32_t cs = colsize(ga);                     /* taille colonne */
  int32_t N = rs * cs;                          /* taille image */
  uint8_t *F = UCHARDATA(ga);         /* valuation des aretes de depart */
  int32_t *G = SLONGDATA(marqueurs); /* labels des sommets du graph */
  int32_t N_t=2*N;                              /* index maximum d'un arete de ga */
  Rbt *L;                            /* ensembles des aretes adjacentes à exactement un label */

  if (depth(ga) != 1){
    //fprintf(stderr, "%s: cette version ne traite pas les images volumiques, je refile le bebe à une autre version\n", F_NAME);
    return MSF3d(ga, marqueurs);
  }
 
  if ((rowsize(marqueurs) != rs) || (colsize(marqueurs) != cs)){
    fprintf(stderr, "%s: incompatible image sizes\n", F_NAME);
    return 0;
  }
  IndicsInit(N_t);
  L = mcrbt_CreeRbtVide(N_t);
  for(u = 0; u < N_t; u ++){
    if( ( (u < N) && (u%rs < rs-1)) || ((u >= N) && (u < N_t - rs))){
      x = Sommetx(u,N,rs);
      y = Sommety(u,N,rs);
      if((mcmin(G[x],G[y]) == 0) && (mcmax(G[x],G[y]) > 0)){
	/* u est growing edge */
	mcrbt_RbtInsert(&L, (TypRbtKey)F[u], u);
	Set(u, TRUE);
      }else Set(u, FALSE);
    }else Set(u,FALSE);
  }
  
 while(!mcrbt_RbtVide(L)){
   u = RbtPopMin(L);
#ifdef DEBUG
   printf("poped arete u no: %d de niveau %d\n",u,F[u]);
#endif
   x = Sommetx(u,N,rs);
   y = Sommety(u,N,rs);
   if(G[x] > G[y]){z=x; x=y; y=z;}
#ifdef DEBUG
   printf("extremites de u (%d,%d);(%d,%d) \n",x%rs, x/rs, y%rs, y/rs); 
#endif
   if((mcmin(G[x],G[y]) == 0) && (mcmax(G[x],G[y]) > 0)){
     /* si u est une growing edge */
#ifdef DEBUG
     printf("label x: %d, label y :%d\n", G[x], G[y]);
#endif
     G[x] = G[y];
     /* parcours des aretes incidente à x */
     for(i = 0; i < 4; i++){
       v = incidente(x,i,rs,N);
       if((v != -1) && (!IsSet(v, TRUE))){
	 /* si v n'est pas dans L */
#ifdef DEBUG
	 printf("aretes incidentes F[%d] %d\n", v, F[v]);
#endif
	 x_1 = Sommetx(v,N,rs);
	 y_1 = Sommety(v,N,rs);
#ifdef DEBUG
	 printf("extremites de v (%d), incidente à u (%d,%d);(%d,%d) \n",v, x_1%rs, x_1/rs, y_1%rs, y_1/rs);
#endif
	 if((mcmin(G[x_1],G[y_1]) == 0) && (mcmax(G[x_1],G[y_1]) > 0)){
	   /* v est une growing edge */
	   mcrbt_RbtInsert(&L, (TypRbtKey)F[v], v);
	   Set(v,TRUE);
	 }	  
       }
     }
   }
   UnSet(u,TRUE);
 }  
 
 for(u = 0; u < N_t; u++)
   if( ((u < N) && (u%rs < rs-1)) || ((u >= N) && (u < N_t - rs))){
     if(G[Sommetx(u,N,rs)] != G[Sommety(u,N,rs)])/*F[u]=255*/; else F[u] = 0;
   }
  /* Terminer indicateur + R&B tree ... */
 IndicsTermine();
 mcrbt_RbtTermine(L);
 return 1;
}
//#define DEBUG

int32_t MSF3d(struct xvimage *ga, struct xvimage *marqueurs)
#undef F_NAME
#define F_NAME "MSF3d"
{
  int32_t i;
  int32_t u,v;                                  /* une arete */
  int32_t x,y,z, x_1,y_1;                       /* des sommets */
  int32_t rs = rowsize(ga);                     /* taille ligne */
  int32_t cs = colsize(ga);                     /* taille colonne */
  int32_t ps = rs*cs;
  int32_t ds = depth(ga);                       /* taille plan */
  int32_t N = ds * ps;                          /* taille image */
  uint8_t *F = UCHARDATA(ga);         /* valuation des aretes de depart */
  int32_t *G =  SLONGDATA(marqueurs); /* labels des sommets du graph */
  int32_t N_t=3*N;                              /* index maximum d'une arete de ga */
  Rbt *L;                                   /* ensembles des aretes adjacentes à exactement un label */

  if (depth(ga) == 1){
    return MSF(ga, marqueurs);
  }
  if ((rowsize(marqueurs) != rs) || (colsize(marqueurs) != cs) || (depth(marqueurs) != ds)) 
  {
    fprintf(stderr, "%s: incompatible image sizes\n", F_NAME);
    return 0;
  }
  IndicsInit(N_t);
  L = mcrbt_CreeRbtVide(N_t);
  for(u = 0; u < N_t; u ++){
    if( ( (u < N) && (u%rs < rs-1)) ||
	((u >= N) && (u < 2*N) && ( (u%ps) < (ps-rs))) ||
	((u >= 2*N) && (((u- (2*N))/ps) < (ds-1)))){
      x = Sommetx3d(u,N,rs,ps);
      y = Sommety3d(u,N,rs,ps);
      if((mcmin(G[x],G[y]) == 0) && (mcmax(G[x],G[y]) > 0)){
	/* u est growing edge */
	/*	printf("Initialisation: ds Rbt: (%d,%d)\n", x,y);*/
	mcrbt_RbtInsert(&L, (TypRbtKey)F[u], u);
	Set(u, TRUE);
      }else Set(u, FALSE);
    }else Set(u,FALSE);
  }
  
  while(!mcrbt_RbtVide(L)){
    u = RbtPopMin(L);
#ifdef DEBUG
    printf("poped arete u no: %d de niveau %d\n",u,F[u]);
#endif
    x = Sommetx3d(u,N,rs,ps);
    y = Sommety3d(u,N,rs,ps);
    if(G[x] > G[y]){z=x; x=y; y=z;}
#ifdef DEBUG
    printf("extremites de u (%d,%d);(%d,%d) \n",x%rs, x/rs, y%rs, y/rs); 
#endif
    if((mcmin(G[x],G[y]) == 0) && (mcmax(G[x],G[y]) > 0)){
      /* si u est une growing edge */
#ifdef DEBUG
      printf("label x: %d, label y :%d\n", G[x], G[y]);
#endif
      G[x] = G[y];
      /* parcours des aretes incidente à x */
      for(i = 0; i < 6; i++){
	v = incidente3d(x,i,rs,N,ps);
	if((v != -1) && (!IsSet(v, TRUE))){
	  /* si v n'est pas dans L */
#ifdef DEBUG
	  printf("aretes incidentes F[%d] %d\n", v, F[v]);
#endif
	  x_1 = Sommetx3d(v,N,rs,ps);
	  y_1 = Sommety3d(v,N,rs,ps);
#ifdef DEBUG
	  printf("extremites de v (%d), incidente à u (%d,%d);(%d,%d) \n",v, x_1%rs, x_1/rs, y_1%rs, y_1/rs);
#endif
	  if((mcmin(G[x_1],G[y_1]) == 0) && (mcmax(G[x_1],G[y_1]) > 0)){
	    /* v est une growing edge */
	    mcrbt_RbtInsert(&L, (TypRbtKey)F[v], v);
	    Set(v,TRUE);
	  }	  
	}
      }
    }
    UnSet(u,TRUE);
  }  
  writeimage(marqueurs,"./toto1");
  for(u = 0; u < N_t; u++)
    if( ( (u < N) && (u%rs < rs-1)) ||
	((u >= N) && (u < 2*N) && ( (u%ps) < (ps-rs))) ||
	((u >= 2*N) && (((u- (2*N))/ps) < (ds-1)))){
      if(G[Sommetx3d(u,N,rs,ps)] != G[Sommety3d(u,N,rs,ps)]) F[u] = 255; else F[u] = 0;
    }
  /* Terminer indicateur + R&B tree ... */

  IndicsTermine();
  mcrbt_RbtTermine(L);
  return 1;
}

//#define DEBUG

int32_t MSF4d(struct GA4d *ga, struct xvimage4D *marqueurs)
#undef F_NAME
#define F_NAME "MSF4d"
{
  int32_t i;
  int32_t u,v;                                  /* une arete */
  int32_t x,y,z, x_1,y_1;                       /* des sommets */ 
  int32_t rs = rowsize(ga);                     /* taille ligne */ 
  int32_t cs = colsize(ga);                     /* taille colone */
  int32_t ps = rs * cs;                         /* taille d'un plan */
  int32_t ds = depth(ga);                       /* nbre de plans */
  int32_t vs = ps*ds;                           /* taille volume */
  int32_t ss = seqsizeGA(ga);                   /* nbre de volules*/
  int32_t N = ss * vs;
  uint8_t *F = UCHARDATA(ga);         /* graphe d'arere 4d */
  uint8_t **G;                        /* image de marqueurs 4D */
  int32_t N_t=4*N;                              /* index maximum d'une arete de ga */
  Rbt *L;                                   /* ensembles des aretes adjacentes à exactement un label */
  
  G = (uint8_t **)malloc(sizeof(char *) * ss);
  for(i = 0; i < ss; i++)
    G[i] = UCHARDATA(marqueurs->frame[i]);

  if ((rowsize(marqueurs->frame[0]) != rs) || (colsize(marqueurs->frame[0]) != cs) || 
      (depth(marqueurs->frame[0]) != ds) || (marqueurs->ss != ss)) 
  {
    fprintf(stderr, "%s: incompatible image sizes\n", F_NAME);
    return 0;
  }
  IndicsInit(N_t);
  /*Pas du tout robuste c'est juste qqch pour passer sur des images 4d du coeur*/
  L = mcrbt_CreeRbtVide(N_t/10);
  for(u = 0; u < N_t; u ++){
    if( ( (u < N) && (u%rs < rs-1)) ||
	( (u >= N) && (u < 2*N) && ( (u%ps) < (ps-rs) ) ) ||
	( (u >= 2*N) && (u < 3*N) && ( (u%vs) < (vs-ps) ) ) ||
	( (u >= 3*N) && ( (u%N) < (N-vs) ) )
	){
#ifdef DEBUG
      printf("F[(%d,%d,%d,%d),%d] = %d\n", u%rs, (u%ps)/rs, (u%vs)/ps, (u%N)/vs, u/N,F[u]);
#endif
      x = Sommetx4d(u,N,rs,ps,vs);
      y = Sommety4d(u,N,rs,ps,vs);
      /* On pourrait faire plus rapide avec une representation memoire
	 plus adequat des images 4d */
      if((mcmin(G[x/vs][x%vs],G[y/vs][y%vs]) == 0) && (mcmax(G[x/vs][x%vs],G[y/vs][y%vs]) > 0)){
	/* u est growing edge */
	/*	printf("Initialisation: ds Rbt: (%d,%d)\n", x,y);*/
	mcrbt_RbtInsert(&L, (TypRbtKey)F[u], u);
	Set(u, TRUE);
      }else Set(u, FALSE);
    }else Set(u,FALSE);
  }
  printf("Initialisation OK \n");
  while(!mcrbt_RbtVide(L)){
    u = RbtPopMin(L);
#ifdef DEBUG
    printf("Arete poped F[(%d,%d,%d,%d),%d] = %d\n", u%rs, (u%ps)/rs, (u%vs)/ps, (u%N)/vs, u/N,F[u]); 
#endif
    x = Sommetx4d(u,N,rs,ps,vs);
    y = Sommety4d(u,N,rs,ps,vs);
#ifdef DEBUG
    printf("extremites de u (%d,%d);(%d,%d) \n",x%rs, x/rs, y%rs, y/rs); 
#endif
    if((mcmin(G[x/vs][x%vs],G[y/vs][y%vs]) == 0) && (mcmax(G[x/vs][x%vs],G[y/vs][y%vs]) > 0)){
      /* si u est une growing edge */
#ifdef DEBUG
      printf("Arete growing F[(%d,%d,%d,%d),%d] = %d\n", u%rs, (u%ps)/rs, (u%vs)/ps, (u%N)/vs, u/N,F[u]);
#endif 
      if(G[x/vs][x%vs] > G[y/vs][y%vs]){z=x; x=y; y=z;}
      G[x/vs][x%vs] = G[y/vs][y%vs];
      /* parcours des aretes incidente à x */
      for(i = 0; i < 8; i++){
	v = incidente4d(x,i,rs,N,ps,vs);
	if((v != -1) && (!IsSet(v, TRUE))){
	  /* si v n'est pas dans L */
#ifdef DEBUG
	  printf("Arete incidentes F[(%d,%d,%d,%d),%d] = %d\n", v%rs, (v%ps)/rs, (v%vs)/ps, (v%N)/vs, v/N,F[v]);
#endif
	  x_1 = Sommetx4d(v,N,rs,ps,vs);
	  y_1 = Sommety4d(v,N,rs,ps,vs);
#ifdef DEBUG
	  printf("extremites de v (%d), incidente à u (%d,%d);(%d,%d) \n",v, x_1%rs, x_1/rs, y_1%rs, y_1/rs);
#endif
	  if((mcmin(G[x_1/vs][x_1%vs],G[y_1/vs][y_1%vs]) == 0) && (mcmax(G[x_1/vs][x_1%vs],G[y_1/vs][y_1%vs]) > 0)){
	    /* v est une growing edge */
#ifdef DEBUG
	    printf("Arete pushed F[(%d,%d,%d,%d),%d] = %d\n", v%rs, (v%ps)/rs, (v%vs)/ps, (v%N)/vs, v/N,F[v]);
#endif	    
	    mcrbt_RbtInsert(&L, (TypRbtKey)F[v], v);
	    Set(v,TRUE);
	  }	  
	}
      }
    }
    UnSet(u,TRUE);
  }  
  printf("Label map of MSF OK \n");
  for(u = 0; u < N_t; u++)
    if( ( (u < N) && (u%rs < rs-1)) ||
	( (u >= N) && (u < 2*N) && ( (u%ps) < (ps-rs) ) ) ||
	( (u >= 2*N) && (u < 3*N) && ( (u%vs) < (vs-ps) ) ) ||
	( (u >= 3*N) && ( (u%N) < (N-vs) ) )
	){
      x = Sommetx4d(u,N,rs,ps,vs);
      y = Sommety4d(u,N,rs,ps,vs);
      if(G[x/vs][x%vs] != G[y/vs][y%vs]) F[u] = 255; else F[u] = 0;
    }
  /* Terminer indicateur + R&B tree ... */
  
  IndicsTermine();
  mcrbt_RbtTermine(L);
  return 1;
}
