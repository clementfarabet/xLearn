/***********************************************************
 * what: toolBox and imageBox compiled code.
 * why:  any function that needs to be compiled should be 
 *       here.
 ***********************************************************/
#include "luaT.h"
#include <TH.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/time.h>
#include <string.h>
//#include <ncurses.h>

int image_saturate(lua_State *L) {
  THTensor *input = luaT_checkudata(L, 1, 
                                    luaT_checktypename2id(L, "torch.Tensor"));
  THTensor *output = input;

  TH_TENSOR_APPLY2(double, output, double, input, \
                     *output_p = (*input_p < 0) ? 0 : (*input_p > 1) ? 1 : *input_p;)
    
  return 1;
}

int image_threshold(lua_State *L) {
  THTensor *input = luaT_checkudata(L, 1, 
                                    luaT_checktypename2id(L, "torch.Tensor"));
  double thres = 0.5;
  double low = 0.0;
  double mid = 0.5;
  double high = 1.0;
  THTensor *output = input;

  if (lua_isnumber(L,2)) thres = lua_tonumber(L, 2);
  if (lua_isnumber(L,3)) low = lua_tonumber(L, 3);
  if (lua_isnumber(L,4)) mid = lua_tonumber(L, 4);
  if (lua_isnumber(L,5)) high = lua_tonumber(L, 5);

  TH_TENSOR_APPLY2(double, output, double, input, \
                     *output_p = (*input_p < -thres) ? low : (*input_p > thres) ? high : mid;)

  return 1;
}

int image_lower(lua_State *L) {
  THTensor *input = luaT_checkudata(L, 1, 
                                    luaT_checktypename2id(L, "torch.Tensor"));
  double thres = 0.5;
  THTensor *output = input;

  if (lua_isnumber(L,2)) thres = lua_tonumber(L, 2);

  TH_TENSOR_APPLY2(double, output, double, input, \
                     *output_p = (*input_p-thres < 0) ? 0 : *input_p-thres;)

  return 1;
}

int toolbox_createTable(lua_State *L) {
  int arr_size = 0;
  int rec_size = 0;
  if (lua_isnumber(L,1)) arr_size = lua_tonumber(L,1);
  if (lua_isnumber(L,2)) rec_size = lua_tonumber(L,2);
  lua_createtable(L, arr_size, rec_size);
  return 1;
}

int toolbox_usleep(lua_State *L) {
  int time = 1;
  if (lua_isnumber(L, 1)) time = lua_tonumber(L, 1);
  usleep(time);
  return 1;
}

int toolbox_fillFloatTensor(lua_State *L) {
  // tensor to fill:
  THFloatTensor *tofill = luaT_checkudata(L, 1, luaT_checktypename2id(L, "torch.FloatTensor"));
  // source storage:
  THByteStorage *toget = luaT_checkudata(L, 2, luaT_checktypename2id(L, "torch.ByteStorage"));
  // optional pointer:
  int pointer = 0;
  if (lua_isnumber(L, 3)) pointer = lua_tonumber(L, 3) - 1;
  // char pointers:
  unsigned char *src = (unsigned char *)toget->data;
  unsigned char *dest = (unsigned char *)tofill->storage->data;
  long int size = tofill->storage->size * 4;
  // fill:
  long int i;
  memcpy(dest, src+pointer, size);
  return 0;
}

int toolbox_fillByteTensor(lua_State *L) {
  // tensor to fill:
  THByteTensor *tofill = luaT_checkudata(L, 1, luaT_checktypename2id(L, "torch.ByteTensor"));
  // source storage:
  THByteStorage *toget = luaT_checkudata(L, 2, luaT_checktypename2id(L, "torch.ByteStorage"));
  // optional pointer:
  int pointer = 0;
  if (lua_isnumber(L, 3)) pointer = lua_tonumber(L, 3) - 1;
  // char pointers:
  unsigned char *src = (unsigned char *)toget->data;
  unsigned char *dest = (unsigned char *)tofill->storage->data;
  long int size = tofill->storage->size;
  // fill:
  long int i;
  memcpy(dest, src+pointer, size);
  return 0;
}

int toolbox_fillTensor(lua_State *L) {
  // tensor to fill:
  THTensor *tofill = luaT_checkudata(L, 1, luaT_checktypename2id(L, "torch.Tensor"));
  // source storage:
  THByteStorage *toget = luaT_checkudata(L, 2, luaT_checktypename2id(L, "torch.ByteStorage"));
  // optional pointer:
  int pointer = 0;
  if (lua_isnumber(L, 3)) pointer = lua_tonumber(L, 3) - 1;
  // char pointers:
  unsigned char *src = (unsigned char *)toget->data;
  unsigned char *dest = (unsigned char *)tofill->storage->data;
  long int size = tofill->storage->size * 8;
  // fill:
  long int i;
  memcpy(dest, src+pointer, size);
  return 0;
}

int toolbox_getMicroTime(lua_State *L) {
  struct timeval tv;
  struct timezone tz;
  struct tm *tm;
  gettimeofday(&tv, &tz);
  tm=localtime(&tv.tv_sec);
  double precise_time = tm->tm_hour*3600 + tm->tm_min*60 + tm->tm_sec + tv.tv_usec / 1000000.0;
/*   printf(" %d:%02d:%02d %d \n", tm->tm_hour, tm->tm_min,  */
/*          tm->tm_sec, tv.tv_usec); */
  lua_pushnumber(L,precise_time);
  return 1;
}

/*
int toolbox_ncurseStart(lua_State *L) {
  initscr();
  noecho();
  raw();
  keypad(stdscr, true);
  if(has_colors() == FALSE){	
    endwin();
    printf("Your terminal does not support color\n");
    exit(1);
  }
  return 0;
}

int toolbox_ncurseEnd(lua_State *L) {
  endwin();
  return 0;
}

int toolbox_ncursePrint(lua_State *L) {
  int x = 0, y = 0;
  const char * toprint = luaL_checkstring(L, 1);
  if (lua_isnumber(L, 2)) y = lua_tonumber(L, 2);
  if (lua_isnumber(L, 3)) x = lua_tonumber(L, 3);
  mvprintw(y,x,toprint);
  return 0;
}

int toolbox_ncurseRefresh(lua_State *L) {
  refresh();
  return 0;
}

int toolbox_ncurseGetDims(lua_State *L) {
  int width, height;
  getmaxyx(stdscr,height,width);
  lua_pushnumber(L,height);
  lua_pushnumber(L,width);
  return 2;
}

int toolbox_ncurseGetChar(lua_State *L) {
  char a = getch();
  lua_pushlstring(L,&a,1);
  return 1;
}
*/
