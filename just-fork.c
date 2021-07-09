/*
 * CISC 361: sample program using fork() only
 */

#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>

main(int argc, char **argv, char **envp)
{
  int i;
  pid_t pid;
  char  *arg[4];

  arg[0] = (char *) malloc(100); 
  strcpy(arg[0], "child");
  arg[1] = (char*) 0;

  printf("start a child...\n");

  pid = fork();

  if (pid == 0) {
    pid = getpid();
    while (1) {
      printf("child [%d]\n", pid);
      sleep(2);
    }
  }
  else {
    pid = getpid();
    while (1) {
      printf("parent [%d]\n", pid);
      sleep(3);
    }
  }
}
