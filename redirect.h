#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>

extern const int REDIR_OW;

extern const int REDIR_AP;

extern const int REDIR_ER;

void redirect(char **cmd_argv, char* filename, int options);
