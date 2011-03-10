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

/****************************************************************
*
* Purpose:      useful functions to compute neigbors of points, edges, and gradient of images 
*
* Written By:   Camille Couprie, avril 2009 
*
****************************************************************/

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <math.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include <mccodimage.h>
#include <mcimage.h>
#include <mcindic.h>
#include <mclifo.h>
#include <mcutil.h>

#include <cccodimage.h>
#include <gageodesic.h>
#include <powerwatersegm.h>
#include <image_toolbox.h>
 

/* ======================================================================== */
int neighbor_edge_3D(int node1, /* index of node 1 */
		     int node2, /* index of node 2 */
		     int i,     /* edge index */
		     int k,     /* number of the desired neighbor of edge i */
		     int rs,    /* rowsize of the image */
		     int cs,    /* colsize of the image */
		     int ds)    /* depth of the image */
/* ======================================================================== */
/* return the index of the k_th neighbor
  
%      3       _|_       
%  1 2 0 4 5   _|_       
%  6 7 0 9 10   |        
%      8                 
%                        
% return -1 if the neighbor is outside the image


% example of indexing of the edges in 3D

%        27      28
%       _____ ________
%     /      /       /|
%  12/    13/     14/ |
%   /__6___/___7__ /  | 23
%   |      |      |   |
%  0|     1|     2|17/|
%   |      |      | / |
%   |__8___|___9__|/  | 26
%   |      |      |   |
%  3|     4|     5|20/
%   |      |      | /
%   |__10__|__11__|/

*/
{
  if (ds==1)  return neighbor_edge( i, k, rs, cs, ds); 
  int index=-1;
  int rs_cs = rs*cs;
  int V = (cs-1)*rs;
  int H = (rs-1)*cs;
  if(k<= 6)
    {
      int zp = node1 % (rs_cs); 
      int z = node1 / (rs_cs);   
      switch(k)
	{
	case 1:
	  if (zp % rs >= rs-1) return -1;
	  else index = (zp+V)-(zp/rs)+z*(V+H+rs_cs);break;
	case 3:
	  if (zp % rs == 0) return -1;
	  else index = (zp+V)-(zp/rs)-1+z*(V+H+rs_cs);break;
	case 2:
	  if (zp / rs >= cs-1) return -1;
	  else index = zp+z*(V+H+rs_cs);break;
	case 4:
	  if (zp / rs == 0) return -1;
	  else index = zp-rs+z*(V+H+rs_cs);break;
	case 5:
	  if (z == 0) return -1;
	  else index = z*(V+H)+zp+(z-1)*rs_cs;break;
	case 6:
	  if (z >= ds-1) return -1;
	  else index = (z+1)*(V+H)+zp+z*rs_cs;break;
	}
    }
  else 
    {
      int zp = node2 % (rs_cs); 
      int z = node2 / (rs_cs);   
      switch(k-6)
	{
	case 1:
	  if (zp % rs >= rs-1) return -1;
	  else index = (zp+V)-(zp/rs)+z*(V+H+rs_cs);break;
	case 3:
	  if (zp % rs == 0) return -1;
	  else index = (zp+V)-(zp/rs)-1+z*(V+H+rs_cs);break;
	case 2:
	  if (zp / rs >= cs-1) return -1;
	  else index = zp+z*(V+H+rs_cs);break;
	case 4:
	  if (zp / rs == 0) return -1;
	  else index = zp-rs+z*(V+H+rs_cs);break;
	case 5:
	  if (z == 0) return -1;
	  else index = z*(V+H)+zp+(z-1)*rs_cs;break;
	case 6:
	  if (z >= ds-1) return -1;
	  else index = (z+1)*(V+H)+zp+z*rs_cs;break;
	}
    }
  if (index == i) return -1;
  return index;
}




/* ================================================================== */
int neighbor_edge(int i,  /* edge index */
		  int k,  /* number of the desired neighbor of edge i */
		  int rs, /* rowsize of the image */
		  int cs, /* colsize of the image */
		  int ds) /* depth of the image */
