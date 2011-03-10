//---------------------------------------------------------------------------
// Code written by Lilan Buzer - http://www.esiee.fr/~buzerl/DG/

#ifndef lbdigitalline_h
#define lbdigitalline_h

#include <stdlib.h>
#include <stdint.h>

//---------------------------------------------------------------------------
// mini math kernel
//---------------------------------------------------------------------------
             
typedef int32_t Z;

//int abs(int u)        { if ( u >= 0 ) return u; else  return -u; }
static int max(int a, int b) { if ( a > b  ) return a; else  return  b; }
static int min(int a, int b) { if ( a < b  ) return a; else  return  b; }

class Q
{
  public :
  Z n;
  Z d;
  Q(int a, int b) { n = a; d = b; }
  Q(int a) { n = a; d = 1; }
  Q() {};
  Q(const Q & q)  { n = q.n; d = q.d; }
  Q & operator = (const Q & q) { n = q.n; d = q.d; return *this; }
};


static bool operator  >  (const Q & a, const Q & b) { return ( a.n*b.d >  b.n * a.d ); }
static bool operator  <  (const Q & a, const Q & b) { return ( a.n*b.d <  b.n * a.d ); }
static bool operator  >= (const Q & a, const Q & b) { return ( a.n*b.d >= b.n * a.d ); }
static bool operator  <= (const Q & a, const Q & b) { return ( a.n*b.d <= b.n * a.d ); }
static bool operator  == (const Q & a, const Q & b) { return ( a.n*b.d == b.n * a.d ); }
static bool operator  != (const Q & a, const Q & b) { return ( a.n*b.d != b.n * a.d ); }

class point
{
  public :
  Z x;
  Z y;

  point(int X, int Y) : x(X), y(Y) {}
  point() {}
  point(const point & p) : x(p.x), y(p.y) {}
  point & operator = (const point & p) { x = p.x; y=p.y; return *this; }
  void prime()
  {
    int n = (abs(x)<abs(y)?abs(x):abs(y));  // reduction gcd=1  // to replace
    for (int d = 2 ; d <= n ; d++)
    while ( (x%d==0) && (y%d==0) )  {x /= d; y /= d; }
  }
};

typedef point vector;

typedef point * Lpoint;

static bool operator == (const point & a,const point & b)
{
  return (a.x == b.x) && (a.y == b.y);
}

static point operator - (const point & a,const point & b)
{
  point r;
  r.x = a.x-b.x;
  r.y = a.y-b.y;
  return r;
}

static point operator + (const point & a,const point & b)
{
  point r;
  r.x = a.x+b.x;
  r.y = a.y+b.y;
  return r;
}

// vector product

static int operator ^ (const point & a,const point & b)
{
  return a.x*b.y - a.y*b.x;
}

// dot product

static int operator * (const point & a,const point & b)
{
  return a.x*b.x+a.y*b.y;
}

//---------------------------------------------------------------------------
// digital line recognition
//--------------------------------------------------------------------------- 

#define MAXPOINT 1000

class Reco
{
  protected :
  
  Q Ep(int i_u,int i_l);
  point U[MAXPOINT];
  point L[MAXPOINT];
  int nU,nL;
  int iU,iL;
  Z lastX;

  void InsertPoint(Q & curEp, point & NP);
  void Rebuild(Lpoint LL, int & n,int miny,int maxy);
  bool back;

  public :
    
  Reco();
  void Init();          // new sequence of recognition
  void Normal(int & a, int & b);
  void Tangent(int & a, int & b);
  void Tangent(int type, int & a, int & b);

  // we prefer an horizontal segment of 3 pixels and thickness 1 instead of
  // a vertical segment of thickness 3

  bool valid();
  virtual point Sym(point &P) = 0;
  void Insert(Q & curEp, point &NP);
  void GiveP(Lpoint LL, int & n,int miny,int maxy);
};

class RecoLR : public Reco { public : point Sym(point &P) { return point( P.x, P.y); } };
class RecoRL : public Reco { public : point Sym(point &P) { return point(-P.x, P.y); } };
class RecoBT : public Reco { public : point Sym(point &P) { return point( P.y, P.x); } };
class RecoTB : public Reco { public : point Sym(point &P) { return point(-P.y,-P.x); } };

#endif
