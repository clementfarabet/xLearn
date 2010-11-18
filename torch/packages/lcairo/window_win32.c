#include "lcairo_defines.h"

#ifdef _LCAIRO_WIN32 
#include <windows.h>
#include <winuser.h>
#include <process.h>
#include <stdio.h>
#include <stdlib.h>
#include <cairo.h>
#include <cairo-win32.h>
#include "window.h"
#include "keytable.h"


extern int vb;

extern int vk_key_table[][2];

TCHAR szWindowClass[] = "torchwindowclass"; 
WNDCLASSEX  wcex;
LONG WINAPI WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam );
void WindowThreadFunc( void *userdata );

void last_error(){
  DWORD err = GetLastError();
  if (err!=0){
    LPSTR s;
    if(vb) printf("Error id:%d\n",(int)err);
    if ( FormatMessage(   FORMAT_MESSAGE_ALLOCATE_BUFFER|
          FORMAT_MESSAGE_FROM_SYSTEM,
          NULL,err,0,(LPTSTR)&s,0, NULL) == 0 ){
      if(vb) printf("FormatMessage failed\n");
    }else{
      if(vb) printf("Message:%s\n",s);
      LocalFree(s);
    }
  }
}


void window_init(){

  ATOM at;

  wcex.cbSize        = sizeof(wcex);
  wcex.style         = 0;/* CS_HREDRAW | CS_VREDRAW; */
  wcex.lpfnWndProc   = WndProc;
  wcex.cbClsExtra    = 0;
  wcex.cbWndExtra    = 0;
  wcex.hInstance     = NULL;
  wcex.hIcon         = LoadIcon(NULL,IDI_APPLICATION);
  wcex.hCursor       = LoadCursor(NULL,IDC_ARROW);
  wcex.hbrBackground = NULL;
  wcex.lpszMenuName  = NULL;
  wcex.lpszClassName = szWindowClass;
  wcex.hIconSm       = NULL;

  at = RegisterClassEx(&wcex);
  if (at==0){
    if(vb) printf("no ATOM after registring class\n");
    last_error();
    exit(1);
  }else{
    if(vb) printf("got at atom..\n");
  }
}

void tWindow_create_thread(tWindow *p){
  DWORD dwThreadId;

  mutex_create(&p->mutex);
  if(vb) printf("window create beginning thread\n");

  p->thread = CreateThread(
    NULL,
    0,
    (LPTHREAD_START_ROUTINE)WindowThreadFunc,
    (LPVOID)p,
    0,
    &dwThreadId);
    
  if (p->thread==NULL){
    printf("couldn't create thread\n");
    return;
  }

  if(vb) printf("window create init cond waiting\n");
  condition_create(&p->thread_init_cond);
  condition_wait(&p->thread_init_cond,NULL);
  if(vb) printf("window create init cond return\n");
  condition_destroy(&p->thread_init_cond);
}

void tWindow_destroy( tWindow* p ){

  if(vb) printf("platform window destroy:%p\n",p);

  PostMessage(p->hwnd,WM_CLOSE,0,0);
  WaitForSingleObject(p->thread, INFINITE);

  if(vb) printf("Threads joined\n");

  CloseHandle(p->thread);
}

void tWindow_refresh( tWindow* p ){

  if(vb) printf("crosswin_window_refresh_display\n");
  SendMessage( p->hwnd, WM_PAINT, 0, 0 );
}



