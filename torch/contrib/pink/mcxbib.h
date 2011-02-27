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
#ifndef _STDINT_H
#include <stdint.h>
#endif

#include <X11/Xlib.h>
#include <X11/Xutil.h>

#define MC_LEFTBUTTON     1
#define MC_MIDDLEBUTTON   2
#define MC_RIGHTBUTTON    3

#define MC_BLACK     0
#define MC_RED       64
#define MC_GREEN     65
#define MC_BLUE      66
#define MC_YELLOW    67
#define MC_CYAN      68
#define MC_MAGENTA   69
#define MC_ORANGE    70
#define MC_PURPLE    71
#define MC_BROWN     72
#define MC_PINK      73
#define MC_LIGHTGREY 74

#ifdef PCMAISON
#define BLACK     0
#define WHITE     1
#define LIGHTGREY 2
#define DARKGREY  4
#define RED       21
#define BLUE      23
#define GREEN     6
#endif

#ifdef PCTRAVAIL
#define BLACK     0
#define WHITE     1
#define RED       9
#define GREEN     8
#endif

#ifdef HP
#define BLACK     0
#define WHITE     1
#define RED       3
#define GREEN     8
#define LIGHTGREY 20
#endif

#ifdef HP
#define space     50

#define left  106
#define right 115
#define up    108
#define down  105

#define Left      116
#define Right     125
#define Up        126
#define Down      123
#define LeftUp    117
#define LeftDown  114
#define RightUp   134
#define RightDown 131

#define plus      133
#define moins     141
#define num0      121
#define num1      114
#define num2      123
#define num3      131
#define num4      116
#define num5      124
#define num6      125
#define num7      117
#define num8      126
#define num9      134

#define key_a     37
#define key_b     59
#define key_c     42
#define key_d     44
#define key_e     45
#define key_f     52
#define key_g     61
#define key_h     60
#define key_i     76
#define key_j     68
#define key_k     75
#define key_l     84
#define key_m     67
#define key_n     58
#define key_o     77
#define key_p     86
#define key_q     30
#define key_r     54
#define key_s     36
#define key_t     53
#define key_u     69
#define key_v     51
#define key_w     38
#define key_x     43
#define key_y     62
#define key_z     35
#endif

#ifdef PC
#define space     65

#define left  100
#define right 102
#define up    98
#define down  104

#define Left      83
#define Right     85
#define Up        80
#define Down      88
#define LeftUp    79
#define LeftDown  87
#define RightUp   81
#define RightDown 89

#define plus      21
#define moins     15

#define num0      90
#define num1      87
#define num2      88
#define num3      89
#define num4      83
#define num5      84
#define num6      85
#define num7      79
#define num8      80
#define num9      81

#define key_a     24
#define key_b     56
#define key_c     54
#define key_e     26
#define key_i     31
#define key_l     46
#define key_m     47
#define key_n     57
#define key_p     33
#define key_q     38
#define key_r     27
#define key_s     39
#define key_u     30
#define key_v     55
#define key_w     52
#define key_x     53
#define key_y     29
#define key_z     25
#endif

#define BORDER_WIDTH 2

#define FONTE10 "-adobe-courier-bold-r-normal--10-100-75-75-m-60-iso8859-1"
#define FONTE8 "-adobe-courier-bold-r-normal--8-80-75-75-m-50-iso8859-1"
#define FONTE12 "-adobe-courier-bold-r-normal--12-120-75-75-m-70-iso8859-1"
#define FONTE14 "-adobe-courier-bold-r-normal--14-140-75-75-m-90-iso8859-1"
#define FONTE18 "-adobe-courier-bold-r-normal--18-180-75-75-m-110-iso8859-1"
#define FONTE24 "-adobe-courier-bold-r-normal--24-240-75-75-m-150-iso8859-1"

/* ============== */
/* prototypes for mcxbib.c    */
/* ============== */

extern void ColToWhite(
);

extern void ColToBlack(
);

extern void Col(
  uint32_t c
);

extern void Color(
  uint32_t c
);

extern void Line(
  int32_t x1, 
  int32_t y1, 
  int32_t x2, 
  int32_t y2
);

extern void Point(
  int32_t x, 
  int32_t y 
);

extern void FRectangle(
  int32_t x, 
  int32_t y, 
  int32_t w, 
  int32_t h
);

extern void Rectangle(
  int32_t x, 
  int32_t y, 
  int32_t w, 
  int32_t h
);

extern void SetFont(
  char *fontName
);

extern void String(
  int32_t x, 
  int32_t y, 
  char *str
);

extern void InitGraphics(
  int32_t x, 
  int32_t y, 
  int32_t w, 
  int32_t h
);

extern void InitColorGraphics(
  int32_t x, 
  int32_t y, 
  int32_t w, 
  int32_t h
);

extern void WaitMouseEvent(
  int32_t *x, 
  int32_t *y
);

int32_t WaitKbdEvent(
);

int32_t WaitEvent(
  int32_t *x, 
  int32_t *y
);

int32_t WaitAnyEvent(
  int32_t *x, 
  int32_t *y,
  int32_t *c
);

extern void FlushGraphics(
);

extern void TerminateGraphics(
);

#ifdef __cplusplus
}
#endif
