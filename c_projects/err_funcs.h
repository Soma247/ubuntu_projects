#ifndef ERR_FUN_H
#define ERR_FUN_H
/*
 fun     | strerror append | calls
 err_dump    errno           abort()
 err_exit    from param      exit(1)
 err_msg      no             return
 err_quit     no             exit(1)
 err_ret     errno           return
 err_sys     errno           exit(1)
 
 
*/
#define MAXLINE 4096
#include <sys/types.h>
void err_dump(const char *, ...);
void err_msg(const char *, ...);
void err_quit(const char *, ...);
void err_exit(int, const char *, ...);
void err_ret(const char *, ...);
void err_sys(const char *, ...);
#endif//ERR_FUN_H
