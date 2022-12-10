#include "getaddr.h"
#include <stdbool.h>
#include <pthread.h>

int main(int argc, char*argv[]){
   struct addrinfo hint, *ailist, *pai;
   struct sockaddr_in *sinp;
   const char *addr;
   int err;
   char abuf[INET_ADDRSTRLEN];
   if(argc!=3){
      printf("Using: %s, hostname(node) servicename",argv[0]);
      return 1;
   }
   hint.ai_flags = AI_CANONNAME;
   hint.ai_family = 0;
   hint.ai_socktype = 0;
   hint.ai_protocol = 0;
   hint.ai_addrlen = 0;
   hint.ai_canonname = NULL;
   hint.ai_next = NULL;
   if((err=getaddrinfo(argv[1], argv[2], &hint, &ailist))){
      printf("error in getaddrinfo call: %s",gai_strerror(err));
      return 2;
   }
   for(pai=ailist; pai; pai=pai->ai_next){
      print_flags(pai);
      print_family(pai);
      print_type(pai);
      print_proto(pai);
      printf("\n\t host %s", pai->ai_canonname?pai->ai_canonname:"-");
      if(pai->ai_family == AF_INET){
         sinp = (struct sockaddr_in*)pai->ai_addr;
         addr = inet_ntop(AF_INET, &sinp->sin_addr, abuf, INET_ADDRSTRLEN);
         printf(" address %s", addr? addr : "unknown");
         printf(" port %d", ntohs(sinp->sin_port));
      }
      printf("\n");
   }
   return 0;
}

