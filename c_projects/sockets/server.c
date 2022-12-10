#include "../daemonize/daemonize.h"
#include "../err_funcs.h"
#include <netdb.h>
#include <errno.h>
#include <syslog.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <string.h>
#include <stdlib.h>

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

int main(int argc, char*argv[]){
   struct addrinfo hint, *ailist, *pai;
   int sockfd, err, n;
   char *host;
   if(argc != 1)
      err_quit("not need arguments to execute");
#ifdef _SC_HOST_NAME_MAX
   n = sysconf(_SC_HOST_NAME_MAX);
   if(n<0)
#endif
   n = HOST_NAME_MAX;
   host = (char*)malloc(n);
   if(!host)
      err_sys("error in call calloc");
   if(gethostname(host, n) < 0)
      err_sys("error call gethostname(%s, %d)",host, n);
   daemonize("ruptimed");
   hint.ai_flags = AI_CANONNAME;
   hint.ai_family = 0;
   hint.ai_socktype = SOCK_STREAM;
   hint.ai_protocol = 0;
   hint.ai_addrlen = 0;
   hint.ai_canonname = NULL;
   hint.ai_addr = NULL;
   hint.ai_next = NULL;
   if((err = getaddrinfo(host, "ruptime", &hint, &ailist))){
      syslog(LOG_ERR, "ruptimed: error call getaddrinfo: %s.",gai_strerror(err));
      exit(1);
   }
   for(pai = ailist; pai; pai = pai->ai_next){
      if((sockfd = initserver(SOCK_STREAM, pai->ai_addr, 
                                    pai->ai_addrlen, QLEN))>=0){

         serve(sockfd);
         return 0;
      }
   }
   exit(1);
}

