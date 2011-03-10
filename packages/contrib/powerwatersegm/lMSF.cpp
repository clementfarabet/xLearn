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
#include <stdint.h>
#include <stdlib.h>
#include <sys/types.h>
#include <mccodimage.h>
#include <mcimage.h>
#include <mclifo.h>
#include <mcindic.h>
#include <mcutil.h>
#include <mcrbt.h>
#include <cccodimage.h>
#include <powerwatersegm.h>
#include <union_find.h>
#include <ccsort.h>


/*=====================================================================================*/
struct xvimage * MSF_Prim(int ** edges,       /* array of node indexes composing edges */ 
			  uint32_t * weights, /* weights values on the edges */
			  int *seeds,         /* array of seeded nodes indexes */
			  uint8_t * labels,   /* label values on the seeded nodes */
			  int size_seeds,     /* nb of seeded nodes */ 
			  int rs,             /* row size of the image */
			  int cs,             /* col size of the image */
			  int ds,             /* depth size of the image */
			  int nblabels)       /*nb of different labels */		  
/*=====================================================================================*/
/* returns a segmentation performed by Prim's algorithm for Maximum Spanning Forest computation */
{  
  int32_t i, u,v, x,y,z, x_1,y_1;                        
  int32_t N = rs * cs * ds;                         /* nb of nodes */
  int   M = ds*rs*(cs-1)+ds*(rs-1)*cs+(ds-1)*cs*rs; /* number of edges */
  int numvoisins = 6;
    
  struct xvimage * temp;
  temp = allocimage(NULL, rs, cs, ds, VFF_TYP_1_BYTE);

  uint8_t * G = (uint8_t*)calloc(N ,sizeof(uint8_t));
  for (i=0;i<size_seeds;i++)
    G[seeds[i]] = labels[i];
           
  Rbt *L;                           
  
  IndicsInit(M);
  L = CreeRbtVide(M);
  i=0;
  int sizeL = 0;
  for(u = 0; u < M; u ++)
    Set(u, false);
    
  for (u=0;u<M;u++)
    {
      if (u==seeds[i])
	{
	  for (x=1;x<=numvoisins;x++)
	    {
	      y = neighbor_node_edge(seeds[i], x, rs, cs, ds);
	      if ((y != -1) && (!IsSet(y, true)))
		{ 
		  RbtInsert(&L, (TypRbtKey)(255-weights[y]), y);
		  sizeL++;
		  Set(y, true);
		}
	    }
	  i++;
	}
    }

  while(sizeL != 0)
    {
      u = RbtPopMin(L);
      sizeL--;
      x = edges[0][u];
      y = edges[1][u];
      if(G[x] > G[y])
	{z=x; x=y; y=z;}
      if((mcmin(G[x],G[y]) == 0) && (mcmax(G[x],G[y]) > 0))
	{
	  G[x] = G[y];
	  for(i = 1; i <= numvoisins; i++)
	    {
	      v = neighbor_node_edge(x,i,rs,cs, ds);
	      if((v != -1) && (!IsSet(v, true)))
		{
		  x_1 = edges[0][v];
		  y_1 = edges[1][v];
		  if((mcmin(G[x_1],G[y_1]) == 0) && (mcmax(G[x_1],G[y_1]) > 0))
		    {
		      RbtInsert(&L, (TypRbtKey)(255-weights[v]), v);
		      sizeL++;
		      Set(v,true);
		    }	  
		}
	    }
	}
      UnSet(u,true);
    }

  unsigned char *F = UCHARDATA(temp);
 
  for(i = 0; i < N; i++)
    F[i]=255*(G[i]-1)/(nblabels-1);
  
  IndicsTermine();
  free(G);
  RbtTermine(L);
  for (i=0;i<2;i++) free(edges[i]); free(edges);
  return temp;
}

	  

/*=====================================================================*/
struct xvimage * MSF_Kruskal(int ** edges,       /* array of node indexes composing edges */ 
			     uint32_t * weights, /* weights values on the edges */
			     int max_weight,     /* maximum weight value */
			     int *seeds,         /* array of seeded nodes indexes */
			     uint8_t * labels,   /* label values on the seeded nodes */
			     int size_seeds,     /* nb of seeded nodes */ 
			     int rs,             /* row size of the image */
			     int cs,             /* col size of the image */
			     int ds,             /* depth size of the image */
			     int nblabels)       /* nb of different labels */
