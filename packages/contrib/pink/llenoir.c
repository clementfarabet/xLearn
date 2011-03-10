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
   Calcul d'une géodésique dans un espace discret (2D ou 3D)
   à partir de algorithmes de M. Alexandre Lenoir

   Massieu Julien & Parly Sébastien - Février 2005 
*/
#include <stdio.h>
#include <stdint.h>
#include <sys/types.h>
#include <stdlib.h>
#include <mcimage.h>
#include <mccodimage.h>
#include <mcfifo.h>
#include "llenoir.h"

/* voisinage 2D
3 2 1
4 X 0
5 6 7
*/

void Lenoir(struct xvimage * image, 
            int32_t x, 
	    int32_t y, 
	    int32_t connex, //type de connexité (4 6 8 18 26)
	    struct xvimage * out) //image contenant la geodesique
{
	/* Déclarations */
	struct xvimage *etat, *distance, *path, *paththrough, *pathfrom; //xvimage spour stocker les proprietes des pixels
	int32_t rs, cs, ps, ds, N, s, s1, s2, Ndim;
	int32_t direction, i; //compteur de boucle
	int32_t testX, testY;// test de boucle
	double maxi;
	uint8_t *ptrimage, *ptretat, *ptrout;
	double *ptrdistance, *ptrpath, *ptrpaththrough, *ptrpathfrom;
	Fifo * F;//Fifo F
	
	rs = rowsize(image);
	cs = colsize(image);
	ds = depth(image);
	ps = rs*cs;
	N = ps*ds;
	F = CreeFifoVide(N);
	
	/* Alouer les images memoire */
	etat = allocimage(NULL, rs, cs, ds, VFF_TYP_1_BYTE);
	distance = allocimage(NULL, rs, cs, ds, VFF_TYP_DOUBLE); //attention TYPE DOUBLE!
	path = allocimage(NULL, rs, cs, ds, VFF_TYP_DOUBLE);//attention TYPE DOUBLE!
	paththrough = allocimage(NULL, rs, cs, ds, VFF_TYP_DOUBLE);//attention TYPE DOUBLE!
	pathfrom = allocimage(NULL, rs, cs, ds, VFF_TYP_DOUBLE);//attention TYPE DOUBLE!
	
	/* initialisations pointeurs */
	ptrimage = UCHARDATA(image);
	ptretat = UCHARDATA(etat);
	ptrout = UCHARDATA(out);
	ptrdistance = DOUBLEDATA(distance);
	ptrpath = DOUBLEDATA(path);
	ptrpaththrough = DOUBLEDATA(paththrough);
	ptrpathfrom = DOUBLEDATA(pathfrom);
	
	/* Connexite */
	if(connex==4||connex==8) Ndim = 2;
	if(connex==6||connex==18||connex==26) Ndim = 3;
	
	
	
	/* Blindage, x ou y n'appartient pas a l'objet*/
	if(ptrimage[x]!=255||ptrimage[y]!=255) 
	{
    		fprintf(stderr, "Lenoir: les points ne sont pas dans l'objet\n");
    		exit(0);
  	}
	
	
	/***** Premiere partie algo *****/
	for (i=0; i<N; i++)
	{
		ptretat[i] = 0;
		ptrdistance[i] = 0;
		ptrpath[i] = 0;
	}
	
	ptrpath[x] = 1;
	ptretat[x] = 1;
	FifoPush(F, x);
	testY = 0;

	while ((!FifoVide(F)) && (testY!=1) )//Tant que la liste n'est pas vide
	{
		s = FifoPop(F); //on recupère l'élement en tête de pile
		if(s==y)//Si l'element depile est y
		{
			testY = 1;
		}
		else if(ptrimage[s]==(uint8_t)255) //si le pixel d'index s est blanc 
		{
			if(Ndim==2) //2D
			{	
				for(direction=0; direction<8; direction=direction+1)//parcours des 8 voisins de s
				{
					s1 = voisin(s,direction,rs,N);//s1 un  8-voisin de s
					if((connex==4 && voisins4(s, s1, rs)) || connex==8)
					{
						if(ptretat[s1]==0)
						{
							ptrdistance[s1]=(ptrdistance[s]+1);
							ptrpath[s1]=ptrpath[s];
							FifoPush(F, s1);
							ptretat[s1]=1;
						} 
						else if(ptrdistance[s1]==(ptrdistance[s]+1))
						{
							ptrpath[s1]=ptrpath[s1]+ptrpath[s];
						}
					}
				}
			}
			if(Ndim==3) //3D
			{
				for(direction=0; direction<26; direction=direction+1)//parcours des 26 voisins de s
				{
					if(connex==6) s1 = voisin6(s,direction,rs,ps,N);
					if(connex==18) s1 = voisin18(s,direction,rs,ps,N);
					if(connex==26) s1 = voisin26(s,direction,rs,ps,N);
					if(ptretat[s1]==0)
					{
						ptrdistance[s1]=(ptrdistance[s]+1);
						ptrpath[s1]=ptrpath[s];
						FifoPush(F, s1);
						ptretat[s1]=1;
					}	 
					else if(ptrdistance[s1]==(ptrdistance[s]+1))
					{
							ptrpath[s1]=ptrpath[s1]+ptrpath[s];
					}
				}
			}
		}
	}	
	FifoFlush(F);//Vide la pile


	/***** Deuxieme partie algo *****/
	for (i=0; i<N; i++)
	{
		ptrpaththrough[i] = 0;
		ptrpathfrom[i] = 0;
	}
	
