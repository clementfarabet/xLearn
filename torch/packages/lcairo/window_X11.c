#include "lcairo_defines.h"

#ifndef _LCAIRO_WIN32
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <signal.h>
#include <memory.h>
#include <unistd.h>
#include <errno.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <cairo.h>
#include <cairo-xlib.h>
#include "window.h"

extern int vb;

void *WindowThreadFunc(void *userdata);

int xerror_handler( Display *display, XErrorEvent *err ){
	char buf[256];
	XGetErrorText(display,err->error_code,buf,254);
	fprintf(stderr,"Xerror: %s\n",buf);
	return 0;
}

void window_init(){
  XInitThreads();
  XSetErrorHandler( xerror_handler );
}

void tWindow_create_thread(tWindow *p){
  pthread_attr_t* att=NULL;

  mutex_create(&p->mutex);
  mutex_create(&p->thread_init_mutex);
  condition_create(&p->thread_init_cond);
  mutex_lock(&p->thread_init_mutex);

  if(vb) printf("window create beginning thread\n");
  if (vb) printf("tWindow create thread %p %p \n", (void*)p, (void*)WindowThreadFunc);
  if ((pthread_create(&p->thread, att, WindowThreadFunc, (void*)p ))!=0){
 	fprintf(stderr,"pthread create error\n");
	exit(1);
  }
  if (vb) printf("tWindow create thread called\n");

  condition_wait(&p->thread_init_cond,&p->thread_init_mutex);
  mutex_destroy(&p->thread_init_mutex);
  if(vb) printf("window create init cond return\n");
  condition_destroy(&p->thread_init_cond);
}

void tWindow_destroy( tWindow* p ){
  if (vb) printf("window destroy:%p\n",(void*)p);
  if (p->valid){
    XSync(p->display,True);
    XDestroyWindow(p->display, p->window ); /* sends DestroyNotify */
  }
  pthread_join(p->thread ,NULL);
  if (vb) printf("rejoined thread\n");
}

void _refresh( tWindow* p){
  Display *d;
  Window   w;
  cairo_surface_t *ps;
  cairo_t *cr;
  float margx, margy, x1, x2, x3, x4, y1, y2, y3, y4;

  if (!p->valid) return; /* might be good for windoze */
  if (vb) printf("window _refresh:%p\n",(void*)p);

  mutex_lock(&p->mutex);
  if (vb) printf("window _refresh got lock\n");

  d = p->display;
  w = p->window;

  /* and blit to X */
  ps  = cairo_xlib_surface_create(d,
          w, 
          DefaultVisual(d,DefaultScreen(d)),
          p->width,
          p->height);
  if (ps==NULL){
    fprintf(stderr, "%s: can't open cairo xlib surface\n",XDisplayName(NULL));
    exit(1);
  }

  cr = cairo_create(ps);

  /* centred blit */
  margx = (p->width  - p->surf_width)/2;
  margy = (p->height - p->surf_height)/2;
  x1=0; x2=margx; x3=p->width-margx; x4=p->width;
  y1=0; y2=margy; y3=p->height-margy; y4=p->height;
  cairo_set_source_rgba(cr,0.5,0.5,0.6,1.0);
  cairo_rectangle(cr, x1,y1,x2,y4); /* left  */
  cairo_rectangle(cr, x3,y1,x4,y4); /* right */
  cairo_rectangle(cr, x2,y1,x3,y2); /* middle upper */
  cairo_rectangle(cr, x2,y3,x3,y4); /* middle lower */
  cairo_fill(cr);

  /* doesn't like translate of 0.5 here.. */
  if (vb) printf("marge:%g,%g\n",margx,margy);
  cairo_translate(cr,(int)margx,(int)margy);

  cairo_set_source_surface(cr,p->surf,0,0);
  cairo_paint(cr);
  cairo_destroy(cr);
  cairo_surface_flush(ps);
  cairo_surface_flush(p->surf);
  cairo_surface_destroy(ps);
  
  XFlush(p->display);    /* was XSync(p->display,True) */
  
  mutex_unlock(&p->mutex);
}
void tWindow_refresh( tWindow* p ){
  _refresh(p);
  usleep(10); /* give the normal events some time in tight loops */
}


