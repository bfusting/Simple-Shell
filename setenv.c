#include<stdio.h>
#include<stdlib.h>
#include<string.h>
extern char **environ;
void setenv_s(char *env, char* val){
     if(env == NULL){
 	int i = 1;
 	char *s = *environ;

 	for(; s; i++) {
          printf("%s\n", s);
          s = *(environ+i);
 	}
      }
    else{
	setenv(env,val,1);
	printf("%s\n", getenv(env));





   }

}



