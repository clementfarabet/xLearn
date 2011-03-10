/*
  This software is licensed under 
  CeCILL FREE SOFTWARE LICENSE AGREEMENT

  This software comes in hope that it will be useful but 
  without any warranty to the extent permitted by aplicable law.
  
  (C) UjoImro, 2007-2010
  Université Paris-Est, Laboratoire d'Informatique Gaspard-Monge, Equipe A3SI, ESIEE Paris, 93162, Noisy le Grand CEDEX
  ujoimro@gmail.com
*/

#ifdef __cplusplus
#ifndef UIFIBRETYPES_H_
#define UIFIBRETYPES_H_
/** \file uiFibreTypes.h
 * \brief Header file for uiFibreTypes.cpp
 * 
 * Declaration of special types. The purpose of the type declaration is the universality. I want the program to be size- and dimension independent.
 */

/* Line 12+1 */
/* 
uiFibreTypes.h
declaration of special types. The purpose of the type 
declaration is the universality. I want the program to 
be size-independent
*/
#define PTR boost::shared_ptr
#define ARRAY boost::shared_array
using boost::lexical_cast;
using std::string;
using std::cout;
using std::stringstream;
using std::endl;
using std::vector;
using std::fstream;
using std::pair;
using std::ofstream;
using std::ios_base;
using std::ifstream;
using std::stringstream;





#ifdef UJIMAGE_DEBUG
  #define DEBUG(x) cout << "debug: " << BOOST_PP_STRINGIZE(x) << " = " << x << "\n"
  #define PRINTIMAGE(x)							\
    cout << "writing image: " << BOOST_PP_STRINGIZE(x) << " as '" << BOOST_PP_STRINGIZE(x) << ".pgm'\n"; \
    x->_writeimage(BOOST_PP_STRINGIZE(x)".pgm")

#else
  #define DEBUG(x)
  #define PRINTIMAGE(x)
#endif /* UJIMAGE_DEBUG */

#define _DEBUG(x) cout << "_debug: " << BOOST_PP_STRINGIZE(x) << " = " << x << "\n"

#define _PRINTIMAGE(x)							\
 cout << "_writing image: " << BOOST_PP_STRINGIZE(x) << " as '" << BOOST_PP_STRINGIZE(x) << ".pgm'\n"; \
 x._writeimage(BOOST_PP_STRINGIZE(x)".pgm")

#define _PRINTAMIRA(x)							\
 cout << "_writing image: " << BOOST_PP_STRINGIZE(x) << " as '" << BOOST_PP_STRINGIZE(x) << ".am'\n"; \
 x._write_amira(BOOST_PP_STRINGIZE(x)".am")

// eigen2 debug facility
#define DVECT(x) cout << "----------------\n" << BOOST_PP_STRINGIZE(x) << " = \n" << x << "\n"

 // this macro generates from 'IMAGE_TYPE(potencial)' -> 'image_type(potencial), "potencial"' 
#define IMAGE_TYPE(x)				\
 image_type( x, BOOST_PP_STRINGIZE(x) )

// Limit of a blob for sqlite. A little under 900MB
#define BLOB_SIZE 900000000

#define OPEN_BLOB_RO 0
#define OPEN_BLOB_RW 1

// Sentinel constant
#define PBAR_REPORT_INTERVALL 1

// General Macros
#define FOR(i,st) for (int i = 0; i<=st-1; i++)
#define FORR(i,st) for (i = 0; i<=st-1; i++)

#define ClassDef(Type)                          \
  class T##Type;                                \
  typedef T##Type * P##Type

#define StructDef(Type)                         \
  typedef struct T##Type;                       \
  typedef T##Type * P##Type

#define EPSILON 0.01

#define round(z) ((z-(double)((int)z))<0.5?((int)z):((int)z+1))

typedef char *   pchar;
typedef char ** ppchar;

typedef sqlite3 * psqlite3;

typedef sqlite3_blob *   psqlite3_blob;
typedef sqlite3_blob ** ppsqlite3_blob;

typedef sqlite3_stmt *   psqlite3_stmt;
typedef sqlite3_stmt ** ppsqlite3_stmt;

//typedef float uiVal_type; /* !!! Still assumed to be a simple type e.g. automatically copyes the value*/
#define uiVal_type float
//#define uiVal_type double


// The coresponding image for maxflow calculations
#define UVT_image float_image
//#define UVT_image double_image


template <class pixel_type> 
inline pixel_type _max( pixel_type a, pixel_type b )
{
  return a > b ? a : b;
} /* _max */

template <class pixel_type> 
inline pixel_type _min( pixel_type a, pixel_type b )
{
  return a < b ? a : b;
} /* _min */



class vval_type: public vector<uiVal_type> {
public:
  vval_type();

  vval_type( const vval_type & src ); //copy constructor
  vval_type( int size );
  vval_type( int size, int defvalue );
  ~vval_type();
  void normate(); //sets it up the same direction but 1. length.
};

class vint: public vector<int> {
#ifdef UJIMAGE_DEBUG
private:
  string debug;
#endif /* UJIMAGE_DEBUG */

public:
  vint();
  vint( const vint & src ); //copy constructor
  vint(const vint & src, string debug); // copy constructor with debugging
  vint( int size, string debug="" );
  vint( int size, int defvalue );
  vint( const boost::python::list & src );
  ~vint();
  void reset();
  int prod() const;
  int prodExcept( int p ) const;
  uiVal_type fabs() const;
  string repr() const;
  void nextStep( int step, vint & result ) const;
  bool on_side( const vint & point ) const;
  bool inside( const vint & ) const;
  //bool inside( const boost::python::list & point ) const;
  bool nextStep_it( vint & result ) const;
  bool operator==( const vint & other ) const;
  bool operator!=( const vint & other ) const;
  int position( const vint & elem ) const;
  bool addSet( const vint & other );
  vint & operator << ( const int & initiator );
  vint & operator,   ( const int & next );
};

