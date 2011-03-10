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
extern skel * limage2skel(struct xvimage *image, int32_t connex, int32_t len);
extern skel * limage2skel2(struct xvimage *image, struct xvimage *morejunctions, int32_t connex);
  extern struct xvimage * lskel2image(skel *S, int32_t id);
extern struct xvimage * lskelmarked2image(skel *S);
extern int32_t lskelmarkvertex(skel *S, int32_t vertex_id);
extern int32_t lskelfilter1(skel *S, double length, double delta1, double delta2);
extern int32_t lskelfilter1a(skel *S, double delta1, double delta2, double theta, int32_t length);
extern int32_t lskelfilter2(skel *S, double delta1, double delta2);
extern int32_t lskelfilter2b(skel *S, double delta1, double delta2);
extern struct xvimage * lskelfilter3(skel *S, double delta1, double delta2, double maxbridgelength, double maxelbowangle);
extern struct xvimage * lskelfindelbows(skel *S, double length, double angle);
#ifdef __cplusplus
}
#endif
