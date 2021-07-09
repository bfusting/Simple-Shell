#include "get_path.h"

int pid;
char *prompt(char *message);
char *where(char *command, struct pathelement *pathlist);
char *which(char *command, struct pathelement *pathlist);
void setenv_s(char *env, char *val);
void printenv();
void list(char *path);
void list(char *dir);
int *kill_s(pid_t pid, int signal);
int cd(char *path);
#define PROMPTMAX 64
#define MAXARGS   16
#define MAXLINE   128
