#include "config.h"
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <sys/types.h>
#include <sys/stat.h>
#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif

#define RDWRMODE (O_RDWR)
#define RDMODE (O_RDONLY)


int doread(int handle,void * buf,ssize_t len, const char * name);
int dowrite(int handle, const void * buf, ssize_t len,const char *name,const char *name2="");
int doopen(int &handle,const char * name,int access,int mode=0);


#ifndef HAVE_STRERROR
const char * strerror(int err);
#endif
