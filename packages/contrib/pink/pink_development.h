/*
  This software is licensed under 
  CeCILL FREE SOFTWARE LICENSE AGREEMENT

  This software comes in hope that it will be useful but 
  without any warranty to the extent permitted by aplicable law.
  
  (C) UjoImro, 2010
  Université Paris-Est, Laboratoire d'Informatique Gaspard-Monge, Equipe A3SI, ESIEE Paris, 93162, Noisy le Grand CEDEX
  ujoimro@gmail.com
*/

// development headers, that are not present in the core version

#ifndef PINK_DEVELOPMENT_H
#define PINK_DEVELOPMENT_H

#ifdef __cplusplus

  #include <boost/accumulators/accumulators.hpp>
  #include <boost/accumulators/statistics.hpp>
  #include <boost/python.hpp>
  #include <boost/python/suite/indexing/vector_indexing_suite.hpp>
  #include <boost/python/module.hpp>
  #include <boost/python/def.hpp>
  #include <boost/python/implicit.hpp>

  #include <eigen2/Eigen/Core>
  #include <eigen2/Eigen/LU>
  #include <eigen2/Eigen/Geometry>

  #include <gsl/gsl_interp.h> // uiGradient is using it
  #include <gsl/gsl_multifit_nlin.h> // uiCircle is using it

#endif /* __cplusplus */

#include <pink.h>

#ifdef __cplusplus

  #include <uiGradient.hpp>
  #include <uiFlow.hpp>
  #include <uiUngerFlow.hpp>
  #include <uiQDistance.hpp>
  #include <uiCircle.hpp>
  #include <ui_fit_circle.hpp>

  #include <pyexport.hpp>
#endif /* __cplusplus */



#endif /* PINK_DEVELOPMENT_H */
//LuM end of file
