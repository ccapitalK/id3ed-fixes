#include "misc.h"
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>


int doread(int handle,void * buf,ssize_t len, const char * name){
   ssize_t i;
   if ((i=read(handle,buf,len))!=len){
      fprintf(stderr,"Error reading %s (%i/%i): %s\n",name,i,len,strerror(errno));
      return -1;
   }
   return 0;
}
int dowrite(int handle, const void * buf, ssize_t len,const char *name,const char *name2){
   ssize_t i;
   if ((i=write(handle,buf,len))!=len){
      fprintf(stderr,"Error writing %s %s (%i/%i): %s\n",name,name2,i,len,strerror(errno));
      return -1;
   }
   else return 0;
}
int doopen(int &handle,const char * name,int access,int mode) {
   if ((handle=open(name,access,mode))==-1){
      fprintf(stderr,"Error opening %s: %s\n",name,strerror(errno));
      return -1;
   }
   else return 0;
}
#ifndef HAVE_STRERROR
const char * strerror(int err){
	static char buf[5];
#ifdef HAVE_SNPRINTF
	snprintf(buf,5,"%i",err);
#else
	sprintf(buf,"%i",err);
#endif
	return buf;
}
#endif 
