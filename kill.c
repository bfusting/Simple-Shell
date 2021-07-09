#include <stdio.h>
#include <sys/types.h>
#include<signal.h>

int kill_s(pid_t pid, int sig){

 kill(pid, sig);

 return 0;

}

