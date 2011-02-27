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
#include <mcchrono.h>
#include <stdint.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>

/*************************************************
	Fonctions de mesure de temps 
	----------------------------
        resolution: 4 microsecondes

    Michel Couprie - ESIEE - juillet 1996
**************************************************/

/* retourne le compteur microsecondes de l'horloge - utile pour la generation aleatoire */
/* ==================================== */
int32_t usecs()
/* ==================================== */
{
   struct timeval tp;
   struct timezone tzp;
   
   if ( gettimeofday(&tp, &tzp) != 0 ) /* code en retour: 0=Ok, -1=erreur */
     fprintf(stderr, "usecs() : gettimeofday failed\n");
   return(tp.tv_usec);
}

/*---------- demarrage chronometrage -------------------*/
/* ==================================== */
void start_chrono(
		 chrono *tp)
/* ==================================== */
{
   struct timezone tzp;
   
   if ( gettimeofday(tp, &tzp) != 0 ) /* code en retour: 0=Ok, -1=erreur */
     fprintf(stderr, "start_chrono() : gettimeofday failed\n");
}

/*----------- Lecture chronometrage : retourne en entier le nb de microsecondes */
/* ==================================== */
int32_t read_chrono(
		chrono *tp)
/* ==================================== */
{
   struct timeval tp2;
   struct timezone tzp;
   
   if ( gettimeofday(&tp2, &tzp) != 0 ) /* code en retour: 0=Ok, -1=erreur */
     fprintf(stderr, "read_chrono() : gettimeofday failed\n");
   if (tp->tv_usec >  tp2.tv_usec)
   {
      tp2.tv_usec += 1000000;
      tp2.tv_sec--;
   }
   return( (tp2.tv_sec - tp->tv_sec)*1000000 + (tp2.tv_usec - tp->tv_usec));
}

/*----------------------------------------------------------------*/
/* ==================================== */
void save_time(
  int32_t n,
  int32_t t,
  char *funcname,
  char *imagename)
/* ==================================== */
{
  char filename[256];
  char *pinkdir;
  FILE *fd = NULL;

  pinkdir = getenv("PINK");
  if (!pinkdir)
  {
    fprintf(stderr, "save_time: environment variable PINK not set\n");
    return;
  }
  strcpy(filename, pinkdir);
  strcat(filename, PERF_DIR);
  strcat(filename, funcname);
  strcat(filename, PERF_EXT);
  fd = fopen(filename,"a");
  if (!fd)
  {
    fprintf(stderr, "save_time: cannot open file: %s\n", filename);
    return;
  }
  /* fprintf(fd, "%9d %9d %s\n", n, t, imagename); */
  fprintf(fd, "%9d\t%9d\n", n, t);
  fclose(fd);
}

/*----------------------------------------------------------------*/
/* ==================================== */
void save_time2(
  int32_t n,
  int32_t n2,
  int32_t t,
  char *funcname,
  char *imagename)
/* ==================================== */
{
  char filename[256];
  char *pinkdir;
  FILE *fd = NULL;

  pinkdir = getenv("PINK");
  if (!pinkdir)
  {
    fprintf(stderr, "save_time: environment variable PINK not set\n");
    return;
  }
  strcpy(filename, pinkdir);
  strcat(filename, PERF_DIR);
  strcat(filename, funcname);
  strcat(filename, PERF_EXT);
  fd = fopen(filename,"a");
  if (!fd)
  {
    fprintf(stderr, "save_time: cannot open file: %s\n", filename);
    return;
  }
  /* fprintf(fd, "%9d %9d %9d %s\n", n, n2, t, imagename); */
  fprintf(fd, "%9d\t%9d\t%9d\n", n, n2, t);
  fclose(fd);
}

/*----------------------------------------------------------------*/
/* ==================================== */
void save_time4(
  int32_t n,
  int32_t n2,
  int32_t na,
  int32_t na2,
  int32_t t,
  char *funcname,
  char *imagename)
/* ==================================== */
{
  char filename[256];
  char *pinkdir;
  FILE *fd = NULL;

  pinkdir = getenv("PINK");
  if (!pinkdir)
  {
    fprintf(stderr, "save_time: environment variable PINK not set\n");
    return;
  }
  strcpy(filename, pinkdir);
  strcat(filename, PERF_DIR);
  strcat(filename, funcname);
  strcat(filename, PERF_EXT);
  fd = fopen(filename,"a");
  if (!fd)
  {
    fprintf(stderr, "save_time: cannot open file: %s\n", filename);
    return;
  }
  /* fprintf(fd, "%9d %9d %9d %9d %9d %s\n", n, n2, na, na2, t, imagename); */
  fprintf(fd, "%9d\t%9d\t%9d\t%9d\t%9d\n", n, n2, na, na2, t);
  fclose(fd);
}

/*----------- Exemple d'utilisation : 
void main()
{
  chrono Chrono1;
  int32_t i;
  start_chrono(&Chrono1);
  for(i = 0; i<1000; i++);
  printf("nb de usec ecoulees = %d\n", read_chrono(&Chrono1));

}
*/
