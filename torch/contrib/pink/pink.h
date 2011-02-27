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
#ifndef PINK_H
#define PINK_H

#include <errno.h>
#include <math.h>
#include <assert.h>
#include <stdint.h>
#include <curses.h>
#include <time.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <float.h>
#include <limits.h>
#include <alloca.h>
#include <sys/types.h>
#include <sys/time.h>
#include "sqlite3.h"
#include "bdd.h"
#include "lballincl.h"



//#include <sqlite3ext.h> // not yet needed

#ifndef __cplusplus
#  include <string.h>
#  include <ctype.h>
#endif /* __cplusplus */

#ifdef __cplusplus


#  include <iostream>
#  include <fstream>
#  include <string>
#  include <sstream>
#  include <vector>
#  include <cstdlib>
#  include <cstring>
#  include <cmath>
#  include <locale>

#  include <boost/shared_ptr.hpp>
#  include <boost/shared_array.hpp>
#  include <boost/python/list.hpp>
#  include <boost/python/extract.hpp>
#  include <boost/lexical_cast.hpp>
#  include <boost/thread.hpp>
#  include <boost/algorithm/minmax_element.hpp>
#  include <boost/preprocessor.hpp>
#  include <boost/operators.hpp>
#  include <boost/cast.hpp>
#  include <boost/type_traits/function_traits.hpp>


#endif /* __cplusplus */