LONG WINAPI WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam ){

  tWindow* p = (tWindow*)GetWindowLongPtr(hwnd,GWLP_USERDATA);
  int bpaint=0;

  if(vb) printf("tWindow:%p msg:%d\n",p,msg);

  switch(msg){
    case WM_ERASEBKGND:
      return 1;
    break;
    case WM_ACTIVATE:
      if(vb) printf("wm_activating - eg. focus\n");
    break;
    case WM_MOVE:
      if(vb) printf("wm_move\n");
      break;
    case WM_CREATE:
      if(vb) printf("wm_create\n");
      return 0;
    case WM_SIZE: {
      tMsg m;
      m.id      = TMSG_RESIZE;
      m.width   = LOWORD(lParam);
      m.height  = HIWORD(lParam);
      tWindow_message(p,m);

      if(vb) printf("wn_resize\n");
      p->width  = LOWORD(lParam);
      p->height = HIWORD(lParam);
      bpaint=1;
    }
    break;

    case WM_KEYDOWN:{
      tMsg m;
      int i=0;

      if(vb)printf("win32 key down\n");

      if ((int)wParam==VK_ESCAPE){
        PostQuitMessage(0); 
      }

      m.id     = TMSG_KEYDOWN;
      while (vk_key_table[i][0]!=KEY_NULL){
        if (vk_key_table[i][1] == (int)wParam){
          m.key_id=vk_key_table[i][0];    
          tWindow_message(p,m);
          break;
        }
        i++;
      };
    }
    break;    
  
    case WM_KEYUP:{
      tMsg m;
      int i=0;

      if(vb)printf("win32 key up\n");

      m.id     = TMSG_KEYUP;
      while (vk_key_table[i][0]!=KEY_NULL){
        if (vk_key_table[i][1] == (int)wParam){
          m.key_id=vk_key_table[i][0];    
          tWindow_message(p,m);
          break;
        }
        i++;
      };
    }
    break;

    case WM_LBUTTONDOWN:
    case WM_RBUTTONDOWN:{
      tMsg m;

      if(vb)printf("win32 Button Press\n");

      m.id     = (msg==WM_LBUTTONDOWN)?TMSG_MOUSE_BUTTON1_PRESS:TMSG_MOUSE_BUTTON2_PRESS;
      m.x           = LOWORD(lParam);
      m.y           = HIWORD(lParam);
      tWindow_message(p,m);
    }
    break;
    case WM_LBUTTONUP:
    case WM_RBUTTONUP:{
      tMsg m;

      if(vb)printf("win32 Button Release\n");

      m.id     = (msg==WM_LBUTTONUP)?TMSG_MOUSE_BUTTON1_RELEASE:TMSG_MOUSE_BUTTON2_RELEASE;
      m.x      = LOWORD(lParam);
      m.y      = HIWORD(lParam);
      tWindow_message(p,m);
    }
    break;

    case WM_MOUSEMOVE:{

      int mx = LOWORD(lParam);
      int my = HIWORD(lParam);
      /* if(vb)printf("win32 Mouse move:%d,%d\n",mx,my);*/
      tMsg m;
      m.id     = TMSG_MOUSE_MOVE;
      m.x      = mx;
      m.y      = my;
      tWindow_message(p,m);
    }
    break;


    case WM_SYNCPAINT:
    case WM_PAINT:{
      if(vb) printf("wm_paint please..\n");
        bpaint=1;
    }
    break;
    case WM_GETMINMAXINFO:{
      MINMAXINFO FAR *info;

      if (p==NULL)
        break;

      if(vb) printf("wm_getminmaxinfo p: %p minsize:%d,%d\n",p, p->width, p->height);

      info=(MINMAXINFO FAR *)lParam;
      info->ptMinTrackSize.x = p->min_width; 
      info->ptMinTrackSize.y = p->min_height+16; /* +16 border */
    }
    break;
    case WM_CLOSE:
      if(vb) printf("wm_close\n");
      PostQuitMessage(0); /* GetMessage returns false after this */
      return 0;
    case WM_DESTROY:
      if(vb) printf("wm_destroy\n");
      PostQuitMessage(0);
      return 0;
  };

  if (bpaint){


    tMsg m;
    cairo_surface_t* ps;
    cairo_t *cr;
    HDC hdc;

    m.id   = TMSG_PAINT;
    m.width   = -1;
    m.height   = -1;
    tWindow_message(p,m);
    
    /*  copy cairo surface direct on DC */
    hdc = GetDC(p->hwnd);/*, NULL, DCX_CACHE); */
    ps = cairo_win32_surface_create(hdc);

    cr = cairo_create(ps);

    /* simple blit */
    /* cairo_set_source_surface( cr, p->surf, 0,0 ); */
    /* cairo_rectangle(cr, 0,0,p->width,p->height); */
    /* cairo_fill(cr); */

    /* centred blit */
    {
      float margx = (p->width  - p->surf_width)/2;
      float margy = (p->height - p->surf_height)/2;
      float x1=0,x2=margx,x3=p->width-margx,x4=p->width;
      float y1=0,y2=margy,y3=p->height-margy,y4=p->height;

      if (vb) printf("centered blit, %d,%d %d,%d\n",p->width,p->height,p->surf_width,p->surf_height);

      cairo_set_source_rgba(cr,0.5,0.5,0.6,1.0);
      cairo_rectangle(cr, x1,y1,x2,y4); /* left  */
      cairo_rectangle(cr, x3,y1,x4,y4); /* right */
      cairo_rectangle(cr, x2,y1,x3,y2); /* middle upper */
      cairo_rectangle(cr, x2,y3,x3,y4); /* middle lower */
      cairo_fill(cr);
    

      /* doesn't like translate of 0.5 here.. */
      if (vb) printf("marge:%g,%g\n",margx,margy);

      cairo_translate(cr,(int)margx,(int)margy);
    }

    mutex_lock(&p->mutex);
     if (p->surf!=NULL){
      cairo_set_source_surface(cr,p->surf,0,0);
      cairo_surface_flush(p->surf);
      cairo_paint(cr);
      cairo_surface_flush(ps);
     }
    mutex_unlock(&p->mutex);

    cairo_destroy(cr);

    cairo_surface_destroy(ps);
    ReleaseDC(p->hwnd, hdc);

    /* InvalidateRect( p->hwnd, NULL, true); */
  }

  return DefWindowProc(hwnd,msg,wParam,lParam);
}