/* returns bool for  quit */
Bool handle_events(Display *display, tWindow* p ){

  Bool expose=False;
  Bool motion=False;
  Bool quit=False;
  int motion_x = 0;
  int motion_y = 0;
  int new_width=p->width;
  int new_height=p->height;

  XEvent event;
  while (XPending(display)>0 && quit==False){
    XNextEvent(display, &event);

    if(vb) printf("window:%p event:%d\n",(void*)event.xany.window, event.type);

    switch(event.type){
      case Expose:
      {
        /* if (debugX11) printf("X11 Expose\n"); */
        expose=True;
        /* if (debugX11) printf("X11 marking for Expose\n"); */
      }
      break;
      case ConfigureNotify:
      {
        XConfigureEvent *configure_event = (XConfigureEvent*)&event;
        if (vb) printf("X11 Configure Notify: %p\n",(void*)p);
        new_width  = configure_event->width;
        new_height = configure_event->height;
      }
      break;
      case ClientMessage: 
      {
        if(vb) printf("X11 ClientMessage\n");
        if (    event.xclient.message_type   == p->atom_delwin_proto && 
             (Atom)event.xclient.data.l[0] == p->atom_delwin ) {
          if(vb)printf("XDestroyWindow:%p\n",(void*)p);
          quit=True;
        }
      }
      break;
      case MotionNotify:
      {
        XMotionEvent* motion_event = (XMotionEvent*)&event;
        motion_x = motion_event->x;
        motion_y = motion_event->y;
        motion=True;
        /*  if(debugX11)printf("X11 Motion event\n"); */
      }
      break;
      case ButtonPress:
      {
        XButtonEvent* button_event = (XButtonEvent*)&event;
        tMsg m;
        m.id    = TMSG_MOUSE_BUTTON1_PRESS;
        m.x     = button_event->x;
        m.y     = button_event->y;
        tWindow_message(p,m);
        if(vb)printf("X11 Button Press\n");
      }
      break;
      case ButtonRelease:
      {
        XButtonEvent* button_event = (XButtonEvent*)&event;
        tMsg m;
        m.id  = TMSG_MOUSE_BUTTON1_RELEASE;
        m.x     = button_event->x;
        m.y     = button_event->y;
        tWindow_message(p,m);
        if(vb)printf("X11 Button Release\n");
      }
      break;
      case DestroyNotify: 
      {
        if(vb)printf("X11 Destroy\n");
        quit=True;  
      }
      break;
    };/* switch event.type */
  } /* while events */

  if (p->height!=new_height || p->width!=new_width){
    tMsg m;
    m.id   = TMSG_RESIZE;
    m.width   = new_width;
    m.height   = new_height;
    tWindow_message(p,m);

    p->width = new_width;
    p->height = new_height;
    expose=True;
  }
  
  if (motion){
    tMsg m;
    m.id   = TMSG_MOUSE_MOVE;
    m.x     = motion_x;
    m.y     = motion_y;
    tWindow_message(p,m);
  }

  if(expose){
    tMsg m;
    if (vb) printf("X11 doing Expose\n");
    m.id   = TMSG_PAINT;
    tWindow_message(p,m);

    _refresh(p);
  }/* expose */

  if (quit){
    tMsg m;
    m.id   = TMSG_CLOSED;
    tWindow_message(p,m);

    return False; 
  }

  return True; /* S_OK */
}

