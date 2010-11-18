static const void* Kernel_id;

struct Kernel
{
    
};

int Kernel_new(lua_State *L)
{
  Kernel *kernel = luaT_alloc(sizeof(Kernel));
  kernel->eval = NULL;
  luaT_pushudata(L, kernel, Kernel_id);
}

int Kernel_eval(lua_State *L)
{
  Kernel *kernel = luaT_checkudata(L, 1, Kernel_id);
  
}
