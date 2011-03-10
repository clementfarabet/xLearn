/* $Id: bdd2.beta.c,v 1.2 2009-01-06 13:18:06 mcouprie Exp $ */
#include <bdd.h>

int bddBeta3SimpleCarreX(int a, int b)
{
int neg = 1;
if(a)
goto l0_2;
neg = !neg; goto l0_2;
l0_2:	if(b)
goto ret1;
goto ret0;
ret0: return neg; 
ret1: return !neg; 
}
int bddBeta3SimpleCarreY(int a, int b)
{
int neg = 1;
if(a)
goto l0_2;
neg = !neg; goto l0_2;
l0_2:	if(b)
goto ret1;
goto ret0;
ret0: return neg; 
ret1: return !neg; 
}
int bddBeta3SimpleCarreZ(int a, int b)
{
int neg = 1;
if(a)
goto l0_2;
neg = !neg; goto l0_2;
l0_2:	if(b)
goto ret1;
goto ret0;
ret0: return neg; 
ret1: return !neg; 
}
int bddBeta3SimpleInterX(int a, int b, int c, int d, int e, int f, int g, int h)
{
int neg = 1;
if(g)
goto l5_2;
neg = !neg; goto l13_2;
l5_2:	if(f)
goto l3_3;
neg = !neg; goto l7_3;
l3_3:	if(e)
goto l3_4;
neg = !neg; goto l2_6;
l3_4:	if(c)
goto l1_5;
neg = !neg; goto l4_5;
l1_5:	if(a)
goto l0_6;
neg = !neg; goto l2_6;
l0_6:	if(h)
goto l0_7;
goto ret0;
l0_7:	if(d)
goto l0_8;
neg = !neg; goto l0_8;
l0_8:	if(b)
goto ret1;
goto ret0;
l2_6:	if(h)
goto l2_7;
goto ret0;
l2_7:	if(d)
goto l0_8;
goto ret0;
l4_5:	if(a)
goto l2_6;
goto ret0;
l7_3:	if(e)
goto l6_4;
neg = !neg; goto l8_4;
l6_4:	if(a)
goto l6_5;
goto ret0;
l6_5:	if(h)
goto l6_6;
neg = !neg; goto l0_8;
l6_6:	if(d)
goto ret1;
goto ret0;
l8_4:	if(c)
goto ret1;
neg = !neg; goto l6_5;
l13_2:	if(f)
goto l11_3;
neg = !neg; goto l15_3;
l11_3:	if(e)
goto l10_4;
neg = !neg; goto l12_4;
l10_4:	if(c)
goto l10_5;
goto ret0;
l10_5:	if(h)
goto l0_8;
neg = !neg; goto l6_6;
l12_4:	if(a)
goto ret1;
neg = !neg; goto l10_5;
l15_3:	if(e)
goto l14_6;
goto l15_4;
l14_6:	if(h)
goto ret1;
goto l14_7;
l14_7:	if(d)
goto ret1;
goto l0_8;
l15_4:	if(c)
goto l14_5;
goto l16_5;
l14_5:	if(a)
goto ret1;
goto l14_6;
l16_5:	if(a)
goto l14_6;
neg = !neg; goto l16_6;
l16_6:	if(h)
goto ret1;
neg = !neg; goto l0_7;
ret0: return neg; 
ret1: return !neg; 
}
int bddBeta3SimpleInterY(int a, int b, int c, int d, int e, int f, int g, int h)
{
int neg = 1;
if(g)
goto l5_2;
neg = !neg; goto l13_2;
l5_2:	if(f)
goto l3_3;
neg = !neg; goto l7_3;
l3_3:	if(e)
goto l3_4;
neg = !neg; goto l2_6;
l3_4:	if(c)
goto l1_5;
neg = !neg; goto l4_5;
l1_5:	if(a)
goto l0_6;
neg = !neg; goto l2_6;
l0_6:	if(h)
goto l0_7;
goto ret0;
l0_7:	if(d)
goto l0_8;
neg = !neg; goto l0_8;
l0_8:	if(b)
goto ret1;
goto ret0;
l2_6:	if(h)
goto l2_7;
goto ret0;
l2_7:	if(d)
goto l0_8;
goto ret0;
l4_5:	if(a)
goto l2_6;
goto ret0;
l7_3:	if(e)
goto l6_4;
neg = !neg; goto l8_4;
l6_4:	if(c)
goto l6_5;
goto ret0;
l6_5:	if(h)
goto l0_8;
neg = !neg; goto l6_6;
l6_6:	if(d)
goto ret1;
goto ret0;
l8_4:	if(a)
goto ret1;
neg = !neg; goto l6_5;
l13_2:	if(f)
goto l11_3;
neg = !neg; goto l15_3;
l11_3:	if(e)
goto l10_4;
neg = !neg; goto l12_4;
l10_4:	if(a)
goto l10_5;
goto ret0;
l10_5:	if(h)
goto l6_6;
neg = !neg; goto l0_8;
l12_4:	if(c)
goto ret1;
neg = !neg; goto l10_5;
l15_3:	if(e)
goto l14_6;
goto l15_4;
l14_6:	if(h)
goto ret1;
goto l14_7;
l14_7:	if(d)
goto ret1;
goto l0_8;
l15_4:	if(c)
goto l14_5;
goto l16_5;
l14_5:	if(a)
goto ret1;
goto l14_6;
l16_5:	if(a)
goto l14_6;
neg = !neg; goto l16_6;
l16_6:	if(h)
goto ret1;
neg = !neg; goto l0_7;
ret0: return neg; 
ret1: return !neg; 
}
int bddBeta3SimpleInterZ(int a, int b, int c, int d, int e, int f, int g, int h)
{
int neg = 1;
if(g)
goto l5_2;
neg = !neg; goto l13_2;
l5_2:	if(f)
goto l3_3;
neg = !neg; goto l7_3;
l3_3:	if(e)
goto l3_4;
neg = !neg; goto l2_6;
l3_4:	if(c)
goto l1_5;
neg = !neg; goto l4_5;
l1_5:	if(a)
goto l0_6;
neg = !neg; goto l2_6;
l0_6:	if(h)
goto l0_7;
goto ret0;
l0_7:	if(d)
goto l0_8;
neg = !neg; goto l0_8;
l0_8:	if(b)
goto ret1;
goto ret0;
l2_6:	if(h)
goto l2_7;
goto ret0;
l2_7:	if(d)
goto l0_8;
goto ret0;
l4_5:	if(a)
goto l2_6;
goto ret0;
l7_3:	if(e)
goto l6_4;
neg = !neg; goto l8_4;
l6_4:	if(c)
goto l6_5;
goto ret0;
l6_5:	if(h)
goto l0_8;
neg = !neg; goto l6_6;
l6_6:	if(d)
goto ret1;
goto ret0;
l8_4:	if(a)
goto ret1;
neg = !neg; goto l6_5;
l13_2:	if(f)
goto l11_3;
neg = !neg; goto l15_3;
l11_3:	if(e)
goto l10_4;
neg = !neg; goto l12_4;
l10_4:	if(a)
goto l10_5;
goto ret0;
l10_5:	if(h)
goto l6_6;
neg = !neg; goto l0_8;
l12_4:	if(c)
goto ret1;
neg = !neg; goto l10_5;
l15_3:	if(e)
goto l14_6;
goto l15_4;
l14_6:	if(h)
goto ret1;
goto l14_7;
l14_7:	if(d)
goto ret1;
goto l0_8;
l15_4:	if(c)
goto l14_5;
goto l16_5;
l14_5:	if(a)
goto ret1;
goto l14_6;
l16_5:	if(a)
goto l14_6;
neg = !neg; goto l16_6;
l16_6:	if(h)
goto ret1;
neg = !neg; goto l0_7;
ret0: return neg; 
ret1: return !neg; 
}