void *WindowThreadFunc(void *userdata){

  tWindow *p = (tWindow*)userdata;
  int screen, startx, starty;
  unsigned long win_mask;
  XSetWindowAttributes win_attrib;
  XSizeHints  sizehints;
  Display* ptemp;

  /* should probably pass these from main.. */
  char **argv=NULL; int argc=0;

  if(vb)printf("ThreadFunc\n");

  if ((p->display = XOpenDisplay(NULL)) == NULL) {
    fprintf(stderr, "warning: can't XOpenDisplay %s\n",XDisplayName(NULL));
    return NULL;
  }
  if (vb) printf("XDisplay:%p\n",(void*)p->display);

  screen = DefaultScreen(p->display);
  startx = 10;
  starty = 20;

  win_mask = CWBackPixel | CWBorderPixel;
  win_attrib.border_pixel   =WhitePixel(p->display,screen);
  win_attrib.background_pixel   =WhitePixel(p->display,screen); /* BlackPixel */
  win_attrib.override_redirect   = 0;

  if(vb)printf("creating XWindow..\n");

  p->window = XCreateWindow(  p->display, 
          DefaultRootWindow(p->display),
          startx,starty,
          p->width,
          p->height,
                              0,  /* borderwidth */
          DefaultDepth(p->display,screen),
          InputOutput,
          CopyFromParent,
          win_mask,
          &win_attrib );


  /* tell the window manager NOT to quit on delete window - we will handle it */
  /* if this doesn't go here, but later, then valgrind doesn't like it */
  p->atom_delwin_proto  = XInternAtom (p->display, "WM_PROTOCOLS",False);  
  p->atom_delwin    = XInternAtom (p->display, "WM_DELETE_WINDOW", False);
  XSetWMProtocols (p->display, p->window, &p->atom_delwin_proto, 1);
  XSetWMProtocols (p->display, p->window, &p->atom_delwin, 1);

  if(vb)printf("created X window:%p\n",(void*)p->window);

  sizehints.flags = PSize | PMinSize; /* let windowmanager decide - PPosition */
/*  sizehints.x=0;sizehints.y=0; */
  sizehints.height = p->height;
  sizehints.width  = p->width;
  sizehints.min_height = sizehints.height;
  sizehints.min_width  = sizehints.width;

  if (vb) printf("setting properties..\n");
  XSetStandardProperties(  p->display, p->window, 
                           p->name, 
                           "", None,  /* icon name and pixmap  */
                           argv, argc, 
                           &sizehints);

  /* GC gc = XCreateGC(p->display,p->window,0,NULL); */
   
  XMapRaised(p->display, p->window); /* visible and on top - else XMapWindow */

  XSelectInput(p->display, p->window,   ExposureMask | 
          KeyPressMask | 
          DestroyNotify | 
          StructureNotifyMask | 
          PointerMotionMask |
               /* PointerMotionHintMask | */
          ButtonPressMask | 
          ButtonReleaseMask ); 
  /* ClientMessage always reported */

  /* _init_display(p, p->width,p->height); */

  p->valid=1; /* = True; */

  /* lock and unlock pthread->init mutex - this ensures that calling thread is in pthread_wait before we send continue condition back */
  /* ( because condition_wait unlocks it) */
  mutex_lock(&p->thread_init_mutex);
  mutex_unlock(&p->thread_init_mutex);

  if(vb)printf("X11:init condition send:%p\n", (void*)&p->thread_init_cond);
  condition_send(&p->thread_init_cond);

  while (1) {
    if (XPending(p->display)>0)
      if(handle_events(p->display, p )==False)
        break;
    usleep(100);
  };

  mutex_lock(&p->mutex);
  p->valid=0;    /* global lock should be made when querying or change any windows valid state. */

  XSetCloseDownMode(p->display, DestroyAll);
  ptemp = p->display;
  p->display=NULL;
  XCloseDisplay(ptemp);
  
  
  /* after X close */

  if(vb)printf("X11:freeing cairo surface\n");
/*  _clean_display(p); */
  mutex_unlock(&p->mutex);
  if(vb)printf("X11:window thread quit\n");
  return NULL;
}




#endif /* ifndef _LCAIRO_WIN32 */
