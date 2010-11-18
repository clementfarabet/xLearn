#include "utils.h"

static const void* torch_LongStorage_id = NULL;

THLongStorage* lab_checklongargs(lua_State *L, int index)
{
  THLongStorage *storage;
  int i;
  int narg = lua_gettop(L)-index+1;

  if(narg == 1 && luaT_toudata(L, index, torch_LongStorage_id))
  {
    THLongStorage *storagesrc = luaT_toudata(L, index, torch_LongStorage_id);
    storage = THLongStorage_newWithSize(storagesrc->size);
    THLongStorage_copy(storage, storagesrc);
  }
  else
  {
    storage = THLongStorage_newWithSize(narg);
    for(i = index; i < index+narg; i++)
    {
      if(!lua_isnumber(L, i))
      {
        THLongStorage_free(storage);
        luaL_argerror(L, i, "number expected");
      }
      storage->data[i-index] = lua_tonumber(L, i);
    }
  }
  return storage;
}

void lab_utils_init(lua_State *L)
{
  torch_LongStorage_id = luaT_checktypename2id(L, "torch.LongStorage");
}
