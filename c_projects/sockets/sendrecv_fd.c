#include "sendrecv_fd.h"
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include "../err_funcs.h"
static int send_err(int fd, int errcode, const char *msg){
   int n;
   if((n = strlen(msg)) > 0)
      if(write(fd, msg, n) != n)
         return -1;
   if(errcode >= 0)
      errcode = -1;
   if(send_fd(fd, errcode) < 0)//send error message
      return -1;
   return 0;
}

//send file descriptor no another process, errcode is -(sending fd)
static struct cmsghdr *cmptr = NULL;//call malloc once
int send_fd(int fd, int sending_fd){
   struct iovec iov[1];
   struct msghdr msg;
   char buf[2];//for 2-byte protocol send_fd/recv_fd
   iov[0].iov_base = buf;
   iov[0].iov_len = 2;
   msg.msg_iov = iov;
   msg.msg_iovlen = 1;
   msg.msg_name = NULL;
   msg.msg_namelen = 0;
   if(sending_fd < 0){//if fd0 is not allowed to send, set buf[1] to -1
      msg.msg_control = NULL;
      msg.msg_controllen = 0;
      buf[1] = -sending_fd;//not zero is error
     //if(sending_fd == 0)
     //buf[1] = -1;
   }
   else{//not error, set buf[0], buf[1] to zero, 
        //copy sending_fd to cmptr->data, cmptr to msg.msg_control
      if(cmptr == NULL && 
            (cmptr = (struct cmsghdr *)malloc(CONTROLLEN)) == NULL)
         return -1;
      cmptr->cmsg_level = SOL_SOCKET;
      cmptr->cmsg_type = SCM_RIGHTS;//send some rights
      cmptr->cmsg_len = CONTROLLEN;
      msg.msg_control = cmptr;
      msg.msg_controllen = CONTROLLEN;
      *((int *)CMSG_DATA(cmptr)) = sending_fd;
      buf[1] = 0;
   }
   buf[0] = 0;
   if(sendmsg(fd, &msg, 0) != 2)
      return -1;
   return 0;
}

int recv_fd(int fd, ssize_t (*userfunc)(int, const void *, size_t)){
   int newfd, nr, status = -1;
   char buf[MAXLINE], *ptr;
   struct iovec iov[1];
   struct msghdr msg;
   for(;;){
      iov[0].iov_base = buf;
      iov[0].iov_len = sizeof(buf);
      msg.msg_iov = iov;
      msg.msg_iovlen = 1;
      msg.msg_name = NULL;
      msg.msg_namelen = 0;
      if(cmptr == NULL && 
            (cmptr = (struct cmsghdr *)malloc(CONTROLLEN)) == NULL)
         return -1;
      msg.msg_control = cmptr;
      msg.msg_controllen = CONTROLLEN;
      if((nr = recvmsg(fd, &msg, 0)) < 0){
         err_sys("error call recvmsg");
      }
      else if(nr == 0){
         err_ret("connection closed by a server");
         return -1;
      }
      for(ptr = buf; ptr < &buf[nr];){
         if(*ptr++ == 0){
            if(ptr != &buf[nr-1])
               err_dump("message format violation");
            status = *ptr & 0xFF;//prevent the extention of the sign bit
            if(!status){
               if(msg.msg_controllen != CONTROLLEN)
                  err_dump("received code zero without fd");
               newfd = *(int *)CMSG_DATA(cmptr);
            }
            else{newfd = -status;}
            nr -= 2;
         }
         if(nr > 0 && (*userfunc)(STDERR_FILENO, buf, nr) != nr)
            return -1;
         if(status >= 0)//added final data
            return newfd;//fd or errcode
      }
   }
}
