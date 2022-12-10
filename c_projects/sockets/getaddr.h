/******************************************************************************


*******************************************************************************/
#ifndef GETADDR_H
#define GETADDR_H
#include "../daemonize/daemonize.h"
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <stdio.h>
void print_family(struct addrinfo* pai){
   printf("family: ");
   switch(pai->ai_family){
   case AF_INET:
      printf("inet,");
      break;
   case AF_INET6:
      printf("inet6,");
      break;
   case AF_UNIX:
      printf("unix,");
      break;
   case AF_UNSPEC:
      printf("unspecified,");
      break;
   default: 
      printf("unknown,");
   }
}

void print_type(struct addrinfo* pai){
   printf(" type: ");
   switch(pai->ai_socktype){
      case SOCK_STREAM:
         printf("sockstream,");
         break;
      case SOCK_DGRAM:
         printf("sockdgram,");
         break;
      case SOCK_SEQPACKET:
         printf("srqpacket,");
         break;
      default:
         printf("unknown (%d),",pai->ai_socktype);
   }
}

void print_proto(struct addrinfo* pai){
   printf(" protocol: ");
   switch(pai->ai_protocol){
      case 0:
         printf("default,");
         break;
      case IPPROTO_TCP:
         printf("TCP,");
         break;
      case IPPROTO_UDP:
         printf("UDP,");
         break;
      case IPPROTO_RAW:
         printf("raw,");
         break;
      default:
         printf("unknown(%d),",pai->ai_protocol);
   }
}

void print_flags(struct addrinfo* pai){
   printf(" flags:");
   if(!pai->ai_flags){
      printf(" 0");
   }
   else{
      if(pai->ai_flags & AI_PASSIVE)
         printf(" passive, ");
      if(pai->ai_flags & AI_CANONNAME)
         printf(" canonical, ");
      if(pai->ai_flags & AI_NUMERICHOST)
         printf(" numeric_host, ");
#ifdef AI_NUMERICSERV
      if(pai->ai_flags & AI_NUMERICSERV)
         printf("numeric_serv");
#endif
#ifdef AI_ALL
      if(pai->ai_flags & AI_ALL)
         printf(" all");
#endif
   }
}
#endif
