#include "window.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "keytable.h"
/* #include <cairo.h> */
/* #include <cairo-win32.h> */

#include "luaT.h"

int vb=0;

tWindow*  tWindow_create(lua_State *L, int width, int height, const char *name )
{  
  tWindow *p = (tWindow*)luaT_alloc(L, sizeof(tWindow));

  if(vb) printf("window create..width,height:%d,%d\n",width,height);

  p->name    = (char *)  luaT_alloc(L, strlen(name)+1);
  strcpy(p->name, name );

  p->width  	= width;
  p->height  	= height;
  p->min_width  = 0;
  p->min_height = 0;
  p->min_height = 0;
  p->num_messages = 0;

  p->surf=NULL;
  p->surf_width = 0;
  p->surf_height = 0;

  tWindow_create_thread(p);

  return p;
}

void tWindow_set_surface( tWindow *p, cairo_surface_t *surf ){
  p->surf=surf;
  p->surf_width=cairo_image_surface_get_width(surf); 
  p->surf_height=cairo_image_surface_get_height(surf); 
  if (vb) printf("set_display_surface, w,h:%d,%d\n",p->surf_width,p->surf_height);
  tWindow_refresh(p);
}

void tWindow_message( tWindow *p, tMsg msg ){

 if (p->num_messages>=TMSG_BUFFER_SIZE){
   /* remove old messages */
   memcpy(&p->messages[0],&p->messages[1],sizeof(tMsg)*p->num_messages-1);
   p->num_messages--;
 }
 memcpy(&p->messages[p->num_messages],&msg,sizeof(tMsg));
 p->num_messages++;
}

tMsg tWindow_message_next( tWindow *p ){
 tMsg m;
 m.id = TMSG_NULL; 
 if (p->num_messages>0){
   memcpy(&m,             &p->messages[0],sizeof(tMsg));
   memcpy(&p->messages[0],&p->messages[1],sizeof(tMsg)*p->num_messages-1);
   p->num_messages--;
 }
 return m;
}

