#include <netdb.h>
#include <errno.h>
#include <sys/socket.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include "../err_funcs.h"
#define MAXADDRLEN 256
#define BUFLEN 128
#define MAXTIME 120

int connect_retry(int sockfd, const struct sockaddr *addr,
                                       socklen_t alen, size_t seconds){
   if(!seconds) seconds = 1;
   for(size_t time = 1; time <= seconds; time <<= 1){
      if(!connect(sockfd, addr, alen))
         return 0;
      if(time <= seconds/2)
         sleep(time);
   }
   return -1;
}
void print_uptime(int sockfd){
   int n;
   char buf[BUFLEN];
   while((n = recv(sockfd, buf, BUFLEN, 0)) > 0)
      write(STDOUT_FILENO, buf, n);
   if(n<0)
      err_sys("error in call recv");
}

int main(int argc, char *argv[]){
   struct addrinfo hint, *ailist, *pai;
   int sockfd, err;
   if(argc != 2)
      err_quit("Execute: client hostname");
   hint.ai_flags = 0;
   hint.ai_family = 0;
   hint.ai_socktype = SOCK_STREAM;
   hint.ai_protocol = 0;
   hint.ai_addrlen = 0;
   hint.ai_canonname = NULL;
   hint.ai_addr = NULL;
   hint.ai_next = NULL;
   if((err = getaddrinfo(argv[1], "ruptime", &hint, &ailist)))
         err_quit("error call getaddrinfo: %s", gai_strerror(err));
   for(pai = ailist; pai; pai = pai->ai_next){
      if((sockfd = socket(pai->ai_family, SOCK_STREAM, 0)) < 0)
         err = errno;
      if(connect_retry(sockfd, pai->ai_addr, pai->ai_addrlen, 120) < 0){
         err = errno;
      }
      else{
         print_uptime(sockfd);
         return 0;
      }
      fprintf(stderr, "can't connect to %s: %s\n", argv[1], strerror(err));
      return -1;
   }
}