/* =================================================================== */
/* return the index of the k_th neighbor 
(only works in 2D, a little faster than the neighbor_edge3D)
  
%      1       _|_          
%    2 0 6     _|_          2 1      _|_|_
%    3 0 5      |         3 0 0 6     | | 
%      4                    4 5 
%                           
% return -1 if the neighbor is outside the image


% indexing edges 2D
% 
%    _4_ _5_
%  0|  1|
%   |_6_|_7_
%  2|  3|
%   |   |

*/
{
  int V = (cs-1)*rs; // nb vertical edges 
  if (i >= V)
    {
      //horizontal
      switch(k)
	{
	case 2:
	  if ((i-V) < rs-1) return -1;
	  else return ((i-V)/(rs-1)-1)*rs + ((i - V)%(rs-1));break;
	case 3:
	  if ((i-V)%(rs-1)==0) return -1;
	  else return i-1 ;break;
	case 4:
	  if (i>(rs-1)*cs+ V -rs) return -1;
	  else return ((i-V)/(rs-1)-1)*rs + ((i - V)%(rs-1)) + rs;break;
	case 5:
	  if (i>(rs-1)*cs+ V - rs) return -1;
	  else return ((i-V)/(rs-1)-1)*rs + ((i - V)%(rs-1))  + rs +1;break;
	case 6:
	  if ((i-V)%(rs-1)==rs-2) return -1;
	  else return i+1;break;
	case 1:
	  if (i-V<rs-1) return -1;
	  else return ((i-V)/(rs-1)-1)*rs + ((i - V)%(rs-1))+1;break;
	}
    }
  else
    { //vertical
      switch(k)
	{
	case 6:
	  if (i %rs == rs-1) return -1;
	  else return (i+V)-(i/rs);break;
	case 1:
          if (i < rs) return -1;
          else return i-rs;break;
        case 2:
          if (i%rs==0) return -1;
          else return (i+V)-(i/rs)-1;break;
        case 3:
          if (i%rs==0) return -1;
          else  return (i+V)-(i/rs)-1+rs-1;break;
        case 4:
          if (i>=V-rs) return -1;
          else return i+rs;break;
        case 5:
	  if (i %rs == rs-1) return -1;
	  else return (i+V)-(i/rs)+rs-1;break;
	}
    }
  return -1; //never happens 
}


/* ============================================================================= */ 
int neighbor_node_edge( int i,  /* node index */
			int k,  /* number of the desired edge neighbor of node i */
			int rs, /* rowsize of the image */
			int cs, /* colsize of the image */
			int ds) /* depth of the image */
/* ============================================================================== */
/* return the index of the k_th edge neighbor of the node "i" 
       4   
     3 0 1   5 front slice,  6 back slice
       2 
   return -1 if the neighbor is outside the image */
{
  int rs_cs = rs*cs;
  int zp = i % (rs_cs); 
  int z = i / (rs_cs);   
  int V = (cs-1)*rs;
  int H = (rs-1)*cs;
  switch(k)
    {
    case 1:
      if (zp % rs >= rs-1) return -1;
      else return (zp+V)-(zp/rs)+z*(V+H+rs_cs);break;
    case 3:
      if (zp % rs == 0) return -1;
      else return (zp+V)-(zp/rs)-1+z*(V+H+rs_cs);break;
    case 2:
      if (zp / rs >= cs-1) return -1;
      else return zp+z*(V+H+rs_cs);break;
    case 4:
      if (zp / rs == 0) return -1;
      else return zp-rs+z*(V+H+rs_cs);break;
    case 5:
      if (z == 0) return -1;
      else return z*(V+H)+zp+(z-1)*rs_cs;break;
    case 6:
      if (z >= ds-1) return -1;
      else return (z+1)*(V+H)+zp+z*rs_cs;break;
    case -1:
      return i+rs_cs;break;
    case 0:
     return i + rs_cs*2; break;
    }
  return -1; //never happens 
}


/* =================================================================== */ 
int neighbor( int i,  /* node index */
	      int k,  /* number of the desired neighbor node of node i */
	      int rs, /* rowsize of the image */
	      int cs, /* colsize of the image */
	      int ds) /* depth of the image */
/* =================================================================== */
/* 
  return the index of the k_th neighbor 
       5         From the top :
     3 0 1       4: slice below,  2 : slice above
       6 
  return -1 if the neighbor is outside the image */
 
{
  int rs_cs=rs*cs;
  int z = i / (rs_cs);  
  int x = i / (rs*ds);
  int y = i % (rs);
  
  switch(k)
    {
    case 1:
      if (y >= rs-1) return -1;
      else return i+1;break;	
    case 3:
      if (y == 0) return -1;
      else return i-1;break;
    case 2:
      if (x >= cs-1) return -1;
      else return i+rs;break;
    case 4:
      if (x == 0) return -1;
      else return i-rs;break;
    case 5:
      if (z >= ds-1) return -1;
      else return i+rs_cs;break;
    case 6:
      if (z == 0) return -1;
      else return i-rs_cs;break;
    case 0:
       return rs_cs;break;
    case -1:
      return rs_cs+1; break;
    }
   return -1; //never happens 
}

