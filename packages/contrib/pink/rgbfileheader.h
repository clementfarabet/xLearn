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

struct RGBFILEHEADER {       /* size 108 bytes */
  uint16_t magic;      /* size 2 bytes : magic number = 474 */
  uint8_t compression; /* size 1 byte : 0 for no compression */
  uint8_t bytespercha; /* size 1 byte : nb. bytes per channel */
  uint16_t dim;        /* size 2 bytes : nb. channels */
  uint16_t width;      /* size 2 bytes : image row size */
  uint16_t height;     /* size 2 bytes : image col size */
  uint16_t components; /* size 2 bytes : components */
  uint32_t mincol;      /* size 4 bytes : 0 */
  uint32_t maxcol;      /* size 4 bytes : 255 */
  uint32_t dummy;       /* size 4 bytes : dummy */
  char name[80];             /* size 80 bytes : image name or comment */
  uint32_t cmaptype;    /* size 4 bytes : 0 for NORMAL RGB */
}; /** plus 404 bytes dummy padding to make header 512 bytes **/
#ifdef __cplusplus
}
#endif
