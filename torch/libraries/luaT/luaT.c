#include <stdlib.h>
#include <string.h>

#include "luaT.h"

void* luaT_alloc(lua_State *L, long size)
{
  void *ptr;

  if(size == 0)
    return NULL;

  if(size < 0)
    luaL_error(L, "$ Torch: invalid memory size -- maybe an overflow?");

  ptr = malloc(size);
  if(!ptr)
    luaL_error(L, "$ Torch: not enough memory: you tried to allocate %dGB. Buy new RAM!", size/1073741824);

  return ptr;
}

void* luaT_realloc(lua_State *L, void *ptr, long size)
{
  if(!ptr)
    return(luaT_alloc(L, size));
  
  if(size == 0)
  {
    luaT_free(L, ptr);
    return NULL;
  }

  if(size < 0)
    luaL_error(L, "$ Torch: invalid memory size -- maybe an overflow?");
  
  ptr = realloc(ptr, size);
  if(!ptr)
    luaL_error(L, "$ Torch: not enough memory: you tried to reallocate %dGB. Buy new RAM!", size/1073741824);
  return ptr;
}

void luaT_free(lua_State *L, void *ptr)
{
  free(ptr);
}

void luaT_stackdump(lua_State *L)
{
  int i;
  const char *tname;
  int top = lua_gettop(L);
  for(i = 1; i <= top; i++)
  {
    int t = lua_type(L, i);
    printf("%3d. ", i);
    switch(t)
    {
      case LUA_TSTRING:
        printf("'%s'", lua_tostring(L,i));
        break;
      case LUA_TBOOLEAN:
        printf(lua_toboolean(L, i) ? "true" : "false");
        break;
      case LUA_TNUMBER:
        printf("%g", lua_tonumber(L,i));
        break;
      case LUA_TUSERDATA:
        tname = luaT_typename(L, i);
        printf("userdata [%s]", (tname ? tname : "not a Torch object"));
        break;
      case LUA_TTABLE:
        tname = luaT_id2typename(L, lua_topointer(L, i));
        if(tname)
          printf("metaclass [%s]", tname);
        else
        {
          tname = luaT_typename(L, i);
          printf("table [%s]", (tname ? tname : "not a Torch object"));
        }
        break;
      default:
        printf("%s", lua_typename(L,t));
        break;
    }
    printf("\n");
  }
  printf("---------------------------------------------\n");
}

/* Root-metatable methods */
static int luaT_rmt__index(lua_State *L);
static int luaT_rmt__newindex(lua_State *L);
static int luaT_rmt__tostring(lua_State *L);
static int luaT_rmt__add(lua_State *L);
static int luaT_rmt__sub(lua_State *L);
static int luaT_rmt__mul(lua_State *L);
static int luaT_rmt__div(lua_State *L);
static int luaT_rmt__mod(lua_State *L);
static int luaT_rmt__pow(lua_State *L);
static int luaT_rmt__unm(lua_State *L);
static int luaT_rmt__concat(lua_State *L);
static int luaT_rmt__len(lua_State *L);
static int luaT_rmt__eq(lua_State *L);
static int luaT_rmt__lt(lua_State *L);
static int luaT_rmt__le(lua_State *L);
static int luaT_rmt__call(lua_State *L);

/* Constructor-metatable methods */
static int luaT_cmt__call(lua_State *L);
static int luaT_cmt__newindex(lua_State *L);

const void* luaT_newmetatable(lua_State *L, const char *tname, const char *parenttname,
                              lua_CFunction constructor, lua_CFunction destructor, lua_CFunction factory)
{
  lua_pushcfunction(L, luaT_lua_newmetatable);
  lua_pushstring(L, tname);
  (parenttname ? lua_pushstring(L, parenttname) : lua_pushnil(L));
  (constructor ? lua_pushcfunction(L, constructor) : lua_pushnil(L));
  (destructor ? lua_pushcfunction(L, destructor) : lua_pushnil(L));
  (factory ? lua_pushcfunction(L, factory) : lua_pushnil(L));
  lua_call(L, 5, 1);
  return lua_topointer(L, -1);
}

