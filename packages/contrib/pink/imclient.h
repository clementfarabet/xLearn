/**********************************************************
 imclient.h
 -----------
 
**********************************************************/  

#ifndef IMCLIENT_H
#define IMCLIENT_H

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

#include "liarwrap.h" /* need definition of IMAGE */

typedef enum {SHM_NONE = 0, SHM_POSIX, SHM_SYSV} ipctype;

/* prototypes from imclient.c - connection with imview server */
int imviewput(IMAGE          *I,  const char *name,  const char *user,
	      const char     *hostname,   short  port);

void imview_force_socket(void);
int imviewlogin(const char *user,  const char *hn, int port, int *conn_id);
char *imviewsendcommand(const char *cmd, int         conn_id);
int imviewputimage(IMAGE *I,  const char  *name, int  conn_id);
void imview_set_transfer_method(const ipctype method);
void imviewlogout(int conn_id);

#ifdef __cplusplus
}
#endif /* __cplusplus */
    
#endif /* IMCLIENT_H */
