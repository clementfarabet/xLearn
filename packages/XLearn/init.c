#include "luaT.h"
#include <TH.h>

extern void nn_AbsModule_init(lua_State *L);
extern void nn_AbsModuleHessian_init(lua_State *L);
extern void nn_SpatialConvolutionTable_init(lua_State *L);
extern void nn_SpatialConvolutionTableHessian_init(lua_State *L);
extern void nn_SpatialMaxPooling_init(lua_State *L);
extern void nn_SpatialSubSamplingHessian_init(lua_State *L);
extern void nn_SpatialReSampling_init(lua_State *L);
extern void nn_TanhHessian_init(lua_State *L);
extern void nn_LcEncoder_init(lua_State *L);
extern void nn_LcDecoder_init(lua_State *L);
extern void nn_Threshold_init(lua_State *L);
extern void nn_SparseCriterion_init(lua_State *L);

extern int image_saturate(lua_State *L);
extern int image_threshold(lua_State *L);
extern int image_lower(lua_State *L);
extern int image_maskToRGB(lua_State *L);
extern int image_rgb2hsl(lua_State *L);
extern int image_hsl2rgb(lua_State *L);
extern int image_rgb2hsv(lua_State *L);
extern int image_hsv2rgb(lua_State *L);
extern int image_mergeVectorsIntoSegm(lua_State *L);

extern int toolbox_usleep(lua_State *L);
extern int toolbox_fillTensor(lua_State *L);
extern int toolbox_fillFloatTensor(lua_State *L);
extern int toolbox_fillByteTensor(lua_State *L);
extern int toolbox_createTable(lua_State *L);
extern int toolbox_getMicroTime(lua_State *L);
extern int toolbox_spatialDist(lua_State *L);
extern int toolbox_dist2vectors(lua_State *L);
extern int toolbox_convFixedPoint(lua_State *L);
extern int toolbox_mapFixedPoint(lua_State *L);
extern int toolbox_get_k_local_maxima(lua_State *L);
extern int toolbox_print_tensor_formatted(lua_State *L);
extern int toolbox_fstat_time(lua_State *L);
/*
extern int toolbox_ncursePrint(lua_State *L);
extern int toolbox_ncurseStart(lua_State *L);
extern int toolbox_ncurseEnd(lua_State *L);
extern int toolbox_ncurseGetChar(lua_State *L);
extern int toolbox_ncurseGetDims(lua_State *L);
extern int toolbox_ncurseRefresh(lua_State *L);
*/

// Extra routines
static const struct luaL_reg extraroutines [] = {
  {"saturate", image_saturate},
  {"threshold", image_threshold},
  {"lower", image_lower},
  {"getMicroTime", toolbox_getMicroTime},
  {"fillTensor", toolbox_fillTensor},
  {"fillByteTensor", toolbox_fillByteTensor},
  {"fillFloatTensor", toolbox_fillFloatTensor},
  {"usleep", toolbox_usleep},
  {"fstat_time", toolbox_fstat_time},
  {"spatialDist", toolbox_spatialDist},
  {"createTable", toolbox_createTable},
  {"Dist2",toolbox_dist2vectors},
  {"convFixedPoint",toolbox_convFixedPoint},
  {"mapFixedPoint",toolbox_mapFixedPoint},
  {"get_k_local_maxima",toolbox_get_k_local_maxima},
  {"print_tensor_formatted",toolbox_print_tensor_formatted},
  {"image_maskToRGB", image_maskToRGB},
  {"mergeVectorsIntoSegm", image_mergeVectorsIntoSegm},
  {"rgb2hsl", image_rgb2hsl},
  {"hsl2rgb", image_hsl2rgb},
  {"rgb2hsv", image_rgb2hsv},
  {"hsv2rgb", image_hsv2rgb},
  /*
  {"ncurseStart", toolbox_ncurseStart},
  {"ncurseEnd", toolbox_ncurseEnd},
  {"ncursePrint", toolbox_ncursePrint},
  {"ncurseGetChar", toolbox_ncurseGetChar},
  {"ncurseGetDims", toolbox_ncurseGetDims},
  {"ncurseRefresh", toolbox_ncurseRefresh},
  */
  {NULL, NULL}  /* sentinel */
};

DLL_EXPORT int luaopen_libXLearn(lua_State *L)
{
  // has to be part of nn
  lua_getfield(L, LUA_GLOBALSINDEX, "nn");
  if(lua_isnil(L, -1))
    luaL_error(L, "nn module required");
  lua_pushvalue(L, -1);

  nn_AbsModule_init(L);
  nn_AbsModuleHessian_init(L);
  nn_SpatialConvolutionTable_init(L);
  nn_SpatialConvolutionTableHessian_init(L);
  nn_SpatialMaxPooling_init(L);
  nn_SpatialSubSamplingHessian_init(L);
  nn_SpatialReSampling_init(L);
  nn_TanhHessian_init(L);
  nn_LcEncoder_init(L);
  nn_LcDecoder_init(L);
  nn_SparseCriterion_init(L);
  nn_Threshold_init(L);
  luaL_openlib(L, "libxlearn", extraroutines, 0);

  return 1;
}