void luaT_pushmetatable(lua_State *L, const void *id)
{
  lua_pushlightuserdata(L, (void*)id);
  lua_rawget(L, LUA_REGISTRYINDEX);
}

void luaT_pushmetaclass(lua_State *L, const void *id)
{
  luaT_pushmetatable(L, id);
  if(!lua_isnil(L, -1))
  {
    if(!lua_getmetatable(L, -1))
      luaL_error(L, "internal error: cannot find metaclass");
    lua_remove(L, -2); /* remove metatable */
  }
}

const char* luaT_id2typename(lua_State *L, const void *id)
{
  const char* tname = NULL;

  lua_getfield(L, LUA_REGISTRYINDEX, "*torch.id2tname*");
  if(lua_isnil(L, -1))
  {
    lua_pop(L, 1);
    return NULL;
  }
  lua_pushlightuserdata(L, (void*)id);
  lua_gettable(L, -2);
  if(!lua_isnil(L, -1))
    tname = lua_tostring(L, -1);
  lua_pop(L, 2);
  return tname; /* still exists, because in a table ... */
}

const void* luaT_typename2id(lua_State *L, const char *tname)
{
  const void *id = NULL;

  lua_getfield(L, LUA_REGISTRYINDEX, "*torch.tname2id*");
  if(lua_isnil(L, -1))
  {
    lua_pop(L, 1);
    return NULL;
  }
  lua_pushstring(L, tname);
  lua_gettable(L, -2);
  if(!lua_isnil(L, -1))
    id = lua_topointer(L, -1);
  lua_pop(L, 2);
  return id; /* id still exists, because in a table ... */
}

const void* luaT_checktypename2id(lua_State *L, const char *tname)
{
  const void* id = luaT_typename2id(L, tname);
  if(!id)
    luaL_error(L, "unknown class <%s>", tname);
  return id;
}

int luaT_getmetaclass(lua_State *L, int index)
{
  if(lua_getmetatable(L, index)) /* get metatable */
  {
    if(lua_getmetatable(L, -1))  /* get metaclass */
    {
      lua_remove(L, -2);
      return 1;
    }
    else
    {
      lua_pop(L, 1);
      return 0;
    }
  }
  return 0;
}

const void* luaT_id(lua_State *L, int ud)
{
  if(luaT_getmetaclass(L, ud))
  {
    const char *id = lua_topointer(L, -1);
    lua_pop(L, 1);
    if(luaT_id2typename(L, id))
      return id;
  }
  return NULL;
}

const char* luaT_typename(lua_State *L, int ud)
{
  if(luaT_getmetaclass(L, ud))
  {
    const char *tname = luaT_id2typename(L, lua_topointer(L, -1));
    lua_pop(L, 1);
    return tname;
  }
  return NULL;
}

void luaT_pushudata(lua_State *L, void *udata, const void *id)
{
  void **udata_p = lua_newuserdata(L, sizeof(void*));  
  *udata_p = udata;
  luaT_pushmetatable(L, id);
  if(lua_isnil(L, -1))
    luaL_error(L, "Torch internal problem: cannot find a metatable");
  lua_setmetatable(L, -2);
}

void *luaT_toudata (lua_State *L, int ud, const void *id)
{
  void **p = lua_touserdata(L, ud);
  if (p != NULL) /* value is a userdata? */
  {
    lua_pushvalue(L, ud); /* initialize the table we want to get the metatable on */
    if(lua_getmetatable(L, -1)) /* get the metatable */
    {
      lua_remove(L, -2); /* remove the original value */
      while(lua_getmetatable(L, -1)) /* get the next metaclass */
      {
        lua_remove(L, -2); /* remove the original metatable/metaclass */
        if(lua_topointer(L, -1) == id)
        {
          lua_pop(L, 1);  /* remove metaclass */
          return *p;
        }
      }
    }
    lua_pop(L, 1); /* remove remaing value/metatable/metaclass */
  }
  return NULL;
}

