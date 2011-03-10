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
extern int bddAlpha3SimpleCube(
  int a, int b, int c, int d, int e, int f, int g, int h, int i, int j, 
  int k, int l, int m, int n, int o, int p, int q, int r, int s, int t, 
  int u, int v, int w, int x, int y, int z);
extern int bddAlpha3SimpleCarreX(int a, int b, int c, int d, int e, int f, int g, int h);
extern int bddAlpha3SimpleCarreY(int a, int b, int c, int d, int e, int f, int g, int h);
extern int bddAlpha3SimpleCarreZ(int a, int b, int c, int d, int e, int f, int g, int h);
extern int bddAlpha3SimpleInterX(int a, int b);
extern int bddAlpha3SimpleInterY(int a, int b);
extern int bddAlpha3SimpleInterZ(int a, int b);
extern int bddBeta3SimpleCarreX(int a, int b);
extern int bddBeta3SimpleCarreY(int a, int b);
extern int bddBeta3SimpleCarreZ(int a, int b);
extern int bddBeta3SimpleInterX(int a, int b, int c, int d, int e, int f, int g, int h);
extern int bddBeta3SimpleInterY(int a, int b, int c, int d, int e, int f, int g, int h);
extern int bddBeta3SimpleInterZ(int a, int b, int c, int d, int e, int f, int g, int h);
extern int bddSurfendCarreX(int a, int b, int c, int d, int e, int f, int g, int h, int i, int j);
extern int bddSurfendCarreY(int a, int b, int c, int d, int e, int f, int g, int h, int i, int j);
extern int bddSurfendCarreZ(int a, int b, int c, int d, int e, int f, int g, int h, int i, int j);
extern int bddSurfendCube(
  int a, int b, int c, int d, int e, int f, int g, int h, int i, int j, 
  int k, int l, int m, int n, int o, int p, int q, int r, int s, int t, 
  int u, int v, int w, int x, int y, int z);
extern int bddSurfsingCarreX(int a, int b, int c, int d, int e, int f, int g, int h, int i, int j);
extern int bddSurfsingCarreY(int a, int b, int c, int d, int e, int f, int g, int h, int i, int j);
extern int bddSurfsingCarreZ(int a, int b, int c, int d, int e, int f, int g, int h, int i, int j);
extern int bddSurfsingCube(
  int a, int b, int c, int d, int e, int f, int g, int h, int i, int j, 
  int k, int l, int m, int n, int o, int p, int q, int r, int s, int t, 
  int u, int v, int w, int x, int y, int z);

#ifdef __cplusplus
}
#endif
