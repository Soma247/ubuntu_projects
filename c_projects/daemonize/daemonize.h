/******************************************************************************


*******************************************************************************/
#ifndef DAEMONIZE_H
#define DAEMONIZE_H
#include <stdio.h>
#include <signal.h>
#include <syslog.h>
#include <fcntl.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <unistd.h>

void daemonize(const char* cmd){
    pid_t pid;
    int fd0, fd1, fd2;
    struct rlimit rl;
    struct sigaction sa;
    umask(0);//clear create file umask
    //get max filedescriptor
    if((pid=fork())<0)
        exit(2);
    else if(pid!=0)
        exit(0);//close parent
    setsid();//get new session leader without term
    //ignore sighup, for not get control terminal in future
    sa.sa_handler=SIG_IGN;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags=0;
    if(sigaction(SIGHUP, &sa, NULL)<0)
        exit(3);
    //fork second time, not a leader of session, but orphan
    if((pid=fork())<0)
        exit(4);
    else if(pid!=0)
        exit(0);
    if(chdir("/")<0)//curdir is root
        exit(5);
    //close all fd, set 0,1,2 to dev/null, open syslog;
    if(getrlimit(RLIMIT_NOFILE, &rl)<0)
    exit(1);
    if(rl.rlim_max == RLIM_INFINITY)
        rl.rlim_max = 1024;
    for(rlim_t i=0; i<rl.rlim_max; ++i)
        close(i);
    fd0=open("/dev/null",O_RDWR);
    fd1=dup(0);
    fd2=dup(0);
    openlog(cmd, LOG_CONS|LOG_PID|LOG_PERROR, LOG_DAEMON);
    
    if(fd0!=0 || fd1!=1 || fd2!=2){
        syslog(LOG_ERR, "faulure file descriptors %d %d %d", fd0, fd1, fd2);
        exit(2);
    }
}
#endif