int luaT_isudata(lua_State *L, int ud, const void *id)
{
  if(luaT_toudata(L, ud, id))
    return 1;
  else
    return 0;
}

void *luaT_checkudata (lua_State *L, int ud, const void *id)
{
  void *p = luaT_toudata(L, ud, id);
  if(!p)
    luaT_typerror(L, ud, luaT_id2typename(L, id));
  return p;
}

void *luaT_getfieldcheckudata (lua_State *L, int ud, const char *field, const void *id)
{
  void *p;
  lua_getfield(L, ud, field);
  if(lua_isnil(L, -1))
    luaL_error(L, "bad argument #%d (field %s does not exist)", ud, field);
  p = luaT_toudata(L, -1, id);
  if(!p)
    luaL_error(L, "bad argument #%d (field %s is not a %s)", ud, field, luaT_id2typename(L, id));
  return p;
}

void *luaT_getfieldchecklightudata (lua_State *L, int ud, const char *field)
{
  void *p;
  lua_getfield(L, ud, field);
  if(lua_isnil(L, -1))
    luaL_error(L, "bad argument #%d (field %s does not exist)", ud, field);

  if(!lua_islightuserdata(L, -1))
    luaL_error(L, "bad argument #%d (field %s is not a light userdata)", ud, field);

  p = lua_touserdata(L, -1);

  return p;
}

double luaT_getfieldchecknumber (lua_State *L, int ud, const char *field)
{
  lua_getfield(L, ud, field);
  if(lua_isnil(L, -1))
    luaL_error(L, "bad argument #%d (field %s does not exist)", ud, field);
  if(!lua_isnumber(L, -1))
    luaL_error(L, "bad argument #%d (field %s is not a number)", ud, field);
  return lua_tonumber(L, -1);
}

int luaT_getfieldcheckint (lua_State *L, int ud, const char *field)
{
  lua_getfield(L, ud, field);
  if(lua_isnil(L, -1))
    luaL_error(L, "bad argument #%d (field %s does not exist)", ud, field);
  if(!lua_isnumber(L, -1))
    luaL_error(L, "bad argument #%d (field %s is not a number)", ud, field);
  return (int)lua_tonumber(L, -1);
}

const char* luaT_getfieldcheckstring (lua_State *L, int ud, const char *field)
{
  lua_getfield(L, ud, field);
  if(lua_isnil(L, -1))
    luaL_error(L, "bad argument #%d (field %s does not exist)", ud, field);
  if(!lua_isstring(L, -1))
    luaL_error(L, "bad argument #%d (field %s is not a string)", ud, field);
  return lua_tostring(L, -1);
}

int luaT_getfieldcheckboolean (lua_State *L, int ud, const char *field)
{
  lua_getfield(L, ud, field);
  if(lua_isnil(L, -1))
    luaL_error(L, "bad argument #%d (field %s does not exist)", ud, field);
  if(!lua_isboolean(L, -1))
    luaL_error(L, "bad argument #%d (field %s is not a boolean)", ud, field);
  return lua_toboolean(L, -1);
}

void luaT_getfieldchecktable (lua_State *L, int ud, const char *field)
{
  lua_getfield(L, ud, field);
  if(lua_isnil(L, -1))
    luaL_error(L, "bad argument #%d (field %s does not exist)", ud, field);
  if(!lua_istable(L, -1))
    luaL_error(L, "bad argument #%d (field %s is not a table)", ud, field);
}

/**** type checks as in luaL ****/
int luaT_typerror(lua_State *L, int narg, const char *tname)
{
  const char *tnamenarg = (lua_istable(L, narg) ? luaT_id2typename(L, lua_topointer(L, narg)) : NULL);
  const char *msg;

  if(tnamenarg)
  {
    msg = lua_pushfstring(L, "%s expected, got %s metatable",
                          tname,
                          (tnamenarg ? tnamenarg : luaL_typename(L, narg)));
  }
  else
  {
    tnamenarg = luaT_typename(L, narg);
    msg = lua_pushfstring(L, "%s expected, got %s",
                          tname,
                          (tnamenarg ? tnamenarg : luaL_typename(L, narg)));
  }
  return luaL_argerror(L, narg, msg);
}