/* ======================================================================= */
void compute_edges(  int ** edges, /*array of node indexes composing edges */
		     int rs, /* rowsize of the image */
		     int cs, /* colsize of the image */
		     int ds) /* depth of the image */
/* ======================================================================= */
{
  int i,j,k,l,M;
  M=0; 
  int rs_cs = rs*cs;
  for(k=0;k<ds;k++) 
    {
      for(i=0;i<cs;i++) 
	{
	  for(j=0;j<rs;j++)
	    {
	      if(i<(cs-1))
		{
		  edges[0][M]=j+i*rs+k*rs_cs;
		  edges[1][M]=j+(i+1)*rs+k*rs_cs;
		  M++;
		}
	    }
	}
      for(i=0;i<cs;i++) 
	{
	  for(j=0;j<rs;j++)
	    {
	      if(j<(rs-1))
		{
		  edges[0][M]=j+i*rs+k*rs_cs;
		  edges[1][M]=j+1+i*rs+k*rs_cs;
		  M++;
		}
	    }
	}
      if (k != ds-1)
	for(l=k*rs*cs;l<(k+1)*rs_cs;l++) 
	  {
	    edges[0][M]=l;
	    edges[1][M]=l+rs_cs;
	    M++;
	  }
    }
}



/* =============================================================== */
void grey_weights( struct xvimage * image,
		   uint32_t * weights, /* array to store the values of weights on the edges */
		   int ** edges,       /* array of node indexes composing edges */ 
		   int * seeds,        /* array of seeded nodes indexes */
		   int size_seeds,     /* nb of seeded nodes */
		   bool geod,          /* if true, geodesic reconstruction is performed */
		   bool quicksort)     /* true : bucket sort used; false : stochastic sort o(n log n) */
/* =============================================================== */
/* Computes weights inversely proportionnal to the image gradient for grey level images */
{
  int i,M, rs, cs, ds;
  uint8_t *img;

  if (image == NULL)
    {
      fprintf(stderr, "msf_rw: readimage failed\n");
      exit(1);
    }
  rs = rowsize(image);
  cs = colsize(image);
  ds = depth(image);
  img = UCHARDATA(image); 
 
  M = ds*rs*(cs-1)+ds*(rs-1)*cs+(ds-1)*cs*rs;  // number of edges
  if (geod==false)
    {
      for (i=0;i<M;i++)
	weights[i]= 255-mcabs(img[edges[0][i]]-img[edges[1][i]]);
    }
  else 
    {
      int j,k,n;
      uint32_t * weights_tmp = (uint32_t*) malloc(M*sizeof(uint32_t));
      uint32_t * seeds_function =(uint32_t *)calloc(M,sizeof(uint32_t));
      int numvoisins = 4;
      if (ds>1) numvoisins = 6;
      for (i=0;i<M;i++)
	weights_tmp[i]=255-mcabs(img[edges[0][i]]-img[edges[1][i]]) ;
	
      for (j=0;j<size_seeds;j++)
	for (k=1;k<=numvoisins; k++)
	  {
	    n = neighbor_node_edge(seeds[j], k, rs, cs, ds);
	    if (n != -1)
	      seeds_function[n]= weights_tmp[n];
	  } 
     
      gageodilate_union_find(seeds_function, weights_tmp, weights, edges, rs, cs, ds, 255, quicksort);
     
      free(weights_tmp);
      free(seeds_function);
    }
}


/* ======================================================================================================== */
uint32_t * grey_weights_PW(struct xvimage * image,
			   int ** edges,      /*IN: array of node indexes composing edges */
			   int * seeds,       /*IN: array of seeded nodes indexes */
			   int size_seeds,    /*IN : nb of seeded nodes */
			   uint32_t * weights, /*OUT : array to store the reconstructed weights on the edges */
			   bool quicksort)    /*IN : true : bucket sort used; false : stochastic sort o(n log n) */
/* ======================================================================================================== */
/* Computes weights inversely proportionnal to the image gradient for grey level (pgm) images 
   Returns the normal weights and computes the reconstructed weights in the array weights */

