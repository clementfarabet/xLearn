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
typedef struct {
  int32_t N;               /* la dimension de l'espace */
  int32_t *C;              /* les cotes de l'hyperpave */
  int32_t *D;              /* les offsets pour le codage des points */
  uint8_t *S;
} Khalimsky;

extern Khalimsky * AllocKhalimskyNd(int32_t dim, int32_t *userC);
extern void InitNd(int32_t N, int32_t *userC);
extern void TermineNd();
extern int32_t EffaceCliquesSimplesNd(Khalimsky *K);
extern int32_t EnsembleSimpleNd(Khalimsky *K);
extern int32_t CliqueSimpleNd(Khalimsky *K, int32_t x);
extern Khalimsky * KhalimskizeNd(struct xvimage *o);
extern struct xvimage *DeKhalimskizeNd(Khalimsky * K);
extern void Connex8ObjNd(Khalimsky *K);
extern void Connex4ObjNd(Khalimsky *K);
extern void PrintKhalimsky(Khalimsky * K);
extern void InitKhalimsky3Nd(uint32_t c, Khalimsky * K);
extern void InitCubeKhalimsky3Nd(uint32_t c, Khalimsky * K);
extern void InitCubeKhalimsky4Nd(uint32_t c, Khalimsky * K);
extern int32_t CourbeSimpleNd(Khalimsky *K);
extern int32_t SurfaceSimpleNd(Khalimsky *K, Khalimsky *Kp);

#ifdef __cplusplus
}
#endif
