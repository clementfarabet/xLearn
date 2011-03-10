/*
  This software is licensed under 
  CeCILL FREE SOFTWARE LICENSE AGREEMENT

  This software comes in hope that it will be useful but 
  without any warranty to the extent permitted by aplicable law.
  
  (C) UjoImro, 2010
  Université Paris-Est, Laboratoire d'Informatique Gaspard-Monge, Equipe A3SI, ESIEE Paris, 93162, Noisy le Grand CEDEX
  ujoimro@gmail.com
*/

#ifndef UI_SSEXPORT_H_
#define UI_SSEXPORT_H_

#undef error
#define error(msg) {stringstream fullmessage; fullmessage << "in ssexport.h: " << msg; call_error(fullmessage.str());}

/// THIS IS A HACK!!!

#ifdef __cplusplus
extern "C" {
#endif

  void ui_mesh_export_cstring( void * /*stringstream*/, char * );
  void ui_mesh_export_int    ( void * /*stringstream*/, int    );
  void ui_mesh_export_double ( void * /*stringstream*/, double );
  
#ifdef __cplusplus
} /* extern "C" */
#endif
#endif /*UI_SSEXPORT_H_*/
/* LuM end of file */
