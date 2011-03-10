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
#include <math.h>
#include <stdint.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <mcprobas.h>

static int32_t Randx = 237465;

/* --------------------------------------------------------------------------
 * Function      : Prob
 * Result        : double
 * Parameters    : void
 *    Name              Type              Usage
 *
 * Functionality : [0, 1[ uniform probability distribution
 * -------------------------------------------------------------------------*/

double Prob(void)
{
  static double divis = 2147483648.0;  /*  = 2**31, cycle of rand()  */

  Randx = Randx * 1103515245 + 12345;

  return(((double) (Randx & 0x7fffffff)) / divis);
}

/* --------------------------------------------------------------------------
 * Function      : ResetProb
 * Result        : void
 * Parameters    :
 *    Name              Type              Usage
 *    num               int32_t               New value for the random number
 *                                        generator
 *
 * Functionality : Reset probability
 * -------------------------------------------------------------------------*/

void ResetProb(int32_t num)
{
  Randx = (int32_t) num;
}


/* --------------------------------------------------------------------------
 * Function      : Erlang
 * Result        : double
 * Parameters    :
 *    Name              Type              Usage
 *    mean              double            mean
 *    variance          double            variance
 *
 * Functionality : Calculate the erlang distribution
 * -------------------------------------------------------------------------*/

double Erlang(double mean, double variance)
{
  int32_t k, i;
  float a, y;
  double prod;

  k = (mean*mean/variance + 0.5);
  k = (k > 0) ? k : 1;
  a = k/mean;
  prod = 1.0;
  for(i=1; i<=k; i++)
    prod *= Prob();

  y = -log(prod)/a;

  return(y);
}


/* --------------------------------------------------------------------------
 * Function      : Expntl
 * Result        : double
 * Parameters    :
 *    Name              Type              Usage
 *    mean              double            mean
 *
 * Functionality : Calculate the exponential distribution
 * -------------------------------------------------------------------------*/

double Expntl(double mean)
{
  double x;

  x = Prob();
  return(-mean * log(x));
}


/* --------------------------------------------------------------------------
 * Function      : HyperExpntl
 * Result        : double
 * Parameters    :
 *    Name              Type              Usage
 *    mean              double            mean
 *    variance          double            variance
 *
 * Functionality : Return the random derivate drawn from a hyperexponential
 *                 distribution with mean mean and variance variance
 * -------------------------------------------------------------------------*/

double HyperExpntl(double mean, double variance)
{
  float z,p,d;
  double y;

  z = variance / (mean*mean);
  p = 0.5 * (1.0 - sqrt((z - 1.0) / (z + 1.0)));

  d = (Prob() > p) ? 1.0 - p : p;
  y = -mean * log(Prob()) / (2.0 * d);

  return(y);
}


/* --------------------------------------------------------------------------
 * Function      : Normal
 * Result        : double
 * Parameters    :
 *    Name              Type              Usage
 *    mean              double            mean
 *    deviation         double            standard deviation
 *
 * Functionality : Return the random derivate drawn from a normal
 *                 distribution with mean mean and standard deviation
 *                 deviation
 * -------------------------------------------------------------------------*/

double Normal(double mean, double deviation)
{
  int32_t i;
  double sum, y;

  sum = 0.0;
  for(i=1; i <= 12; i++)
    sum += Prob();

  y = deviation * (sum - 6.0) + mean;

  return(y);
}

/* --------------------------------------------------------------------------
 * Function      : Random
 * Result        : int32_t
 * Parameters    :
 *    Name              Type              Usage
 *    low               int32_t               Low limit
 *    high              int32_t               High limit
 *
 * Functionality : Return an integer random number in [low, high]
 * -------------------------------------------------------------------------*/

int32_t Random(int32_t low, int32_t high)
{
  return(low + (high-low+1)*Prob());
}

/* --------------------------------------------------------------------------
 * Function      : Uniform
 * Result        : double
 * Parameters    :
 *    Name              Type              Usage
 *    low               double            low limit
 *    high              double            high limit
 *
 * Functionality : Floating point uniform probability
 * -------------------------------------------------------------------------*/

double Uniform(double low, double high)
{
  return(low+(high-low)*Prob());
}



