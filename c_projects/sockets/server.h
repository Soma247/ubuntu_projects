#ifndef SERVER_H
#define SERVER_H
#include "../daemonize/daemonize.h"
#include "../err_funcs.h"
#include <netdb.h>
#include <errno.h>
#include <syslog.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <string.h>
#define QLEN 10
#ifndef HOST_NAME_MAX
#define HOST_NAME_MAX 256
#endif//hostname max

int initserver(int type, const struct sockaddr* addr,
                                   socklen_t alen, int qlen){
   int fd, err=0;
   if((fd = socket(addr->sa_family, type, 0)) < 0)
      return (-1);
   if(bind(fd, addr, alen) < 0){
      err = errno;
      goto errout;
   }
   if(type == SOCK_STREAM || type == SOCK_SEQPACKET){
      if(listen(fd, qlen) < 0){
         err = errno;
         goto errout;
      }
   }
   return fd;
errout:
   close(fd);
   errno = err;
   return (-1);


}

void serve(int sockfd){
   int clfd, status;
   pid_t pid;
   for(;;){
      clfd = accept(sockfd, NULL, NULL);
      if(clfd < 0){
         syslog(LOG_ERR, "ruptimed: error accept call: %s",strerror(errno));
         exit(1);
      }
      if((pid = fork()) < 0){
         syslog(LOG_ERR, "ruptimed: error call fork: %s", strerror(errno));
         exit(1);
      }
      else if(!pid){//forked proc
         //daemonize called, std fd's on /dev/null
         if(dup2(clfd, STDOUT_FILENO) != STDOUT_FILENO ||
               dup2(clfd, STDERR_FILENO) != STDERR_FILENO){
            syslog(LOG_ERR, "ruptimed: error in dup2 call");
            exit(1);
         }
         close(clfd);
         execl("/usr/bin/uptime", "uptime", NULL);
         syslog(LOG_ERR, "ruptimed: unexpected return from exec: %s",
                                                         strerror(errno));

      }
      else{//parent proc
         close(clfd);
         waitpid(pid, &status, 0);
      }
   }
}

#endif//SERVER_H
