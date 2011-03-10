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
    Approximation d'une courbe discrete par un ensemble de segments de droites

    Auteur: Silvio Jamil Guimaraes - septembre 2001
    D'apres "Optimum Uniform Piecewise Linear Approximation of Planar Curves", J.G. Dunham, 
    IEEE PAMI v8 n1 1986
*/

#include <stdlib.h>
#include <stdint.h>
#include <sys/types.h>
#include <stdio.h>
#include <math.h>

#define P_INV -1

#define ABS(X) ((X)>=0?(X):-(X))
#define MAX(X,Y) ((X)>=(Y)?(X):(Y))
#define MIN(X,Y) ((X)<=(Y)?(X):(Y))

double EuclidianDistance(double px0, double py0, double pxi, double pyi){

	return sqrt((pxi-px0)*(pxi-px0)+(pyi-py0)*(pyi-py0));

}

double LineDistance(double px0, double py0, double pxi, double pyi, double pxk, double pyk){

	double d = 0;
	double coseno = 0;
	double seno = 0;
	double aux = 0;

	double pk_x = 0;
	double pk_y = 0;

	aux = sqrt((pxi-px0)*(pxi-px0)+(pyi-py0)*(pyi-py0));
	if (aux != 0)
		coseno = ((pxi-px0))/(aux);
	else coseno = 0;

	if (aux != 0)
		seno = ((pyi-py0))/(aux);
	else seno = 0;


	pk_x = (double) ((pxk - px0)*coseno + (pyk-py0)*seno);
	pk_y = (double) ((-1)*(pxk - px0)*seno + (pyk-py0)*coseno);

	if (pk_x < 0){
		d = sqrt((pxk-px0)*(pxk-px0)+(pyk-py0)*(pyk-py0));
	}else if (pk_x > pxi){
		d = sqrt((pxk-pxi)*(pxk-pxi)+(pyk-pyi)*(pyk-pyi));
	}else{
		d = ABS(pk_y);
	}

	return d;
}
/*

	Function to avaliate the distance criterion to
	consider pi as a line point

	return
		1:	respect the distance considerations
		0:	otherwise

*/

int32_t DistanceAnalysis (double* t_min,double* t_max,double* vector_x, double* vector_y, int32_t p0, int32_t pi, double epsilon, double * distance){

	double max = 0; //Distance maximum
        double c, d;
	int32_t i;	//auxiliar
	
	for (i = p0; i <= pi; i++){
		c = EuclidianDistance(vector_x[p0],vector_y[p0],vector_x[i],vector_y[i]);
		distance[i] = c;
		max = MAX(max,c);
	}//for (i

	if (max < epsilon)
		return 1;

	if (distance[pi] >= max){
		for (i = p0+1; i <= pi-1; i++){
			d = LineDistance(vector_x[p0],vector_y[p0],vector_x[pi],vector_y[pi],vector_x[i],vector_y[i]);
			if (d > epsilon){
				return 0;
			} //if
		}//for (i
		return 1;
	}//if
	else {
		for (i = p0+1; i <= pi-1; i++){
			d = LineDistance(vector_x[p0],vector_y[p0],vector_x[pi],vector_y[pi],vector_x[i],vector_y[i]);
			if (d > epsilon){
				return 0;
			} //if
		}//for (i
		return 1;
	} //else

	return 0;

}

/*
	Calculate the phi number from two points (ArcTangent)
*/
double Phi (double p0_x, double p0_y, double pi_x, double pi_y){
	if ((pi_x!=p0_x))
		return atan(((double)(((pi_y)-(p0_y))))/((pi_x-p0_x)));//(ABS((pi_y)-(p0_y))))/ABS((pi_x-p0_x)));
	else return 0;
}

/*
	Calculate the psi number from two points (ArcSin)
	associated with a error number
*/
double Psi (double p0_x, double p0_y, double pi_x, double pi_y, double error){
        double x;
	if ((pi_x==p0_x) && (pi_y==p0_y))
		return 0;
	x = ((error))/( sqrt(( (pi_y-p0_y)*(pi_y-p0_y) ) + ( (pi_x-p0_x)*(pi_x-p0_x) )) );
	
	/*
		The asin argument must be between -1 and 1
	*/
	if (x > 1)
		x = 1;
	else if (x < -1)
		x = -1;
	return asin(x);
}


