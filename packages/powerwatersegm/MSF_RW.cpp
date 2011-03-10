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
#include <sys/types.h>
#include <math.h>
#include <mccodimage.h>
#include <mcimage.h>
#include <mclifo.h>
#include <mcutil.h>
#include <cccodimage.h>
#include <MSF_RW.h>
#include <union_find.h>
#include <ccsort.h>
#include <powerwatersegm.h>
#include <random_walker.h>


/*========================================================================================================*/
void memory_allocation_PW(Lifo ** LIFO,       /* stack for plateau labeling */
			  Lifo ** LCP,        /* list of the edges belonging to a plateau */        
			  bool ** indic_E,    /* indicator for edges */
			  bool ** indic_P,    /* indicator for edges */
			  int ** indic_VP,    /* indicator for nodes */
			  int ** Rnk,         /* array needed for union-find efficiency */
			  int ** Fth,         /* array for storing roots of merged nodes trees */
			  int ** local_seeds, /* array for storing the index of seeded nodes of a plateau */
			  int ** LCVP ,       /* list of vertices of a plateau */
			  int ** Es,          /* array of sorted edges according their reconstructed weight */
			  int ** NEs,         /* array of sorted edges according their weight */
			  int N,              /* number of vertices */  
			  int M)              /* number of edges*/
/*==========================================================================================================*/
{
#undef F_NAME
#define F_NAME "memory_allocation_PW"
 
  *LIFO = CreeLifoVide(M);
  if (*LIFO == NULL) { fprintf(stderr, "%s : CreeLifoVide failed\n", F_NAME); exit(0); }

  *LCP = CreeLifoVide(M);
  if (*LCP == NULL) { fprintf(stderr, "%s : CreeLifoVide failed\n", F_NAME); exit(0); }
 
  *indic_E = (bool*)calloc(M ,sizeof(bool)); 
  if (*indic_E == NULL) { fprintf(stderr, "%s: calloc indic_E failed\n", F_NAME); exit(0);}
  *indic_P = (bool*)calloc(M ,sizeof(bool)); 
  if (*indic_P == NULL) { fprintf(stderr, "%s: calloc indic_P failed\n", F_NAME); exit(0);}
  *indic_VP = (int*)calloc(N ,sizeof(int));
  if (*indic_VP == NULL) { fprintf(stderr, "%s: calloc indic_VP failed\n", F_NAME); exit(0);}
  *Rnk = (int*)calloc(N, sizeof(int));
  if (*Rnk == NULL) { fprintf(stderr, "%s: malloc Rnk failed\n", F_NAME); exit(0);}
  *Fth = (int*)malloc(N*sizeof(int));
  if (*Fth == NULL) { fprintf(stderr, "%s: malloc Fth failed\n", F_NAME); exit(0);}
  
  *local_seeds = (int*)malloc(N*sizeof(int));
  if (*local_seeds == NULL) { fprintf(stderr, "%s: malloc local_seeds failed\n", F_NAME); exit(0);}
  
  *LCVP = (int*)malloc(N*sizeof(int)); // vertices of a plateau. 
  if (*LCVP == NULL) { fprintf(stderr, "%s: malloc LCVP failed\n", F_NAME); exit(0);}
  
  *Es = (int*)malloc(M*sizeof(int)); 
  if (*Es == NULL) { fprintf(stderr, "%s: malloc Es failed\n", F_NAME); exit(0);}
  
  *NEs = (int*)malloc(M*sizeof(int)); 
  if (*NEs == NULL) { fprintf(stderr, "%s: malloc NEs failed\n", F_NAME); exit(0);}
}


/*===================================================================================*/
void merge_node (int e1,            /* index of node 1 */
		 int e2,            /* index of node 2 */
		 int * Rnk,         /* array needed for union-find efficiency */
		 int *Fth,          /* array for storing roots of merged nodes trees */
		 DBL_TYPE ** proba, /* array for storing the result x */
		 int nb_labels)     /* nb of labels */
/*===================================================================================*/
/* update the result, Rnk and Fth arrays when 2 nodes are merged */
{
  int k,re1, re2;
  re1 = element_find(e1, Fth );
  re2 = element_find(e2, Fth );
 
  if ((re1 != re2) && (!(proba[0][re1]>=0 && proba[0][re2]>=0))) 
    {
      element_link(re1,re2, Rnk, Fth);
      if (proba[0][re2]>=0 && proba[0][re1]<0) 
	for(k=0;k<nb_labels-1;k++)
	  proba[k][re1]= proba[k][re2];
      else if (proba[0][re1]>=0 && proba[0][re2]<0)
	for(k=0;k<nb_labels-1;k++)
	  proba[k][re2]= proba[k][re1];
    }
}

