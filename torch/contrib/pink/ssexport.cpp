/*
  This software is licensed under 
  CeCILL FREE SOFTWARE LICENSE AGREEMENT

  This software comes in hope that it will be useful but 
  without any warranty to the extent permitted by aplicable law.
  
  (C) UjoImro, 2010
  Université Paris-Est, Laboratoire d'Informatique Gaspard-Monge, Equipe A3SI, ESIEE Paris, 93162, Noisy le Grand CEDEX
  ujoimro@gmail.com
*/

// THIS IS A HACK
// These functions are only used in pink::python
// if you compile this file with the classical makefile, it compiles dummy functions

#ifdef __cplusplus

#include <sstream>
using std::stringstream;


#undef error
#define error(msg) {stringstream fullmessage; fullmessage << "in ssexport.cpp: " << msg; call_error(fullmessage.str());}

// NO NAMESPACE, this is a hack
// namespace pink { 


extern "C" {
#endif /* __cplusplus */


void ui_mesh_export_cstring( void * ss, char * data )
{
  #ifdef __cplusplus
  (*reinterpret_cast<stringstream *>(ss)) << data;
  #endif
} /* ui_mesh_export_cstring */


void ui_mesh_export_int    ( void * ss, int data )
{
  #ifdef __cplusplus
  (*reinterpret_cast<stringstream *>(ss)) << data;
  #endif
} /* ui_mesh_export_int */


void ui_mesh_export_double ( void * ss, double data )
{
  #ifdef __cplusplus
  (*reinterpret_cast<stringstream *>(ss)) << data;
  #endif
} /* ui_mesh_export_double */


#ifdef __cplusplus
} /* extern "C" */
#endif





















//  } /* namespace pink */


/* LuM end of file */