{
  int i, M, rs, cs, ds;
  uint8_t *img;

  if (image == NULL)
    {
      fprintf(stderr, "msf_rw: readimage failed\n");
      exit(1);
    }
  rs = rowsize(image);
  cs = colsize(image);
  ds = depth(image);
  img = UCHARDATA(image); 
 
  M = ds*rs*(cs-1)+ds*(rs-1)*cs+(ds-1)*cs*rs;  /*number of edges*/
  uint32_t * normal_weights = (uint32_t *)malloc(sizeof(uint32_t)*M);
  int j,k,n;
  uint32_t * seeds_function =(uint32_t *)calloc(M,sizeof(uint32_t));
  int numvoisins = 4;
  if (ds>1) numvoisins = 6;
  for (i=0;i<M;i++)
    normal_weights[i]=  255-mcabs(img[edges[0][i]]-img[edges[1][i]]);

  for (j=0;j<size_seeds;j++)
    for (k=1;k<=numvoisins; k++)
      {
	n = neighbor_node_edge(seeds[j], k, rs, cs, ds);
	if (n != -1)
	  seeds_function[n]= normal_weights[n];
      } 
  gageodilate_union_find(seeds_function, normal_weights, weights, edges, rs, cs, ds, 255, quicksort);
 
  free(seeds_function);
  return normal_weights;
}


/* ======================================================================== */
void print_gradient( int ** edges, /* array of node indexes composing edges */ 
		     uint32_t * weights, /* weights values on the edges */
		     int rs, /* rowsize of the image */
		     int cs, /* colsize of the image */
		     int ds) /* depth of the image */
		    
/* ======================================================================== */
/* Prints the weight function in a image "gradient.pgm" */
{
  int i,j, k, n;
  int N = rs*cs*ds;
 int numvoisins = 4;
 if (ds>1) numvoisins = 6;
  
 struct xvimage * gradient = allocimage(NULL, rs, cs, ds, VFF_TYP_1_BYTE);
  unsigned char *Temp = UCHARDATA(gradient);
      
  double w = 0;
  for (j = 0; j < N; j++)
    {
      i=0;
      w=0;
      for (k=1;k<=numvoisins;k++)
	{
	  n = neighbor_node_edge(j, k, rs, cs, ds);
	  if (n != -1)
	    {
	      w = w + weights[n];
	      i++;
	    }
	}
      Temp[j] = (uint8_t)((w*2)/i);
    }
  writeimage(gradient, (char*)"gradient.pgm");
  freeimage(gradient);
}


/* ================================================================================================================= */
uint32_t * color_standard_weights_PW( struct xvimage * image_r,
                                      struct xvimage * image_v,
                                      struct xvimage * image_b,
				      uint32_t * weights, /* OUT : array to store the values of weights on the edges */
				      int ** edges,       /* IN : array of node indexes composing edges */ 
				      int * seeds,        /* IN : array of seeded nodes indexes */
				      int size_seeds,     /* IN : nb of seeded nodes */
				      uint32_t * maxi,         /* OUT : the maximum weight value */
				      bool quicksort)     /* IN : true : bucket sort used; false : stochastic sort */
/* ================================================================================================================= */
/* Computes weights inversely proportionnal to the image gradient for 2D color (ppm) images 
   Returns the normal weights and computes the reconstructed weights in the array weights */
{
  *maxi = 0;
  int i,M, rs, cs, ds;

  uint8_t wr, wv, wb;
  uint8_t *img_r;
  uint8_t *img_v;
  uint8_t *img_b;
  
  ds=1;//default
 
  rs = rowsize(image_r);
  cs = colsize(image_r);
  img_r = UCHARDATA(image_r);
  img_v = UCHARDATA(image_v);
  img_b = UCHARDATA(image_b);
    M = ds*rs*(cs-1)+ds*(rs-1)*cs+(ds-1)*cs*rs; 
  uint32_t * normal_weights = (uint32_t *)malloc(sizeof(uint32_t)*M);
	 
  for (i=0;i<M;i++)
    {
      wr = mcabs(img_r[edges[0][i]]-img_r[edges[1][i]]) ;
      wv = mcabs(img_v[edges[0][i]]-img_v[edges[1][i]]) ;
      wb = mcabs(img_b[edges[0][i]]-img_b[edges[1][i]]) ;
      // weights[i] = wr*wr+wv*wv+wb*wb;
      // if (weights[i]> *maxi) *maxi = weights[i];

      weights[i] = 255-wr;
      if (255-wv < weights[i]) weights[i] = 255-wv; 
      if (255-wb < weights[i]) weights[i] = 255-wb;
      *maxi = 255;
    }
 
  //for (i=0;i<M;i++)
  //  normal_weights[i]=*maxi-weights[i];

 for (i=0;i<M;i++)
    normal_weights[i]=weights[i]; 

  int j,k,n;
  
  uint32_t * seeds_function =( uint32_t *)calloc(M,sizeof(uint32_t));
  int numvoisins = 4;
  if (ds>1) numvoisins = 6;
  
  for (j=0;j<size_seeds;j++)
    for (k=1;k<=numvoisins; k++)
      {
	n = neighbor_node_edge(seeds[j], k, rs, cs, ds);
	if (n != -1)
	  seeds_function[n]=normal_weights[n];
      } 
  gageodilate_union_find(seeds_function, normal_weights, weights, edges, rs, cs, ds, *maxi, quicksort);
  free(seeds_function);
    
  return normal_weights;
}

