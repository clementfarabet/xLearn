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
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
/* #include <time.h> */
#include <sys/time.h>

#ifdef HP
struct timeval {
  uint32_t	tv_sec;		/* seconds */
  int32_t		tv_usec;	/* and microseconds */
};

struct timezone {
  int32_t	tz_minuteswest;	/* minutes west of Greenwich */
  int32_t	tz_dsttime;	/* type of dst correction */
};
#endif

typedef struct timeval chrono;


#define PERF_DIR "/perf/"
#define PERF_EXT ".perf"

/* ============== */
/* prototypes for mcchrono.c */
/* ============== */

extern int32_t usecs();

extern void start_chrono(
  chrono *tp
);

extern int32_t read_chrono(
  chrono *tp
);

extern void save_time(
  int32_t n,
  int32_t t,
  char *imagename,
  char *funcname
);

extern void save_time2(
  int32_t n,
  int32_t n2,
  int32_t t,
  char *imagename,
  char *funcname
);

extern void save_time4(
  int32_t n,
  int32_t n2,
  int32_t na,
  int32_t na2,
  int32_t t,
  char *imagename,
  char *funcname
);

#ifdef __cplusplus
}
#endif
