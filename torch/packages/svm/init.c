#include "luaT.h"

extern void svm_QPSolver_init(lua_State *L);
extern void svm_Kernel_init(lua_State *L);
extern void svm_SparseKernel_init(lua_State *L);
extern void svm_SVM_init(lua_State *L);

DLL_EXPORT int luaopen_libsvm(lua_State *L)
{
  lua_newtable(L);
  lua_pushvalue(L, -1);
  lua_setfield(L, LUA_GLOBALSINDEX, "svm");

  svm_QPSolver_init(L);
  svm_Kernel_init(L);
  svm_SparseKernel_init(L);
  svm_SVM_init(L);

  return 1;
}