/*=====================================================================*/
/*returns a segmentation performed by Kruskal's algorithm for Maximum Spanning Forest computation*/
{  
  int i, j, k, x, y, e1, e2;
  int N, M;
 
  N = rs * cs * ds;                /* number of vertices */   
  M = ds*rs*(cs-1)+ds*(rs-1)*cs+(ds-1)*cs*rs; /*number of edges*/
  int numvoisins = 6;
   
  struct xvimage * temp;
  temp = allocimage(NULL, rs, cs, ds, VFF_TYP_1_BYTE);

   Lifo * LIFO; 
  LIFO = CreeLifoVide(M);
  if (LIFO == NULL) { fprintf(stderr, "lMSF() : CreeLifoVide failed\n"); exit(0); }

  int * Mrk = (int*)calloc(N ,sizeof(int));
  if (Mrk == NULL) { fprintf(stderr, "lMSF() : malloc failed\n"); exit(0); }
  for (i=0;i<size_seeds;i++){
  
    Mrk[seeds[i]] = labels[i];
  }
  int * Rnk = (int*)calloc(N, sizeof(int));
  if (Rnk == NULL) { fprintf(stderr, "lMSF() : malloc failed\n"); exit(0); }
  int * Fth = (int*)malloc(N*sizeof(int));
  if (Fth == NULL) { fprintf(stderr, "lMSF() : malloc failed\n"); exit(0); }
  for(k=0;k<N;k++) { Fth[k]=k; }
  
  // Es : E sorted by decreasing weights
  int * Es = (int*)malloc(M*sizeof(int)); 
  if (Es == NULL) { fprintf(stderr, "lMSF() : malloc failed\n"); exit(0); }
  for(k=0;k<M;k++) Es[k]=k;

  uint32_t * sorted_weights = (uint32_t*)malloc(M*sizeof(uint32_t));
  for(k=0;k<M;k++) 
    sorted_weights[k]=weights[k]; 
  BucketSort (sorted_weights, Es, M, max_weight+1);
  free(sorted_weights);
  
  int nb_arete = 0;
  int e_max, root;
 
  /* beginning of main loop */  
  
  int cpt_aretes = 0;

  while (nb_arete < N-size_seeds)
    {
      e_max=Es[cpt_aretes];
      // printf("%d \n", e_max);
      cpt_aretes=cpt_aretes+1;
      e1 = edges[0][e_max];
      e2 = edges[1][e_max];
      x = element_find(e1, Fth );
      y = element_find(e2, Fth );

      if ((x != y) && (!(Mrk[x]>=1 && Mrk[y]>=1)))
	{
	  root = element_link( x,y, Rnk, Fth);
	  nb_arete=nb_arete+1;
	  if ( Mrk[x]>=1) Mrk[root]= Mrk[x];
	  else if ( Mrk[y]>=1) Mrk[root]= Mrk[y]; 
	}
    }

  //building the map (find the root vertex of each tree)
  int * Map2 = (int *)malloc(N*sizeof(int));
  int * Map = (int *)malloc(N*sizeof(int));
  for (i=0; i<N; i++) 
    Map2[i] = element_find(i, Fth);

  bool* Fullseeds = (bool *)calloc(N,sizeof(bool));
   for (i=0;i<size_seeds;i++)
     {
       Fullseeds[seeds[i]]=1;
       Map[seeds[i]] = (int)labels[i];
     }
   
   for (i=0;i<N;i++)Mrk[i] = false;
   for (i=0;i<size_seeds; i++) 
     {
       LifoPush(LIFO, seeds[i]);
       while (!LifoVide(LIFO))
	 {
	   x = LifoPop(LIFO);
	   Mrk[x]=true;
	   for (k=1;k<=numvoisins;k++)
	     {
	       y = neighbor(x, k, rs, cs, ds);
	       if (y != -1)
		 {
		   if (Map2[y]==Map2[seeds[i]] && Fullseeds[y]!=1 && Mrk[y]==false)
		     {   
		       LifoPush(LIFO, y);
		       Map[y]= labels[i];
		       Mrk[y]=true;
		     }
		 }
	     }
	 }  
       LifoFlush(LIFO);
     }
   
   unsigned char *Temp = UCHARDATA(temp); 
   for (j = 0; j < N; j++)
     Temp[j] = 255*(Map[j]-1)/(nblabels-1);
    
   
   LifoTermine(LIFO);
   for (i=0;i<2;i++) 
     free(edges[i]);
   free(edges);
   free(Mrk);
   free(Rnk);
   free(Fullseeds);
   free(Es);
   free(Map);
   free(Map2);
   free(Fth);
   return temp;
}