	ptrpathfrom[y] = 1;
	ptretat[y] = 2;
	
	FifoPush(F, y);
	s = y;
	testX = 0;

	while ((!FifoVide(F)) && (testX!=1))//Tant que la liste n'est pas vide et que l'élément dépilé n'est pas x
	{
		s = FifoPop(F); //on recupère l'élement en tête de pile
		//fprintf(stdout,"%d - %d : %d\n",ptrpathfrom[s],ptrpath[s],ptrpathfrom[s]*ptrpath[s]);
		ptrpaththrough[s] = ptrpath[s]*ptrpathfrom[s];
		//fprintf(stdout,"%d : %d\n",s,ptrpaththrough[s]);
		if(s==x)//Si l'element depile est y
		{
			testX = 1;
		}
		else if(ptrimage[s]==(uint8_t)255) //si le pixel d'index s est blanc 
		{
			if(Ndim==2) //2D
			{
				for(direction=0; direction<8; direction=direction+1)//parcours des 8 voisins de s
				{
					s1 = voisin(s,direction,rs,N);//s1 un  8-voisin de s
					if((connex==4 && voisins4(s, s1, rs)) || connex==8)
					{
						if(ptrdistance[s1]<ptrdistance[s])
						{
							if(ptretat[s1]==1)
							{
								ptrpathfrom[s1] = ptrpathfrom[s];
								FifoPush(F, s1);
								ptretat[s1] = 2;
							}
							else
							{
								ptrpathfrom[s1] = ptrpathfrom[s] + ptrpathfrom[s1];
							}
						}
						
					} 
					
				}
			}
			if(Ndim==3) //3D
			{
				for(direction=0; direction<26; direction=direction+1)//parcours des 26 voisins de s
				{
					if(connex==6) s1 = voisin6(s,direction,rs,ps,N);
					if(connex==18) s1 = voisin18(s,direction,rs,ps,N);
					if(connex==26) s1 = voisin26(s,direction,rs,ps,N);
					if(ptrdistance[s1]<ptrdistance[s])
					{
						if(ptretat[s1]==1)
						{
							ptrpathfrom[s1] = ptrpathfrom[s];
							FifoPush(F, s1);
							ptretat[s1] = 2;
						}
						else
						{
							ptrpathfrom[s1] = ptrpathfrom[s] + ptrpathfrom[s1];
						}
					}
				}
			}
		}
	}	
	FifoFlush(F);//Liberer la pile
	

	/*******3eme partie algo*******/
	s2 = x;
	s = y;
	maxi = 0;
	ptretat[y] = 3;
	FifoPush(F, y);
	testX = 0;

	while (testX!=1 )//Tant que la liste n'est pas vide et que l'élément dépilé n'est pas x
	{
		s = FifoPop(F); //on recupère l'élement en tête de pile
		maxi = 0;
		if(s==x)//Si l'element depile est y
		{
			testX = 1;
		}
		else /*while(s!=x) {*/
		if(ptrimage[s]==(uint8_t)255) //si le pixel d'index s est blanc 
		{
			if(Ndim==2) //2D
			{
				for(direction=0; direction<8; direction=direction+1)//parcours des 8 voisins de s
				{
					s1 = voisin(s,direction,rs,N);//s1 un  8-voisin de s
					if(((connex==4) && voisins4(s, s1, rs)) || (connex==8))
					{
					  if((ptrdistance[s1]<ptrdistance[s]) && (ptrpaththrough[s1]>maxi))
						{
							s2 = s1;
							maxi = ptrpaththrough[s1];
						}
					}
				}
			}
			if(Ndim==3) //3D
			{
				for(direction=0; direction<26; direction=direction+1)//parcours des 26 voisins de s
				{
					if(connex==6) s1 = voisin6(s,direction,rs,ps,N);
					if(connex==18) s1 = voisin18(s,direction,rs,ps,N);
					if(connex==26) s1 = voisin26(s,direction,rs,ps,N);
					if((ptrdistance[s1]<ptrdistance[s]) && (ptrpaththrough[s1]>maxi))
					{
						s2 = s1;
						maxi = ptrpaththrough[s1];
					}
				}
			}
			s = s2;
			ptretat[s] = 3;
			FifoPush(F, s);
		}
	}	
	FifoTermine(F);//Liberer la pile
	
	/* Il n'y a pas de géodésique entre x et y */
	if(ptrpath[y] < 1) 
	{
    		fprintf(stderr, "Lenoir: Les points source et target ne sont pas connexes\n");
    		exit(0);
  	}
	
        /* Remplir l'image de sortie (2D) */
	if(Ndim==2)
	{
		for (i=0; i<N; i=i+1)
		{
			if(ptrdistance[i] <= ptrdistance[y] && (ptrdistance[i] != 0 || x==i)) ptrout[i] = 110;
			if(ptretat[i] == 2)  ptrout[i] = 160;
			if(ptretat[i] == 3)  ptrout[i] = 210;
		}
	}
	
	/* Remplir l'image de sortie (3D) */
	if(Ndim==3)
	{
		for (i=0; i<N; i=i+1)
		{
			if(ptrimage[i] == 255)  ptrout[i] = 110;
			if(ptretat[i] == 3)  ptrout[i] = 255;
		}
	}
	
	/* Libérer mémoire */
	freeimage(etat);
	freeimage(distance);
	freeimage(path);
	freeimage(paththrough);
	freeimage(pathfrom);
}
