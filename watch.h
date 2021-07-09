#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <utmpx.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>


struct Node{
	char *name;
	int on;
	char *tty;
	char *host;
	pthread_t id;
	struct Node *next;
};


void insert(char *name, pthread_t id, char *type);
void delete(char *name, char *type);
void freeList(char *type);
int watchuser(char *user, int off, int first);
void *threaduser(void *some);
