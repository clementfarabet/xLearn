//==============================================================================
// File: idx
//
// Description: A wrapper for eblearn/libidx
//
// Created: February 10, 2011, 11:57AM
//
// Author: Clement Farabet // clement.farabet@gmail.com
//==============================================================================

extern "C" {
#include <luaT.h>
#include <TH.h>
}

#include <dirent.h>
#include <errno.h>
#include <time.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>

#include <libidx.h>
#include <libeblearn.h>
#include <libeblearntools.h>

//============================================================
// To load this lib in LUA:
// require 'libidx'

//============================================================
// Conversion functions from Tensors to IDXs
//
static int l_tensor2idx_double (lua_State *L) {
  // get args
  THTensor *source = (THTensor *)luaT_checkudata(L, 1, luaT_checktypename2id(L, "torch.Tensor"));

  // create storage that points to tensor's data
  ebl::srg<double> *storage = new ebl::srg<double>();
  storage->data = THTensor_dataPtr(source);
  storage->changesize(THTensor_nElement(source));

  // invert dimensions
  long *sdims = (long *)source->size;
  long *smods = (long *)source->stride;
  long dims[] = {-1,-1,-1,-1,-1,-1,-1,-1};
  long mods[] = {-1,-1,-1,-1,-1,-1,-1,-1};
  for (int i=0; i<source->nDimension; i++) {
    dims[i] = sdims[source->nDimension-i-1];
    mods[i] = smods[source->nDimension-i-1];
  }

  // create IDX
  ebl::idx<double> *dest = new ebl::idx<double>(storage, 0, 
                                                source->nDimension, 
                                                dims, mods);

  // push reference
  lua_pushnumber(L, (long)dest);
  return 1;
}

static int l_idx2tensor_double (lua_State *L) {
  // get args
  ebl::idx<double> *source = (ebl::idx<double> *)(long)lua_tonumber(L, 1);

  // create storage that points to IDX's data
  long nelements = source->getstorage()->size();
  THDoubleStorage *storage = THDoubleStorage_newWithSize(nelements);
  memcpy(storage->data, source->getstorage()->data, sizeof(double)*nelements);

  // invert dimensions
  long *sdims = (long *)source->dims();
  long *smods = (long *)source->mods();
  long dims[] = {-1,-1,-1,-1,-1,-1,-1,-1};
  long mods[] = {-1,-1,-1,-1,-1,-1,-1,-1};
  for (int i=0; i<source->order(); i++) {
    dims[i] = sdims[source->order()-i-1];
    mods[i] = smods[source->order()-i-1];
  }

  // create tensor that points to IDX's data
  THTensor *dest = THTensor_newWithStorage(storage, 0, source->order(),
                                           dims, mods);

  // push reference
  luaT_pushudata(L, dest, luaT_checktypename2id(L, "torch.Tensor"));
  return 1;
}

//============================================================
// IDX basic routines
//
static int l_delete_double (lua_State *L) {
  // get args
  ebl::idx<double> *source = (ebl::idx<double> *)(long)lua_tonumber(L, 1);

  // delete
  delete source;

  // done
  return 0;
}

//============================================================
// IDX I/O
//
static int l_load_double (lua_State *L) {
  // get args
  const char *filename = lua_tostring(L, 1);

  // load
  ebl::idx<double> loaded = ebl::load_matrix<double>(filename);

  // ref
  ebl::idx<double> *ref = new ebl::idx<double>(loaded);

  // done
  lua_pushnumber(L, (long)ref);
  return 1;
}

static int l_save_double (lua_State *L) {
  // get args
  const char *filename = lua_tostring(L, 1);
  ebl::idx<double> *tosave = (ebl::idx<double> *)(long)lua_tonumber(L, 2);

  // load
  bool done = ebl::save_matrix<double>(*tosave, filename);

  // done
  lua_pushboolean(L, done);
  return 1;
}

//============================================================
// EBLearn network loader
//
static int l_load_network (lua_State *L) {
  // get args
  const char *filename = luaL_checkstring(L, 1);
  // load config
  try {
    ebl::configuration conf(filename);
    string dir = ebl::dirname(filename);
    cout << "Looking for trained files in: " << dir << endl;
    conf.set("root2", dir.c_str());
    conf.resolve();
    printf("DIR = %s\n", dir.c_str());
    // create network
    ebl::idx<ebl::ubyte> classes(1,1);
    ebl::load_matrix<ebl::ubyte>(classes, conf.get_cstring("classes"));
    uint noutputs = conf.exists_true("binary_target") ? 1 : classes.dim(0); 
    ebl::parameter< double, ebl::fstate_idx<double> > theparam;
    ebl::module_1_1< double, ebl::fstate_idx<double> > *net = 
      ebl::create_network< double, ebl::fstate_idx<double> >(theparam, conf, noutputs);
    // loading weights
    theparam.load_x(conf.get_cstring("weights"));
    // return net
    lua_pushlightuserdata(L,(void *)net);
    return 1;
  } catch (ebl::eblexception &e) {
    THError(e.c_str());
    return 0;
  }
}

//============================================================
// Register functions in LUA
//
static const struct luaL_reg leblearn [] = {
  {"tensor2idx_double", l_tensor2idx_double},
  {"idx2tensor_double", l_idx2tensor_double},
  {"delete_double", l_delete_double},
  {"load_double", l_load_double},
  {"save_double", l_save_double},
  {"load_network", l_load_network},
  {NULL, NULL}  /* sentinel */
};

extern "C" {
  int luaopen_libleblearn (lua_State *L) {
    luaL_openlib(L, "libleblearn", leblearn, 0);
    return 1; 
  }
}
