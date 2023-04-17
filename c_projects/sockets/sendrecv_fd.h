#ifndef SENDRECV_FD_H
#define SENDRECV_FD_H
#include <sys/socket.h>
//len of struct cmsghdr+ nbytes
#ifndef CMSG_LEN
#define CMSG_LEN(unsigned int nbytes) sizeof(struct{socklen_t s; int i[2];char n[nbytes];})
#endif//CMSG_LEN

/*protocol: send {'/0','/0',fd} if all ok, 
 or {char errmsg[], '/0', unsigned char errcode(1-255)} if error ocured
 recv_fd read all bytes before '/0', call userfun with this bytes, if next byte
 is '/0', next is fd, if not zero - it is errcode and array is errmessage
 * */

#define CONTROLLEN CMSG_LEN(sizeof (int))//len of cmsghdr for this protocol
int send_fd(int fd, int sending_fd);
//userfunc(STDERR_FILENO, &buf, buflen) is error handler, returns count of 
//writed bytes or -(error_code)
int recv_fd(int fd, ssize_t (*userfunc)(int, const void*, size_t));
#endif//SENDRECV_FD_H