int luaT_checkboolean(lua_State *L, int narg)
{
  if(!lua_isboolean(L, narg))
    luaT_typerror(L, narg, lua_typename(L, LUA_TBOOLEAN));
  return lua_toboolean(L, narg);
}

int luaT_optboolean(lua_State *L, int narg, int def)
{
  if(lua_isnoneornil(L,narg))
    return def;

  return luaT_checkboolean(L, narg);
}


/* utility functions */
const char *luaT_classrootname(const char *tname)
{
  int i;
  int sz = strlen(tname);

  for(i = 0; i < sz; i++)
  {
    if(tname[i] == '.')
      return tname+i+1;
  }
  return tname;
}

static char luaT_class_module_name[256];
const char *luaT_classmodulename(const char *tname)
{
  int i;

  strncpy(luaT_class_module_name, tname, 256);
  
  for(i = 0; i < 256; i++)
  {
    if(luaT_class_module_name[i] == '\0')
      break;
    if(luaT_class_module_name[i] == '.')
    {
      luaT_class_module_name[i] = '\0';
      return luaT_class_module_name;
    }
  }
  return NULL;
}

/* Lua only functions */
int luaT_lua_newmetatable(lua_State *L)
{
  const char* tname = luaL_checkstring(L, 1);
  const void *id;

  lua_settop(L, 5);
  luaL_argcheck(L, lua_isnoneornil(L, 2) || lua_isstring(L, 2), 2, "parent class name or nil expected");
  luaL_argcheck(L, lua_isnoneornil(L, 3) || lua_isfunction(L, 3), 3, "constructor function or nil expected");
  luaL_argcheck(L, lua_isnoneornil(L, 4) || lua_isfunction(L, 4), 4, "destructor function or nil expected");
  luaL_argcheck(L, lua_isnoneornil(L, 5) || lua_isfunction(L, 5), 5, "factory function or nil expected");

  if(luaT_classmodulename(tname))
    lua_getfield(L, LUA_GLOBALSINDEX, luaT_classmodulename(tname));
  else
    lua_pushvalue(L, LUA_GLOBALSINDEX);
  if(!lua_istable(L, 6))
    luaL_error(L, "while creating metatable %s: bad ardument #1 (%s is an invalid module name)", tname, luaT_classmodulename(tname));

  /* we first create the new metaclass if we have to */
  if(!luaT_typename2id(L, tname))
  {
    /* create the metaclass */
    lua_newtable(L);
    id = lua_topointer(L, -1); /* id = pointer on metaclass */

    /* __index points on itself */
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");

    /* __typename contains the typename */
    lua_pushstring(L, tname);
    lua_setfield(L, -2, "__typename");

    /* by default, __version equals 1 */
    lua_pushnumber(L, 1);
    lua_setfield(L, -2, "__version");

    /* register in "*torch.id2tname*" registry table 
       (id -> typename) */
    lua_getfield(L, LUA_REGISTRYINDEX, "*torch.id2tname*");
    if(lua_isnil(L, -1))
    {
      lua_pop(L, 1);
      lua_newtable(L);
      lua_setfield(L, LUA_REGISTRYINDEX, "*torch.id2tname*");
      lua_getfield(L, LUA_REGISTRYINDEX, "*torch.id2tname*");
    }
    lua_pushlightuserdata(L, (void*)id);
    lua_pushstring(L, tname);
    lua_settable(L, -3);
    lua_pop(L, 1);

    /* register in "*torch.tname2id*" registry table 
       (typename -> id) */
    lua_getfield(L, LUA_REGISTRYINDEX, "*torch.tname2id*");
    if(lua_isnil(L, -1))
    {
      lua_pop(L, 1);
      lua_newtable(L);
      lua_setfield(L, LUA_REGISTRYINDEX, "*torch.tname2id*");
      lua_getfield(L, LUA_REGISTRYINDEX, "*torch.tname2id*");
    }
    lua_pushstring(L, tname);
    lua_pushlightuserdata(L, (void*)id);
    lua_settable(L, -3);
    lua_pop(L, 1);
  }

  /* we retrieve the existing metaclass */
  else
  {
    id = luaT_typename2id(L, tname);
    luaT_pushmetaclass(L, id);
  }

  /* we assign the parent class if necessary */
  if(!lua_isnoneornil(L, 2))
  {
    if(lua_getmetatable(L, -1))
      luaL_error(L, "class %s has been already assigned a parent class\n", tname);
    else
    {
      const char* parenttname = luaL_checkstring(L, 2);
      luaT_pushmetaclass(L, luaT_typename2id(L, parenttname));
      if(lua_isnil(L, -1))
        luaL_error(L, "bad argument #2 (invalid parent class name %s)", parenttname);
      lua_setmetatable(L, -2);
    }
  }

  /******** root-metatable ********/

  /* id is the pointer on the metatable
     registry[id] = root-metatable, so try to see if it exists */

  lua_pushlightuserdata(L, (void*)id); /* id */
  lua_rawget(L, LUA_REGISTRYINDEX);

  /* not existing? we create a new one! */
  if(lua_isnil(L, -1))
  {
    lua_pop(L, 1); /* remove nil on stack */
    lua_newtable(L);
    
    /* __index handling */
    lua_pushcfunction(L, luaT_rmt__index);
    lua_setfield(L, -2, "__index");
    
    /* __newindex handling */
    lua_pushcfunction(L, luaT_rmt__newindex);
    lua_setfield(L, -2, "__newindex");

    /* __metatable field (point on the metaclass) */
    lua_pushvalue(L, -2);
    lua_setfield(L, -2, "__metatable");

    /* __typename contains the typename */
    lua_pushstring(L, tname);
    lua_setfield(L, -2, "__typename");
    
    /* operators handling */
#define MT_ADD_OPERATOR(name) \
  lua_pushcfunction(L, luaT_rmt__##name); \
  lua_setfield(L, -2, "__" #name)
    
    MT_ADD_OPERATOR(tostring);
    MT_ADD_OPERATOR(add);
    MT_ADD_OPERATOR(sub);
    MT_ADD_OPERATOR(mul);
    MT_ADD_OPERATOR(div);
    MT_ADD_OPERATOR(mod);
    MT_ADD_OPERATOR(pow);
    MT_ADD_OPERATOR(unm);
    MT_ADD_OPERATOR(concat);
    MT_ADD_OPERATOR(len);
    MT_ADD_OPERATOR(eq);
    MT_ADD_OPERATOR(lt);
    MT_ADD_OPERATOR(le);
    MT_ADD_OPERATOR(call);
    
    /* assign the metaclass as metatable... */
    lua_pushvalue(L, -2);
    lua_setmetatable(L, -2);

    /* id is the pointer on the metatable
       set registry[id] = root-metatable */
    lua_pushlightuserdata(L, (void*)id); /* id */
    lua_pushvalue(L, -2);                /* metatable */
    lua_rawset(L, LUA_REGISTRYINDEX);    /* registry[id] = metatable */

  } /* ok, so now we have the root-metatable on the stack */

  /* register the destructor function  */
  if(!lua_isnoneornil(L, 4))
  {
    /* does it exists already? */
    lua_pushstring(L, "__gc");
    lua_rawget(L, -2);

    if(lua_isnil(L, -1))
    {
      lua_pop(L, 1); /* pop nil */
      lua_pushstring(L, "__gc");
      lua_pushvalue(L, 4);
      lua_rawset(L, -3);
    }
    else
      luaL_error(L, "%s has been already assigned a destructor", tname);
  }

  /* register the factory function  */
  if(!lua_isnoneornil(L, 5))
  {
    /* does it exists already? */
    lua_pushstring(L, "__factory");
    lua_rawget(L, -2);

    if(lua_isnil(L, -1))
    {
      lua_pop(L, 1); /* pop nil */
      lua_pushstring(L, "__factory");
      lua_pushvalue(L, 5);
      lua_rawset(L, -3);
    }
    else
      luaL_error(L, "%s has been already assigned a factory", tname);
  }

  /******** Constructor table and metatable ********/
  lua_pushstring(L, "__constructor");
  lua_rawget(L, -2);

  if(lua_isnil(L, -1))
  {
    lua_pop(L, 1);                        /* pop nil */
    lua_newtable(L);                      /* fancy table */
    lua_newtable(L);                      /* fancy metatable */

    lua_pushvalue(L, -4);                 /* metaclass */
    lua_setfield(L, -2, "__index");       /* so we can get the methods */

    lua_pushcfunction(L, luaT_cmt__newindex);
    lua_setfield(L, -2, "__newindex");    /* so we cannot messup */

    lua_pushcfunction(L, luaT_cmt__call);
    lua_setfield(L, -2, "__call");        /* so we can create */

    lua_pushvalue(L, -4); 
    lua_setfield(L, -2, "__metatable");   /* redirect to metatable with methods */

    lua_setmetatable(L, -2);              /* metatable is ... the fancy metatable */

    /* set root-metatable[__constructor] = constructor-metatable */
    lua_pushstring(L, "__constructor");
    lua_pushvalue(L, -2);
    lua_rawset(L, -4);
  }

  /* register the constructor function  */
  if(!lua_isnoneornil(L, 3))
  {
    /* get constructor metatable */
    lua_getmetatable(L, -1);

    /* does it exists already? */
    lua_pushstring(L, "__new");
    lua_rawget(L, -2);

    if(lua_isnil(L, -1))
    {
      lua_pop(L, 1); /* pop nil */
      lua_pushstring(L, "__new");
      lua_pushvalue(L, 3);
      lua_rawset(L, -3);
    }
    else
      luaL_error(L, "%s has been already assigned a constructor", tname);

    /* pop constructor metatable */
    lua_pop(L, 1);
  }
  
  lua_setfield(L, 6, luaT_classrootname(tname)); /* module.name = constructor-metatable */
  lua_pop(L, 1);                        /* pop the root-metatable */

  return 1; /* returns the metaclass */
}


/* Lua only utility functions */
int luaT_lua_factory(lua_State *L)
{ 
  const char* tname = luaL_checkstring(L, 1);
  luaT_pushmetatable(L, luaT_typename2id(L, tname));
  if(!lua_isnil(L, -1))
  {
    lua_pushstring(L, "__factory");
    lua_rawget(L, -2);
  }
  return 1;
}


int luaT_lua_typename(lua_State *L)
{
  luaL_checkany(L, 1);

  if(luaT_getmetaclass(L, 1))
  {
    const char *tname = luaT_id2typename(L, lua_topointer(L, -1));
    if(tname)
    {
      lua_pushstring(L, tname);
      return 1;
    }
  }
  return 0;
}

int luaT_lua_isequal(lua_State *L)
{
  if(lua_isuserdata(L, 1) && lua_isuserdata(L, 2))
  {
    void **u1, **u2;
    luaL_argcheck(L, luaT_id(L, 1), 1, "Torch object expected");
    luaL_argcheck(L, luaT_id(L, 2), 2, "Torch object expected");

    u1 = lua_touserdata(L, 1);
    u2 = lua_touserdata(L, 2);
    if(*u1 == *u2)
      lua_pushboolean(L, 1);
    else
      lua_pushboolean(L, 0);
  }
  else if(lua_istable(L, 1) && lua_istable(L, 2))
    lua_pushboolean(L, lua_rawequal(L, 1, 2));
  else
    lua_pushboolean(L, 0);
  return 1;
}

int luaT_lua_pointer(lua_State *L)
{
  if(lua_isuserdata(L, 1))
  {
    void **ptr;
    luaL_argcheck(L, luaT_id(L, 1), 1, "Torch object expected");
    ptr = lua_touserdata(L, 1);
    lua_pushnumber(L, (long)(*ptr));
    return 1;
  }
  else if(lua_istable(L, 1) || lua_isthread(L, 1) || lua_isfunction(L, 1))
  {
    const void* ptr = lua_topointer(L, 1);
    lua_pushnumber(L, (long)(ptr));
    return 1;
  }
  else
    luaL_error(L, "Torch object, table, thread or function expected");

  return 0;
}

int luaT_lua_setenv(lua_State *L)
{
  if(!lua_isfunction(L, 1) && !lua_isuserdata(L, 1))
    luaL_typerror(L, 1, "function or userdata");
  luaL_checktype(L, 2, LUA_TTABLE);
  lua_setfenv(L, 1);
  return 0;
}

int luaT_lua_getenv(lua_State *L)
{
  if(!lua_isfunction(L, 1) && !lua_isuserdata(L, 1))
    luaL_typerror(L, 1, "function or userdata");
  lua_getfenv(L, 1);
  return 1;
}

int luaT_lua_getmetatable(lua_State *L)
{
  const char *tname = luaL_checkstring(L, 1);
  luaT_pushmetaclass(L, luaT_typename2id(L, tname)); /* note: in Lua, root-metatable is hidden, so... you get it eh... */
  return 1;
}

int luaT_lua_version(lua_State *L)
{
  luaL_checkany(L, 1);

  if(luaT_getmetaclass(L, 1))
  {
    lua_pushstring(L, "__version");
    lua_rawget(L, -2);
    return 1;
  }
  return 0;
}

int luaT_lua_setmetatable(lua_State *L)
{
  const char *tname = luaL_checkstring(L, 2);
  luaL_checktype(L, 1, LUA_TTABLE);

  lua_pushvalue(L, 1);
  luaT_pushmetatable(L, luaT_typename2id(L, tname));
  if(lua_isnil(L, -1))
    luaL_error(L, "unknown typename %s\n", tname);
  lua_setmetatable(L, -2);
  return 1;
}

/* root-metatable functions */
static int luaT_rmt__index(lua_State *L)
{
  if(!luaT_getmetaclass(L, 1))
    luaL_error(L, "critical internal indexing error: no metatable found");

  if(!lua_istable(L, -1))
    luaL_error(L, "critical internal indexing error: not a metatable");

  /* test for __index__ method first */
  lua_getfield(L, -1, "__index__");
  if(!lua_isnil(L, -1))
  {
    int result;

    if(!lua_isfunction(L, -1))
      luaL_error(L, "critical internal indexing error: __index__ is not a function");

    lua_pushvalue(L, 1);
    lua_pushvalue(L, 2);

    lua_call(L, 2, LUA_MULTRET); /* DEBUG: risque: faut vraiment retourner 1 ou 2 valeurs... */

    result = lua_toboolean(L, -1);
    lua_pop(L, 1);

    if(result)
      return 1;

    /* on the stack: 1. the object 2. the value 3. the metatable */
    /* apparently, __index wants only one element returned */
    /* return lua_gettop(L)-3; */

  }
  else
    lua_pop(L, 1); /* remove nil __index__ on the stack */

  lua_pushvalue(L, 2);
  lua_gettable(L, -2);

  return 1;
}

static int luaT_rmt__newindex(lua_State *L)
{
  if(!luaT_getmetaclass(L, 1))
    luaL_error(L, "critical internal indexing error: no metatable found");

  if(!lua_istable(L, -1))
    luaL_error(L, "critical internal indexing error: not a metatable");

  /* test for __newindex__ method first */
  lua_getfield(L, -1, "__newindex__");
  if(!lua_isnil(L, -1))
  {  
    int result;

    if(!lua_isfunction(L, -1))
      luaL_error(L, "critical internal indexing error: __newindex__ is not a function");

    lua_pushvalue(L, 1);
    lua_pushvalue(L, 2);
    lua_pushvalue(L, 3);

    lua_call(L, 3, 1); /* DEBUG: risque: faut vraiment retourner qqch */

    result = lua_toboolean(L, -1);
    lua_pop(L, 1);

    if(result)
      return 0;
  }
  else
    lua_pop(L, 1); /* remove nil __newindex__ on the stack */

  lua_pop(L, 1);    /* pop the metaclass */
  if(lua_istable(L, 1))
    lua_rawset(L, 1);
  else
    luaL_error(L, "the class %s cannot be indexed", luaT_typename(L, 1));

  return 0;
}

/* note: check dans metatable pour ca, donc necessaire */
#define MT_DECLARE_OPERATOR(NAME, NIL_BEHAVIOR) \
int luaT_rmt__##NAME(lua_State *L) \
{ \
  if(!lua_getmetatable(L, 1)) \
    luaL_error(L, "internal error in __" #NAME ": no metatable"); \
\
  if(!lua_getmetatable(L, -1)) \
    luaL_error(L, "internal error in __" #NAME ": no metaclass"); \
\
  lua_getfield(L, -1, "__" #NAME "__"); \
\
  if(lua_isnil(L, -1)) \
  { \
    NIL_BEHAVIOR; \
  } \
  else \
  { \
    if(lua_isfunction(L, -1)) \
    { \
      lua_insert(L, 1); /* insert function */ \
      lua_pop(L, 2); /* remove metatable and metaclass */ \
      lua_call(L, lua_gettop(L)-1, 1); /* we return the result of the call */ \
    } \
    /* we return the thing the user left in __tostring__ */ \
  } \
  return 1; \
}

MT_DECLARE_OPERATOR(tostring, lua_pushstring(L, luaT_typename(L, 1)))
MT_DECLARE_OPERATOR(add, luaL_error(L, "%s has no addition operator", luaT_typename(L, 1)))
MT_DECLARE_OPERATOR(sub, luaL_error(L, "%s has no substraction operator", luaT_typename(L, 1)))
MT_DECLARE_OPERATOR(mul, luaL_error(L, "%s has no multiplication operator", luaT_typename(L, 1)))
MT_DECLARE_OPERATOR(div, luaL_error(L, "%s has no division operator", luaT_typename(L, 1)))
MT_DECLARE_OPERATOR(mod, luaL_error(L, "%s has no modulo operator", luaT_typename(L, 1)))
MT_DECLARE_OPERATOR(pow, luaL_error(L, "%s has no power operator", luaT_typename(L, 1)))
MT_DECLARE_OPERATOR(unm, luaL_error(L, "%s has no negation operator", luaT_typename(L, 1)))
MT_DECLARE_OPERATOR(concat, luaL_error(L, "%s has no concat operator", luaT_typename(L, 1)))
MT_DECLARE_OPERATOR(len, luaL_error(L, "%s has no length operator", luaT_typename(L, 1)))
MT_DECLARE_OPERATOR(eq,
                    lua_settop(L, 2);
                    lua_pushcfunction(L, luaT_lua_isequal);
                    lua_insert(L, 1);
                    lua_call(L, 2, 1);)
MT_DECLARE_OPERATOR(lt, luaL_error(L, "%s has no lower than operator", luaT_typename(L, 1)))
MT_DECLARE_OPERATOR(le, luaL_error(L, "%s has no lower or equal than operator", luaT_typename(L, 1)))
MT_DECLARE_OPERATOR(call, luaL_error(L, "%s has no call operator", luaT_typename(L, 1)))

/* constructor metatable methods */
int luaT_cmt__call(lua_State *L)
{
  if(!lua_istable(L, 1))
    luaL_error(L, "internal error in __call: not a constructor table");

  if(!lua_getmetatable(L, 1))
    luaL_error(L, "internal error in __call: no metatable available");

  lua_pushstring(L, "__new");
  lua_rawget(L, -2);

  if(lua_isnil(L, -1))
    luaL_error(L, "no constructor available");

  lua_remove(L, 1); /* remove root metatable */
  lua_insert(L, 1); /* insert constructor */
  lua_pop(L, 1);    /* remove fancy metatable */

  lua_call(L, lua_gettop(L)-1, 1);
  return 1;
}

int luaT_cmt__newindex(lua_State *L)
{
  luaL_error(L, "constructor tables are read-only");
  return 0;
}