void TESAO(double* t_min,double* t_max,double* vector_x, double* vector_y, int32_t p0, int32_t pi, double epsilon){

	int32_t i = 0;
        double b, c, min, max;
	for (i = p0; i <= pi; i++){
		b = Phi(vector_x[p0],vector_y[p0],vector_x[i],vector_y[i]);
		c = Psi(vector_x[p0],vector_y[p0],vector_x[i],vector_y[i],epsilon);
		t_min[i] = b-c;//ABS(c);
		t_max[i] = b+c;//ABS(c);
	}

	min = -1000;
	max = 1000;
	for (i = p0+1; i <= pi; i++){
		min = MAX(min,t_min[i]);
		max = MIN(max,t_max[i]);
	}

	t_min[pi] = min;
	t_max[pi] = max;

}

void SJCurveAproximate(double* vector_x, 
		       double* vector_y, 
		       int32_t cont, // nb de points
		       double epsilon)
{

/*
-----------------------------------------
	Straight line detection
-----------------------------------------
*/

	int32_t l;

	int32_t P0 = 0;
	int32_t PI = cont - 1;

        double phi = 0;
	double psi = 0;
	double teta_min = 0;
	double teta_max = 0;
	double * distance = (double *)calloc(1,cont * sizeof(double));
	double *t_min = (double *)calloc(1,cont * sizeof(double));
	double *t_max = (double *)calloc(1,cont * sizeof(double));

	int32_t LINE_DETECTED = 0;
	int32_t start = 0;
	int32_t end = cont-1;
        int32_t d;
	int32_t segments = 0;

printf("eps = %g\n", epsilon);

	for (l = 0; l < cont; l++){
		t_min[l] = 0;
		t_max[l] = 0;
		distance[l] = 0;
	}


	while (start != end){
		LINE_DETECTED = 1;
		for (l = 0; l < cont; l++){
			t_min[l] = 0;
			t_max[l] = 0;
		}

		PI = start+1;
		while ((PI != end) && (LINE_DETECTED)){
	
			phi = Phi(vector_x[P0],vector_y[P0],vector_x[PI],vector_y[PI]);
			psi = Psi(vector_x[P0],vector_y[P0],vector_x[PI],vector_y[PI],epsilon);

			TESAO(t_min,t_max,vector_x, vector_y, P0, PI ,epsilon);
			teta_min = t_min[PI];
			teta_max = t_max[PI];

			if ((teta_max==teta_min) && (teta_max == 0)){
				printf("No line between ");
				LINE_DETECTED = 0;
				printf("(%g,%g) => (%g,%g)\n",vector_x[P0],vector_y[P0],vector_x[PI],vector_y[PI]);
			}else{
				if (teta_max==teta_min){
				printf("Exact line\n");
					LINE_DETECTED = 1;
//					printf("(%g,%g) => (%g,%g)\n",vector_x[P0],alt-1-vector_y[P0],vector_x[PI],alt-1-vector_y[PI]);
				}else {
					phi = Phi(vector_x[P0],vector_y[P0],vector_x[PI],vector_y[PI]);
					if ((phi <= teta_max) && (phi >= teta_min)){
//						printf("Pontential\n");
						LINE_DETECTED = 1;
//						printf("(%g,%g) => (%g,%g)\n",vector_x[P0],alt-1-vector_y[P0],vector_x[PI],alt-1-vector_y[PI]);
					}else{
						LINE_DETECTED = 0;
//						printf("No Potential\n");
					}
				}
				if (LINE_DETECTED == 1){
					d = DistanceAnalysis(t_min,t_max,vector_x, vector_y, P0, PI ,epsilon, distance);
//					printf("%i\n",d);
					if (d == 0)
						LINE_DETECTED = 0;

				}
			}
			PI++;
		}

		printf("(%g,%g) => (%g,%g)\n", vector_x[P0], vector_y[P0], vector_x[PI], vector_y[PI]);

		end = cont - 1;
		start = PI;
		P0 = start;
		PI = end;
		segments++;
	}
	printf("Number of segments by component: %i\n", segments);
  free(distance);
  free(t_min);
  free(t_max);

}