/* ================================================================================================= */
int color_standard_weights( struct xvimage * image_r,
                            struct xvimage * image_v,
                            struct xvimage * image_b,
			    uint32_t * weights, /* array to store the values of weights on the edges */
			    int ** edges,       /* array of node indexes composing edges */ 
			    int * seeds,        /* array of seeded nodes indexes */
			    int size_seeds,     /* nb of seeded nodes */
			    bool geod,          /* if true, geodesic reconstruction is performed */
			    bool quicksort)     /* true : bucket sort used; false : stochastic sort o(n log n) */
/* ================================================================================================== */
/* Computes weights inversely proportionnal to the image gradient for 2D color (ppm) images */
{
  uint32_t maxi = 0;
  int i,M, rs, cs, ds;

  uint8_t wr, wv, wb;
  uint8_t *img_r;
  uint8_t *img_v;
  uint8_t *img_b;
  
  ds=1;//default
 
  rs = rowsize(image_r);
  cs = colsize(image_r);
  img_r = UCHARDATA(image_r);
  img_v = UCHARDATA(image_v);
  img_b = UCHARDATA(image_b);
  M = ds*rs*(cs-1)+ds*(rs-1)*cs+(ds-1)*cs*rs; 
	 
  for (i=0;i<M;i++)
    {
      wr = mcabs(img_r[edges[0][i]]-img_r[edges[1][i]]) ;
      wv = mcabs(img_v[edges[0][i]]-img_v[edges[1][i]]) ;
      wb = mcabs(img_b[edges[0][i]]-img_b[edges[1][i]]) ;
      weights[i] = wr*wr+wv*wv+wb*wb;
      if (weights[i]> maxi) (maxi) = weights[i];
    }
  if (geod==false)
    { 
      for (i=0;i<M;i++)
	weights[i]= maxi-weights[i];
    }
  else
    { 
      int j,k,n;
      uint32_t * weights_tmp = (uint32_t*) malloc(M*sizeof(uint32_t));
      uint32_t * seeds_function =(uint32_t *)calloc(M,sizeof(uint32_t));
      int numvoisins = 4;
      if (ds>1) numvoisins = 6;
      for (i=0;i<M;i++)
	weights_tmp[i]=maxi-weights[i];
	
      for (j=0;j<size_seeds;j++)
	for (k=1;k<=numvoisins; k++)
	  {
	    n = neighbor_node_edge(seeds[j], k, rs, cs, ds);
	    if (n != -1)
	      seeds_function[n]=weights_tmp[n];
	  } 
      gageodilate_union_find(seeds_function, weights_tmp, weights, edges, rs, cs, ds, maxi, quicksort);
      free(weights_tmp);
      free(seeds_function);
    }

  return maxi;
}


/*==============================================*/
struct xvimage * boundary(struct xvimage * image)
/*==============================================*/
//return the inner boundary of a white object in an image 
{
  struct xvimage * output;
  unsigned char *img = UCHARDATA(image);
  int rs = rowsize(image);
  int cs = colsize(image);
  
  output = allocimage(NULL, rs, cs, 1, VFF_TYP_1_BYTE);
  unsigned char *out = UCHARDATA(output);
  for (int i=2; i<rs-1;i++)
    for (int j=2; j<cs-1;j++)
      {
	
	if (img[i-1+j*rs]!=img[i+j*rs] || img[i+1+j*rs]!=img[i+j*rs]  || img[i+(j-1)*rs]!=img[i+j*rs] || img[i+(j+1)*rs]!=img[i+j*rs])
	  {
	    out[i+j*rs] = 255;
	    out[i+1+j*rs] = 255;
	    out[i-1+j*rs] = 255;
	    out[i+(j-1)*rs] = 255;
	    out[i+(j+1)*rs] = 255;
	  }
	else out[i+j*rs] = 0;
      }
  return output; 
}



/*=====================================================*/
struct xvimage * mult_image_value(struct xvimage * image, 
				  double n)
/*=====================================================*/
//multiply image values by a constant number n
{
  struct xvimage * output;
  unsigned char *img = UCHARDATA(image);
  int rs = rowsize(image);
  int cs = colsize(image);
  
  output = allocimage(NULL, rs, cs, 1, VFF_TYP_1_BYTE);
  unsigned char *out = UCHARDATA(output);
  for (int i=0; i<rs*cs;i++)
    out[i] = (unsigned char)(n*img[i]);

  return output; 
}





