#include "get_arg.h"

char **get_argv(char *buf, int argc){
	char **argv = malloc((argc+1) * sizeof(char *));
	char *cur_string = strtok(buf, " \n");
	int i = 0;
	while(cur_string != NULL && i < argc) {
		argv[i] = malloc(sizeof(char *) * (strlen(cur_string) +1));
		strcpy(argv[i], cur_string);
		cur_string = strtok(NULL, " ");
		i++;
	}
	if(argv[argc-1][strlen(argv[argc-1])-1]=='\n'){
		argv[argc-1][strlen(argv[argc-1])-1] = '0';}
	argv[argc] = NULL;
	return argv;
}

int get_argc(char *buf){
	int argc = 0;
	if(strlen(buf) > 0){
		if(buf[0] != ' ' && buf[0] != '\n'){argc++;}
		for(int i = 0; buf[i] != '\0' && buf[i]; i++){
			if(i != 0 && 1+i != strlen(buf) -1 && buf[i] == ' ' && (buf[i+1] != ' ')){
				argc++;
			}
		}
	}
	return argc;
}
