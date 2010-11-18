#ifndef _THREADS_HDEF
#define _THREADS_HDEF


#include "lcairo_defines.h"

#ifdef _LCAIRO_WIN32
#include <windows.h>
typedef HANDLE	mutex_t;
typedef HANDLE	cond_t;
#else
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <pthread.h>
typedef pthread_mutex_t mutex_t; 
typedef pthread_cond_t	cond_t;
#endif

void	mutex_create	(mutex_t* cw);
void 	mutex_destroy	(mutex_t* cw);
void	mutex_lock	(mutex_t* cw);
void 	mutex_unlock	(mutex_t* cw);
void 	mutex_destroy	(mutex_t* cw);

void condition_create	(cond_t* cond);
void condition_destroy	(cond_t* cond);
void condition_send	(cond_t* cond);
void condition_wait	(cond_t* cond, mutex_t* mutex);

#endif /* _THREADS_HDEF */
