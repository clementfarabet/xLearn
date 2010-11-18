#include "lcairo_defines.h"
#include "threads.h"
#include "cairo.h"
#include "lua.h"

#ifdef _LCAIRO_WIN32
#else 
 #include <X11/Xlib.h>
 #include <X11/Xutil.h>
#endif

enum tMsg_id{
  TMSG_NULL = 0,
  TMSG_CLOSED,
  TMSG_RESIZE,
  TMSG_MOUSE_MOVE,
  TMSG_MOUSE_BUTTON1_PRESS,
  TMSG_MOUSE_BUTTON1_RELEASE,
  TMSG_MOUSE_BUTTON2_PRESS,
  TMSG_MOUSE_BUTTON2_RELEASE,
  TMSG_KEYDOWN,
  TMSG_KEYUP,
  TMSG_PAINT /* why do we send out paint? this is as a paint is happening! */
};


typedef struct _tMsg {
    int id;/* tMsg_id id; */
    int width;
    int height;
    int x;
    int y;
    int key_id;/*	 key; */
} tMsg;


typedef struct tWindow tWindow;

#define TMSG_BUFFER_SIZE 100

struct tWindow {

 char *name;

 int valid;
 int width;
 int height;
 int min_width;
 int min_height;

 cond_t   thread_init_cond;
 mutex_t  mutex;

 cairo_surface_t* surf;
 int surf_width;
 int surf_height;
/* unsigned char*   surf_data; */

 int  num_messages;
 tMsg messages[TMSG_BUFFER_SIZE];

#ifdef _LCAIRO_WIN32
 HWND   hwnd;
 HANDLE thread;
#else
 pthread_t thread;
 mutex_t   thread_init_mutex;
 Window window;
 Display* display;
 Atom atom_delwin_proto;
 Atom atom_delwin;
 cairo_surface_t* ps;
#endif

};


void  window_init();

tWindow*  tWindow_create( lua_State* L, int width, int height, const char *name );
void      tWindow_message( tWindow *window, tMsg msg );
tMsg      tWindow_message_next( tWindow *window );
void tWindow_refresh( tWindow* p );
void tWindow_create_thread(tWindow *p);
void tWindow_set_surface( tWindow *p, cairo_surface_t *surf );

void tWindow_destroy(tWindow* p);
