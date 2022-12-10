#ifndef LOG_FUN_H
#define LOG_FUN_H
/*
 fun     | strerror append | calls
 log_msg      no             return
 log_quit     no             exit(2)
 log_ret     errno           return
 log_sys     errno           exit(2)
 
 
*/
#include <sys/types.h>
#define MAXLINE 4096
void log_msg(const char *, ...);
void log_open(const char *, int, int);
void log_quit(const char *, ...);
void log_ret(const char *, ...);
void log_sys(const char *, ...);
#endif//LOG_FUN_H
