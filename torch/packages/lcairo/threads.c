#include "lcairo_defines.h"
#include "threads.h"


#ifdef _LCAIRO_WIN32 
#include <process.h>
void mutex_create	(mutex_t* mutex){*mutex = CreateMutex(0,FALSE,0);}
void mutex_destroy	(mutex_t* mutex){CloseHandle( *mutex ); }
void mutex_lock		(mutex_t* mutex){WaitForSingleObject(*mutex,INFINITE); }
void mutex_unlock	(mutex_t* mutex){ReleaseMutex(*mutex); }
void condition_create	(cond_t* cond)	{*cond = CreateSemaphore(NULL,0,1024,NULL); }
void condition_destroy	(cond_t* cond)	{CloseHandle(*cond); }
void condition_send	(cond_t* cond)	{ReleaseSemaphore(*cond,1,NULL); }
void condition_wait	(cond_t* cond, mutex_t* mutex){WaitForSingleObject(*cond,INFINITE);}
#else
void mutex_create	(mutex_t* mutex)	{ pthread_mutex_init( mutex, NULL); }
void mutex_destroy	(mutex_t* mutex)	{ pthread_mutex_destroy( mutex ); }
void mutex_lock		(mutex_t* mutex)	{ pthread_mutex_lock(mutex); }
void mutex_unlock	(mutex_t* mutex)	{ pthread_mutex_unlock(mutex); }

void condition_create	(cond_t* cond)	{ pthread_cond_init( cond,NULL);}
void condition_destroy	(cond_t* cond)	{ pthread_cond_destroy(cond);}
void condition_send	(cond_t* cond)	{ pthread_cond_broadcast(cond); }
void condition_wait	(cond_t* cond, mutex_t* mutex){ pthread_cond_wait(cond, mutex);}
#endif
