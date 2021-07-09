#include <stdio.h>
#include <unistd.h>

int main(void)
{
  char *argv[] = { "/bin/sh", "-c", "env", 0 };
  char *envp[] =
    {
      "HOME=/",
      "PATH=/bin:/usr/bin",
      "TZ=UTC0",
      "USER=cshen",
      "LOGNAME=cshen",
      0
    };
  // execve(argv[0], &argv[0], envp);
  execve(argv[0], argv, envp);
  fprintf(stderr, "Oops! (something is wrong here...\n");
  return -1;
}