/*==================================================================================================================*/
struct xvimage * PowerWatershed_q2(int ** edges,              /*array of node indexes composing edges */
				   uint32_t * weights,        /* reconstructed weights */
				   uint32_t * normal_weights, /* original weights */
				   int max_weight,            /* maximum weight value */
				   int *seeds,                /* array of seeded nodes indexes */
				   uint8_t * labels,          /* label values on the seeded nodes */
				   int size_seeds,            /* nb of seeded nodes */ 
				   int rs,                    /* row size */
				   int cs,                    /* col size */
				   int ds,                    /* depth size */
				   int nb_labels,             /* number of different labels */
				   bool quicksort,            /* true : bucket sort used; false : stochastic sort o(n log n) */
				   struct xvimage *img_proba) /* output image of potential/proba map x minimizing Epq*/
/*==================================================================================================================*/
/*returns the result x of the energy minimization : min_x lim_p_inf sum_e_of_E w_{ij}^p |x_i-x_j|^2 */
{  
#undef F_NAME
#define F_NAME "PowerWatershed_q2"
  int i, j, k, x, y, e1, e2, re1,re2, p, xr;
  int N = rs * cs * ds;                /* number of vertices */   
  int M = ds*rs*(cs-1)+ds*(rs-1)*cs+(ds-1)*cs*rs;    /* number of edges*/ 
  double val;
  int argmax;
  int nb_vertices, e_max, Ne_max, nb_edges, Nnb_edges;
  int nb_neighbor_edges = 6;
  if(ds>1) nb_neighbor_edges = 12;
  bool success, different_seeds;
  uint32_t wmax;
  Lifo * LIFO;
  Lifo * LCP;
  bool * indic_E; 
  bool * indic_P; 
  int * indic_VP;
  int * Rnk; 
  int * Fth;
  int * local_seeds;
  int * LCVP ;
  int * Es;
  int * NEs ;

  memory_allocation_PW( &LIFO, &LCP, &indic_E,  &indic_P,  &indic_VP, &Rnk,  &Fth, &local_seeds, &LCVP, &Es, &NEs, N, M);
 
  DBL_TYPE **proba = (DBL_TYPE **)malloc((nb_labels-1) *sizeof(DBL_TYPE*));
  for (i=0; i<nb_labels-1; i++) 
    {
      proba[i]= (DBL_TYPE *)malloc(N *sizeof(DBL_TYPE));
      for (j=0; j<N; j++) proba[i][j]=-1;
    }
  int ** edgesLCP = (int**)malloc(2*sizeof(int*));
  if (edgesLCP == NULL) { fprintf(stderr, "%s: malloc edgesLCP failed\n", F_NAME); exit(0);}
 for(k=0;k<2;k++) 
   {
    edgesLCP[k] = (int*)malloc(M*sizeof(int));
    if ( edgesLCP[k]== NULL)
      { fprintf(stderr, "%s: malloc edgesLCP failed\n", F_NAME); exit(0);}
   }
  for (i=0;i<size_seeds;i++)
    for (j=0;j<nb_labels-1;j++)
      {
	if (labels[i]==j+1)
	  proba[j][seeds[i]] = 1;
	else proba[j][seeds[i]] = 0;
      }
 
  for(k=0;k<N;k++) Fth[k]=k;

  DBL_TYPE ** local_labels = (DBL_TYPE**)malloc((nb_labels-1)*sizeof(DBL_TYPE*));
  if (local_labels == NULL) { fprintf(stderr, "%s: malloc local_labels failed\n", F_NAME); exit(0);}
  for (i=0; i<nb_labels-1; i++)
    {
      local_labels[i]= (DBL_TYPE *)malloc(N *sizeof(DBL_TYPE));
      if ( local_labels[i]== NULL)	
      	{ fprintf(stderr, "%s: malloc local_labels failed\n", F_NAME); exit(0);}
    }
 
  uint32_t * sorted_weights = (uint32_t*)malloc(M*sizeof(uint32_t));
  if (sorted_weights == NULL) { fprintf(stderr, "%s: malloc sorted_weights failed\n", F_NAME); exit(0);}
 
  for(k=0;k<M;k++) 
    { 
      sorted_weights[k]=weights[k]; 
      Es[k]=k;
    }
  if (quicksort == true)
    BucketSort(sorted_weights, Es, M, max_weight+1);
  else 
    TriRapideStochastique_dec(sorted_weights,Es, 0,M-1); 
  int cpt_aretes = 0;
  int Ncpt_aretes = 0;
 
 /* beginning of main loop */   
  while (cpt_aretes < M)
    {
      do 
	{
	  e_max=Es[cpt_aretes];
	  cpt_aretes=cpt_aretes+1;
	  if(cpt_aretes==M) break;
	}while(indic_E[e_max]==true);
     
      if(cpt_aretes==M) break;
      
      //1. Computing the edges of the plateau LCP linked to the edge e_max
      LifoPush(LIFO, e_max);
      indic_P[e_max]=true;
      indic_E[e_max]=true;
      LifoPush(LCP, e_max);
      nb_vertices=0;
      nb_edges = 0;
      wmax = weights[e_max];
 
      // 2. putting the edges and vertices of the plateau into arrays 
      while (! LifoVide(LIFO))
	{
	  x = LifoPop(LIFO);
	  e1 = edges[0][x]; e2 = edges[1][x];
	  re1 = element_find(e1, Fth );
	  re2 = element_find(e2, Fth );
	  if (proba[0][re1]<0 || proba[0][re2]<0) 
	    {
	      if (indic_VP[e1]==0) 
		{
		  LCVP[nb_vertices]=e1;
		  nb_vertices++;
		  indic_VP[e1]=1;
		}
	      if (indic_VP[e2]==0) 
		{
		  LCVP[nb_vertices]=e2;
		  nb_vertices++;
		  indic_VP[e2]=1;
		}
	      edgesLCP[0][ nb_edges] = e1;
	      edgesLCP[1][ nb_edges] = e2;
	      NEs[nb_edges]=x;
	    
	      nb_edges ++;
	    }

	  for (k = 1; k <= nb_neighbor_edges; k++) 
	    {
	      if (ds>1)
		y = neighbor_edge_3D(e1, e2, x, k, rs, cs, ds);
	        else y = neighbor_edge(x, k, rs, cs, ds);
	      if (y != -1)
		if ((indic_P[y]==false) && (weights[y] == wmax))
		  {
		    indic_P[y]=true;
		    LifoPush(LIFO, y);
		    LifoPush(LCP, y);
		    indic_E[y]= true;
		  } 
	    }
	}
      for (j=0;j<nb_vertices;j++)
	indic_VP[LCVP[j]]=0;
      for (j=0;j<LCP->Sp;j++) 
	indic_P[LCP->Pts[j]]=false;

      // 3. If e_max belongs to a plateau
      if (nb_edges > 0)
	{
	  // 4. Evaluate if there are differents seeds on the plateau
	  p=0;  
	  different_seeds = false;
	  
	  for (i=0;i<nb_labels-1;i++)
	    { 
	      val = -0.5;
	      for (j=0;j<nb_vertices;j++)
		{
		  
		  x = LCVP[j];
		  xr = element_find(x, Fth);
		  if(fabs(proba[i][xr]-val)>epsilon && proba[i][xr]>=0 ) 
		    {
		      p++; val = proba[i][xr]; 
		    }
		}
	      if (p>=2) 
		{
		  different_seeds = true;
		  break;
		}
	      else p=0;
	    }

	  if (different_seeds == true)
	    {
	      // 5. Sort the edges of the plateau according to their normal weight
	      for(k=0;k<nb_edges;k++)
		sorted_weights[k]=normal_weights[NEs[k]]; 
			
	    
	      
	      if (quicksort == true)
		BucketSort(sorted_weights, NEs, nb_edges , max_weight+1);
	      else 
		TriRapideStochastique_dec(sorted_weights,NEs, 0,nb_edges-1); 


	      // Merge nodes for edges of real max weight
	      nb_vertices=0;
	      Nnb_edges = 0;
	      for(Ncpt_aretes = 0; Ncpt_aretes< nb_edges; Ncpt_aretes++)
		{
		  Ne_max=NEs[Ncpt_aretes];
		  e1 = edges[0][ Ne_max];
		  e2 = edges[1][ Ne_max];
		  if (normal_weights[Ne_max] != wmax)
		    merge_node (e1, e2,  Rnk, Fth, proba, nb_labels);
		  else 
		    {
		      re1 = element_find(e1, Fth );
		      re2 = element_find(e2, Fth );
		      if ((re1 !=re2)&&((proba[0][re1]<0 || proba[0][re2]<0)))
			{
			  if (indic_VP[re1]==0) 
			    {
			      LCVP[nb_vertices]=re1;
			      nb_vertices++;
			      indic_VP[re1]=1;
			    }
			  if (indic_VP[re2]==0) 
			    {
			      LCVP[nb_vertices]=re2;
			      nb_vertices++;
			      indic_VP[re2]=1;
			    }
			  edgesLCP[0][ Nnb_edges] = re1;
			  edgesLCP[1][ Nnb_edges] = re2;
			  Nnb_edges ++;
			}
		    }
		}
	      for (i=0;i<nb_labels-1;i++)
		{ 
		  k=0;
		  for (j=0;j<nb_vertices;j++)
		    {
		      xr = LCVP[j];
		      if (proba[i][xr]>=0)
			{
			  local_labels[i][k] = proba[i][xr];
			  local_seeds[k] = xr;
			  k++;
			}
		    }
		}
		      
	      // 6. Execute Random Walker on plateaus

	      if(nb_vertices<SIZE_MAX_PLATEAU)
		success = RandomWalker(edgesLCP, Nnb_edges, LCVP, indic_VP, nb_vertices, local_seeds, local_labels, k, nb_labels, proba);
	      if ((nb_vertices>=SIZE_MAX_PLATEAU)||(success==false))
		{
		  printf("Plateau of a big size (%d vertices,%d edges) the RW is not performed on it\n", nb_vertices, Nnb_edges);
		  for (j=0;j<Nnb_edges;j++)
		    {
		      e1 = edgesLCP[0][j];
		      e2 = edgesLCP[1][j];
		      merge_node (e1, e2,  Rnk, Fth, proba, nb_labels);
		    }
		}
	    
	      for (j=0;j<nb_vertices;j++)
		indic_VP[LCVP[j]]=0;
	    }
	  else // if different seeds = false 
	    // 7. Merge nodes for edges of max weight
	    {
	      for (j=0;j<nb_edges;j++)
		{
		  e1 = edgesLCP[0][j];
		  e2 = edgesLCP[1][j];
		  merge_node (e1, e2,  Rnk, Fth, proba, nb_labels);
		}
	    }
	}
      LifoFlush(LCP);
    } // end main loop
 
  //building the final proba map (find the root vertex of each tree)
  for (i=0; i<N; i++) 
    {
      j=i;
      xr = i;
      while(Fth[i] != i)
	{ 
	  i = xr;
	  xr = Fth[i];
	}
      for(k=0; k< nb_labels-1;k++) proba[k][j] =proba[k][i];
      i=j;
    }

  //writing results
  struct xvimage * temp;
  temp = allocimage(NULL, rs, cs, ds, VFF_TYP_1_BYTE);

  unsigned char *Temp = UCHARDATA(temp);
#ifndef SPEEDY
  unsigned char *Temp2 = UCHARDATA(img_proba);
#endif     
  double maxi;
     for (j = 0; j < N; j++)
     {
       maxi=0; argmax = 0; val =1;
       for(k=0; k< nb_labels-1;k++)
	 {
	   if(proba[k][j]> maxi) 
	     { 
	       maxi = proba[k][j] ;
	       argmax = k;
	     }
	   val = val - proba[k][j];
	 
	 }  
       if (val>maxi) argmax = k;
       Temp[j] = ((argmax)*255)/(nb_labels-1);
     } 
   
#ifndef SPEEDY
     for (j = 0; j < N; j++)
       Temp2[j] = (unsigned char)(255-255*proba[0][j]); 
#endif     

  // free memory 
  LifoTermine(LCP);
  LifoTermine(LIFO);

  for (i=0;i<2;i++) 
    free(edges[i]); free(edges);
  
  for (i=0;i<2;i++) 
    free(edgesLCP[i]); free(edgesLCP);
  
  free(Rnk);
  free(local_seeds);
  for (i=0; i<nb_labels-1; i++)  free(local_labels[i]);
  free(local_labels);
  
  free(LCVP);
  free(Es);
  free(NEs);
  free(indic_E);
  free(indic_VP);
  free(indic_P);
  free(Fth);
  for (i=0; i<nb_labels-1; i++)  free(proba[i]);
  free(proba);
  free(sorted_weights);
 
 return temp;
}