#include "mccodimage.h"
#include "mcimage.h"
#include "mccbt.h"
#include "mcchrono.h"
#include "mccolors.h"
#include "mccomptree.h"
#include "mccomptree1d.h"
#include "mcdrawps.h"
#include "mcfah.h"
#include "mcfahpure.h"
#include "mcfahsalembier.h"
#include "mcfifo.h"
#include "mcfusion.h"
#include "mcgeo.h"
#include "mcgraphe.h"
#include "mcindic.h"
#include "mcrbt.h"
#include "mcrbtp.h"
#include "mcmesh.h"
#include "mciomesh.h"
#include "mckhalimsky2d.h"
#include "mckhalimsky3d.h"
#include "mckhalimskyNd.h"
#include "mclifo.h"
#include "mclin.h"
#include "mcliste.h"
#include "mclistechainee.h"
#include "mcprobas.h"
#include "mcset.h"
#include "mcskelcurv.h"
#include "mcsort.h"
#include "mcsplines.h"
#include "mctopo.h"
//#include "mctopo.h.minimal"
#include "mctopo3d.h"
#include "mctree.h"
#include "mcunionfind.h"
#include "mcutil.h"
#include "mcxbib.h"
#include "mcpowell.h"
#include "avscrop.h"
#include "avsimage.h"
#include "jcimage.h"
#include "jcgraphes.h"
#include "jccodimage.h"
#include "jccomptree.h"
#include "jclabelextrema.h"
#include "jclderiche.h"
#include "l2dkhalimsky.h"
#include "l2dcollapse.h"
#include "l3dcollapse.h"
#include "l3dkhalimsky.h"
#include "lMSF.h"
// moved #include "larith.h"
#include "lasft.h"
#include "lattribarea.h"
#include "lattribheight.h"
#include "lattribute.h"
#include "lattribvol.h"
#include "lbarycentre.h"
#include "lblob.h"
#include "lborder.h"
#include "lbresen.h"
#include "lccv.h"
#include "lclick.h"
#include "lcomphisto.h"
#include "lcontours.h"
#include "lconvexhull.h"
#include "lconvol.h"
#include "lconvol3.h"
#include "lcrop.h"
#include "lderiche.h"
#include "ldetectcercles.h"
#include "ldiZenzo.h"
#include "ldilat.h"
#include "ldilatbin.h"
#include "ldilatbin3d.h"
#include "ldilateros.h"
#include "ldilateros3d.h"
#include "ldilatfast.h"
#include "ldilatfast3d.h"
#include "ldilatplan.h"
#include "ldir.h"
#include "ldirections.h"
#include "ldist.h"
#include "ldistgeo.h"
#include "ldraw.h"
#include "ldynamique.h"
#include "ldynamique_grimaud.h"
#include "legalise.h"
#include "lellipsefit.h"
#include "lellipseincl.h"
#include "lepaisgeo.h"
#include "leros.h"
#include "lerosbin.h"
#include "lerosbin3d.h"
#include "lerosfast.h"
#include "lerosfast3d.h"
#include "lerosplan.h"
#include "lfermetrous3d.h"
#include "lfft.h"
#include "lfiltreordre.h"
#include "lfiltrestopo.h"
#include "lfusreg.h"
#include "lga2khalimsky.h"
#include "lgeo.h"
#include "lgeodesic.h"
#include "lgettree.h"
#include "lgradill.h"
#include "lgradinf.h"
#include "lgradinf3d.h"
#include "lgradsup.h"
#include "lhierarchie.h"
#include "lhisto.h"
#include "lhistscal.h"
#include "llut.h"
#include "lhma.h"
#include "lhoughcercles.h"
#include "lhthiniso.h"
#include "lhtkern.h"
#include "lhtkern3d.h"
#include "lisometry.h"
#include "ljones.h"
#include "lkern.h"
#include "llabelextrema.h"
#include "llabelplateaux.h"
#include "llabeltree.h"
#include "llambdakern.h"
#include "llenoir.h"
#include "llevialdi.h"
#include "llinapprox.h"
#include "lline.h"
#include "llocalextrema.h"
#include "llpeGA.h"
#include "llpemeyer.h"
#include "llpemeyer4d.h"
#include "llpetopo.h"
#include "llpetoporeg.h"
#include "lmaxdiameter.h"
#include "lmedialaxis.h"
#include "lmeshwarp.h"
#include "lminima.h"
#include "lmoments.h"
#include "lnbtopo.h"
#include "lnbvois.h"
#include "loffset.h"
#include "lpoint.h"
#include "lppm2GA.h"
#include "lprofil.h"
#include "lpropgeo.h"
#include "lrecalagerigide.h"
#include "lrecalagerigide_translateplane.h"
#include "lrecons.h"
#include "lreconsplateaux.h"
#include "lregul.h"
#include "lremspnoise.h"
#include "lrotate.h"
#include "lrotations.h"
#include "lsaliency.h"
//#include "lsegbi.h"
#include "lsegment.h"
#include "lsegment3d.h"
#include "lsegment3di.h"
#include "lsegmenti.h"
#include "lsegmentlignes.h"
#include "lsegmentnum.h"
#include "lsegmentnumi.h"
#include "lsegreconsheight.h"
#include "lselndg.h"
#include "lselrect.h"
#include "lseltopo.h"
#include "lseuil.h"
#include "lseuili.h"
#include "lskelcurv.h"
#include "lskeletons.h"
#include "lskelpar.h"
#include "lskelpar3d.h"
#include "lskelpar3d_others.h"
#include "lskew.h"
#include "lsquel.h"
#include "lsquelbin.h"
#include "lsquelval.h"
#include "lstat.h"
#include "lstretch.h"
#include "lsym.h"
#include "ltmp1.h"
#include "ltopotypes.h"
//#include "lvoronoi.h"
#include "lwarp.h"
#include "lwshedtopo.h"
#include "lzoom.h"
#include "lzoomint.h"
#include "rgbfileheader.h"

// new header files
// #include "lattrib.h"
#include "lhello.h"
#include "llong2byte.h"
#include "mcrlifo.h"
#include "lmcube.h"
#include "ssexport.h"

// uiFibres header files
#ifdef __cplusplus
// my comment
#  include "uiFibreTypes.h"
#  include "ujimage.hpp"
#  include "uiFrame.hpp"
#  include "uiSqhool.hpp"
#  include "readpnm.h"
#  include "uiBorder.h"
#  include "uiImage.h"
#  include "lbspline.hpp"
#  include "lminmax.hpp"  
#  include "uiNormalize.hpp"
#  include "uiFileIO.hpp"
#  include "uiInsert.hpp"
#  include "uiCutPlane.hpp"
#  include "ui_convert.hpp"
#  include "ui_genball.hpp"
#  include "ui_extractplane.hpp"
#endif /* __cplusplus */

// new with mixed language declarations (c/c++)
#include "larith.h"



#undef min
#undef max
#define max(a,b) error_max_function_is_ambigous use mcmax instead
#define min(a,b) error_min_function_is_ambigous use mcmin instead

#endif
//LuM end of file
