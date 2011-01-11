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
#include <mccodimage.h>
#include <mclifo.h>
#include <mcutil.h>
#include <gageodesic.h>
#include <ccsort.h>
#include <union_find.h>
#include <cccodimage.h>
#include <powerwatersegm.h>

uint32_t MAX;

/*================================================*/
void element_link_geod_dilate( int n,
			       int p,
			       int *Fth, 
			       uint32_t *G,
			       uint32_t  *O)
/*================================================*/  
{ 
  int r = element_find(n, Fth);
  
  if (r != p)
    {
      if((G[r] == G[p])||(G[p]>=O[r]))
	{
	  Fth[r] = p;
	  O[p] = mcmax(O[r],O[p]);
	}
      else O[p] = MAX;
    } 
}


/* ==================================================================================================== */
void gageodilate_union_find( uint32_t  *F,  /* f : image seeds */ 
			     uint32_t  *G,  /* g : image weights */
			     uint32_t * O,  /* O : result of the reconstruction by dilation of g under f */
			     int ** edges,  /* list of couple of vertices forming edges*/
			     int32_t rs,    /* row size */
			     int32_t cs,    /* col size */
			     int32_t ds,    /* depth size */
			     int max_weight, 
			     bool quicksort)  
/* ===================================================================================================== */
/* reconstruction by dilation of g under f.  Union-find method described by Luc Vicent. */
{
 
  int k, p,i,n;
  int M = ds*rs*(cs-1)+ds*(rs-1)*cs+(ds-1)*cs*rs;      /*number of edges*/
  bool * Mrk = (bool*)calloc(M,sizeof(bool));
  int * Fth = (int*)malloc(M*sizeof(int));
  
  // Es : E sorted by decreasing weights
  int * Es = (int*)malloc(M*sizeof(int)); 
  
  for(k=0;k<M;k++) 
    {
      Fth[k]=k; 
      O[k]=F[k];
      F[k]=G[k]; 
      Es[k] = k;
    }
 
  MAX = max_weight; 
  if(quicksort) BucketSortCroiss (F, Es, M, MAX+1);
  else TriRapideStochastique_inc(F, Es, 0, M-1);
  /* first pass */
  if(ds==1)//2D
    {
      for(k=M-1;k>=0;k--)
	{
	  p = Es[k];
	  for (i = 1; i <= 6; i += 1) // parcourt les 6 voisins  
	    {
	      n = neighbor_edge(p, i, rs, cs, ds);
	      if (n != -1)
		if(Mrk[n]) 
		    element_link_geod_dilate(n,p, Fth, G, O);
		  
	      Mrk[p]=true;
	    }
	}
    }
  else // 3D 
    {
      for(k=M-1;k>=0;k--)
	{
	  p = Es[k];
	  for (i = 1; i <= 12; i += 1) // parcourt les 12 voisins  
	    {
	      n = neighbor_edge_3D(edges[0][p], edges[1][p], p, i, rs, cs, ds);
	      if (n != -1)
		if(Mrk[n]) 
		  element_link_geod_dilate(n,p, Fth, G, O);
	      Mrk[p]=true;
	    }
	}
    }

  /* second pass */
 
  for(k=0;k<M;k++)
    {
      p = Es[k];
      if (Fth[p]==p) // p is root
	{
	  if (O[p]==MAX) O[p]=G[p];
	}
      else O[p]= O[Fth[p]];
    }

  free(Es);
  free(Mrk);
  free(Fth);
} 