void WindowThreadFunc( void *userdata ){

	tWindow *p = (tWindow*)userdata;
	MSG msg;
	BOOL bRet;
	HWND hwnd;

	if(vb) printf("new cw window %p\n",p);

	hwnd = CreateWindow(szWindowClass,
			p->name, 
/* WS_OVERLAPPED|WS_CAPTION|WS_SYSMENU|WS_MINIMIZEBOX|WS_MAXIMIZEBOX,  no resize */
                      WS_OVERLAPPED|WS_CAPTION|WS_SYSMENU|WS_MINIMIZEBOX|WS_MAXIMIZEBOX|WS_THICKFRAME, /* equivalent to WS_OVERLAPPEDWINDOW, */
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			p->width+8,
			p->height+27,
			NULL,
			NULL,
			GetModuleHandle(NULL),
			NULL); 



	if (!hwnd){
		printf("no hWind after CreateWindow\n");
		last_error();
		exit(1);
	}else{
		if(vb) printf("got an hwnd..:%p\n",hwnd);
	}


	p->hwnd  = hwnd; /* used for sending messages  */

		
	/* for copyig to windows */
	/* cairo_image_surface_get_stride */

	SetWindowLongPtr(hwnd,GWLP_USERDATA,(LONG)p);


/*	tWindow_init_display(p,p->width,p->height); */

	p->valid=1;/* = true; */

	/* send signal for init complete */
	condition_send(&p->thread_init_cond);

	ShowWindow(hwnd,SW_SHOW);

	while ((bRet = GetMessage(&msg, NULL,0,0)) != 0){
    /* while (bRet = GetMessage(&msg, hwnd,0,0) != 0){ */
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	};

/*      mutex_lock(&p->mutex); */
/*	if(vb) printf("marking window invalid:%p\n",p); */
	p->valid=0;
/*	tWindow_clean_display(p); */
/*      mutex_unlock(&p->mutex); */

	if(vb) printf("ThreadFunc ending\n");
}

#endif /* _LCAIRO_WIN32 */

