#include "daemonize.h"
#include <stdbool.h>
#include <pthread.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <sys/file.h>
#define LOCKFILE "/var/run/test_daemon.pid"
#define LOCKMODE (S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH)

   int already_running(void){
      int fd;
      char buf[16];
      fd=open(LOCKFILE, O_RDWR|O_CREAT, LOCKMODE);
      if(fd<0){
         syslog(LOG_ERR, "can't open %s: %s",
                        LOCKFILE, strerror(errno));
         exit(1);
      }
      if(flock(fd,LOCK_EX|LOCK_NB)<0){
         if(errno==EWOULDBLOCK){
            close(fd);
            return 1;
         }
         syslog(LOG_ERR, "can't block file %s: %s",
                        LOCKFILE, strerror(errno));
         exit(1);
      }
      ftruncate(fd,0);
      sprintf(buf, "%ld", (long)getpid());
      write(fd, buf, strlen(buf)+1);
      return 0;
   }

   sigset_t mask;
   void reread(void){}
   void* thrfun(void*){
      int err, signo;
      while(true){
         err=sigwait(&mask, &signo);
         if(err){
            syslog(LOG_ERR, "error in calling sigwait");
            exit(1);
         }
         switch(signo){
         case SIGHUP:
            syslog(LOG_INFO, "read configuration file");
            reread();
            break;
         case SIGTERM:
            syslog(LOG_INFO, "SIGTERM signal received, exiting");
            exit(0);
         default:
            syslog(LOG_INFO, "receive signal %d\n",signo);
         }
      }
      return 0;
   }

int main(int argc, char*argv[]){
   int err;
   pthread_t tid;
   char* cmd;
   struct sigaction sa;
   if((cmd=strrchr(argv[0], '/'))==NULL)
      cmd=argv[0];
   else ++cmd;
   //to daemon mode
   daemonize(cmd);
   //check if another instance of this daemon already running
   if(already_running()){
      syslog(LOG_ERR, "another instance of test_daemon already running");
      return 1;
   }
   //set default handler to sughup, block all signals
   sa.sa_handler = SIG_DFL;
   sigemptyset(&sa.sa_mask);
   sa.sa_flags=0;
   if(sigaction(SIGHUP, &sa, NULL)<0){
      syslog(LOG_ERR, "can't set default handler to SIGHUP: %s",
                                                      strerror(errno));
      return 1;
   }
   sigfillset(&mask);
   if((err=pthread_sigmask(SIG_BLOCK, &mask, NULL))){
      syslog(LOG_ERR, "error set SIG_BLOCK");
      return 1;
   }
   if((err=pthread_create(&tid, NULL, thrfun, 0))){
      syslog(LOG_ERR, "error in pthread_create");
      return 1;
   }
   //daemon code begin
   syslog(LOG_INFO, "test_daemon started");
   while(true)sleep(1);
   //daemon code end
   return 0;
}

