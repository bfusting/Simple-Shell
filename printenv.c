#include<stdio.h>
extern char **environ;
void printenv(){
 int i = 1;
 char *s = *environ;

 for(; s; i++) {
   printf("%s\n", s);
   s = *(environ+i);
 }
}


