#include "log_funcs.h"
#include <stdlib.h>
#include <errno.h>
#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include <syslog.h>
static void log_doit(int errnoflag, int priority, const char *fmt, va_list ap){
   int errno_save;
   char buf[MAXLINE];
   errno_save = errno;
   vsnprintf(buf, MAXLINE, fmt, ap);
   if(errnoflag)
         snprintf(buf+strlen(buf), MAXLINE-strlen(buf), ":%s", 
                                                strerror(errno_save));
   strcat(buf, "\n");
   syslog(priority, buf);
}

void log_ret(const char *fmt, ...){
   va_list ap;
   va_start(ap, fmt);
   log_doit(1, LOG_ERR, fmt, ap);
   va_end(ap);
}

void log_sys(const char *fmt, ...){
   va_list ap;
   va_start(ap, fmt);
   log_doit(1, LOG_ERR, fmt, ap);
   va_end(ap);
}

void log_msg(const char *fmt, ...){
   va_list ap;
   va_start(ap, fmt);
   log_doit(0, LOG_ERR, fmt, ap);
   va_end(ap);
}

void log_quit(const char *fmt, ...){
   va_list ap;
   va_start(ap, fmt);
   log_doit(0, LOG_ERR, fmt, ap);
   va_end(ap);
   exit(2);
}