template <class im_type>
class uiVector{
public:
  ARRAY<im_type> values;
  int length;
  uiVector(int length);
  uiVector(const vector<im_type> & src); // copy constructor
  ~uiVector();
};



template <class im_type>
uiVector<im_type>::uiVector(int len){
  values.reset(new im_type[len]);
};



template <class im_type>
uiVector<im_type>::~uiVector(){
//  free(values); //will be freed automaticly by boost:shared_array
  length=0; // setting 0 for no confusion at errorneus read
};



template <class im_type>
uiVector<im_type>::uiVector(const vector<im_type> & src){
  length = src.size();
  values.reset(new im_type[length]);
  FOR(q, length)
    values[q]=src[q];
};


typedef struct {
  int start;
  int end;
} uiDibble;

// jump is for reducing the number of allocation in the dibbling. about to be tuned
#define jump 10
// maxdibble is the size allocation for the constrain. It should never depass L1-data-10%
// on core2 duo it is about 7000 for v + g + 3 * dimension * f
#define MaxDibble 500
// maximal dimension supposed in compile-time
#define compileDim 4

//not yet used
#define MinDibble 100

class progressBar {
private:
  int max, min, pos;
  bool measure;
  bool started;
  time_t begin, finish;
  time_t last_report;
  string time2string(time_t seconds);
//  start_time
public:
  progressBar();
  ~progressBar();
  bool timeToReport();
  void start();
  void stop();
  void maxPos(int maxPos);
  void minPos(int minPos);
  void setPos(int currPos);
  string operator << (int currPos);
  string percent();
  string elapsedTime();
  
  string remainingTime();
  
};

class uiDibbles {
private:
  int size;
  int length;
public:
  ARRAY<uiDibble> values;
  uiDibbles(int n);
  uiDibbles();
  ~uiDibbles();
  void addElement(int start, int end);
  int get_length();
};

void call_error(const string message);

uiVal_type uiAbs( uiVal_type x );

//uiVal_type vAbs(vector<uiVal_type> & v);
//void vTimes(vector<uiVal_type> & v, uiVal_type x); //// it multiplies every element of the vector
//int vProdExcept(vint * v, int p); //// gives the product of all but p-th element

uiVal_type uiSqr( uiVal_type x );
//void vReset(vint & v);

//int vProd(const vint & v);

/* class uiScalarField{ */
/* public: */
/*   vint size; */
/*   PTR< uiVector<uiVal_type> > values; */
/* private: */
/*   int position ( const vint & elem ); */
/*   uiVal_type r( int a, int b, int c, uiVal_type x, uiVal_type y, uiVal_type z); */
/*   uiVal_type r( int a, int b, uiVal_type x, uiVal_type y ); */
/* public: */
/*   uiScalarField( PTR<vint> pdim ); */
/*   uiScalarField( const vint & dim ); */
/*   uiScalarField( const vint & dim, ARRAY<float> data ); */
/*   uiScalarField( const vint & dim, ARRAY<unsigned char> data ); */
/*   uiScalarField( const vint & dim, uiScalarField & src, int xmin, int ymin );////just 2D */
/*   ~uiScalarField( void ); */
/*   //void clear(void); */
/*   void copy( uiScalarField & src ); */
/*   void copy( uiScalarField & src, vint & coord ); //// just 2D */
/*   vint dimensionVect( void ); */
/* //  uiVal_type & operator [] ( vint * n ); */
/*   uiVal_type & operator [] ( const vint & n ); */
/*   uiVal_type & operator [] ( const PTR<vint> & n ); */
/*   uiVal_type & operator [] ( int n ); */
/*   uiVal_type max( void ); */
/*   uiVal_type min( void ); */
/*   void add( uiVal_type v ); */
/*   void _sqrt( void ); */
/*   void measure( void ); /\* should be parametrized later*\/ */
/*   void circles( void ); /\* Should be parametrized later*\/ */
/*   void circles(int x, int y, int z); */
/*   void circles(int x, int y, int z, int a, int b, int c);//min distance from two points */
/*   void circles(int x, int y ); */
/*   void times( uiVal_type q ); // multiplies the field with a constant */
/*   void div( uiScalarField & image ); // divides the image with an other image pixelvise */
/*   void setDefaultSink( void );		 */
/*   vint giveSrcPoint( void ); */
/*   void setSrc( uiScalarField & image ); */
/*   void setSink( uiScalarField & image ); */
/*   ARRAY<unsigned char> getCharRepresentation(); // I use pointers here because sqlite3, the pointers must be deleted after */
/* //  ARRAY<unsigned char> getCharRepresentation( const vint & pos, const vint & size ); */
/*   ARRAY<float> getFloatRepresentation(); // I use pointers here because sqlite3, the pointers must be deleted after */
/* //  ARRAY<float> getFloatRepresentation( const vint & pos, const vint & size ); */
/*   void apply( uiScalarField & src, const vint & pos);   */
/* }; */

/* typedef PTR<uiScalarField> puiScalarField; */































































#endif /*UIFIBRETYPES_H_*/
#endif /*__cplusplus*/
/* LuM end of file */
