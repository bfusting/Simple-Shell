//#define _XOPEN_SOURCE 500
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <sys/wait.h>
#include "sh.h"
#include <pthread.h>


void sig_handler(int sig){
	printf("\ncaught signal\n");
	signal(sig, SIG_IGN);
	
	
}


int
main(int argc, char **argv, char **envp)
{
	signal(SIGINT, sig_handler);
	signal(SIGTSTP, sig_handler);	
	signal(SIGTERM, sig_handler);

	char	buf[MAXLINE];
	char    *arg[MAXARGS];  // an array of tokens
	char    *ptr;
        char    *pch;
	pid_t	pid;
	int	status, i, arg_no;
        char *prompt_message = ">> ";
	char *stat;
	int noclobber = 0;

	printf(">> ");	/* print prompt (printf requires %% to print %) */
	while ((stat = fgets(buf, MAXLINE, stdin)) != NULL) {
		if (strlen(buf) == 1 && buf[strlen(buf) - 1] == '\n')
		  goto nextprompt;  // "empty" command line

		if (buf[strlen(buf) - 1] == '\n')
			buf[strlen(buf) - 1] = 0; /* replace newline with null */
		// parse command line into tokens (stored in buf)
		arg_no = 0;
                pch = strtok(buf, " ");
                while (pch != NULL && arg_no < MAXARGS)
                {
		  arg[arg_no] = pch;
		  arg_no++;
                  pch = strtok (NULL, " ");
                }
		arg[arg_no] = (char *) NULL;

		if (arg[0] == NULL)  // "blank" command line
		  goto nextprompt;

		/* print tokens
		for (i = 0; i < arg_no; i++)
		  printf("arg[%d] = %s\n", i, arg[i]);
                */
		else
		if(strcmp(arg[0], "noclobber") == 0) {// built-in command noclobbe
		  printf("Executing built-in [noclobber]\n");
		  noclobber = 1 - noclobber;
		  printf("%d\n", noclobber);
		}
		else
                if (strcmp(arg[0], "pwd") == 0) { // built-in command pwd 
		  printf("Executing built-in [pwd]\n");
	          ptr = getcwd(NULL, 0);
                  printf("%s\n", ptr);
                  free(ptr);
	        }
		else
		if (strcmp(arg[0], "list") == 0) { //built-in command list
			printf("Executing built-in [list].\n");
			if(arg[1] == NULL){
				list(getcwd(NULL, 0));
			}
			else list(arg[1]);
		}
		else
		if (strcmp(arg[0], "prompt") == 0) { //built-in command prompt
			printf("Executing built-in [prompt].\n");
			if(arg[1] == NULL){
					printf("Enter a prompt: ");
					char str[100];
					fgets(str, 100, stdin);
					prompt_message = prompt(str);
					goto nextprompt;
			}
			else{
				prompt_message = prompt(arg[1]);
				goto nextprompt;
			}
		}
		

		else
		if (strcmp(arg[0], "kill") == 0) { //built-in command kill
			 
		 	printf("Executing built-in [kill]\n");
			if(arg[1] == NULL){
				printf("[kill] not enough arguments");
				goto nextprompt;
				}
			if(arg[2] == NULL){
				int p = *arg[1] - '0';
				kill_s(p, SIGTERM);
				goto nextprompt;
				}
			else{
			  int p = *arg[2] - '0';
			  int s = *arg[1] - '0';
			  kill_s(p, s);
			  goto nextprompt;
			}
		}
		else
		if (strcmp(arg[0], "pid") == 0) { //built-in command pid
			printf("Executing built-in [pid]\n");
			printf("%ld\n",(long)getpid());
		}
		else
		if (strcmp(arg[0], "exit") ==0) {//built-in command exit
			printf("Executing built-in [exit].\n");
			exit(0);
			}
		else
		if (strcmp(arg[0], "printenv") == 0) {
			printf("Executing built-in [printenv].\n");
			if(arg[1] == NULL){
			printenv();
			goto nextprompt;
			}

			if(arg[2] != NULL){
				printf("printenv: too many arguments.\n");
				goto nextprompt;
			}
			else{
				printf("%s\n", getenv(arg[1]));
				goto nextprompt;
			}
		}

		else
		if (strcmp(arg[0], "setenv") == 0) { //built-in command setenv
			printf("Executing built-in [setenv].\n");
			if(arg[3] != NULL){
				printf("setenv: too many arguments.\n");
				goto nextprompt;
			}
			else{
				if(arg[1] == "PATH"){
					char *p = getenv("PATH");
					p = strcat(p, arg[1]);
					setenv_s(p, arg[2]);
					goto nextprompt;
				}
				if(arg[2] == NULL){
					setenv_s(arg[1], "");
					goto nextprompt;
				}
				else{
				setenv_s(arg[1], arg[2]);
				goto nextprompt;
				}
			}

		}
		else
		if (strcmp(arg[0], "cd") == 0) { //built-in command cd
		  printf("Executing built-in [cd].\n");
		  if(arg[1] == NULL){
		    cd(getenv("HOME"));
		    goto nextprompt;
		  }
		  if(strcmp(arg[1], "-") == 0){
			cd("./..");
			
		  }
		  else{
			cd(arg[1]);
		  }
	
		}
		
		else
		if (strcmp(arg[0], "where") == 0) { //built-in command where

		 if(arg[1] == NULL) { //"empty" where
               		printf("where: Too few arguments.\n");
			goto nextprompt;
		       }



		  if(strcmp(arg[1],"where") == 0 || strcmp(arg[1], "cd") == 0 || strcmp(arg[1], "printtenv") == 0 || strcmp(arg[1], "setenv") == 0 || strcmp(arg[1], "pid") == 0
		     || strcmp(arg[1], "pwd") ==0 || strcmp(arg[1], "prompt") == 0 || strcmp(arg[1], "kill") == 0 || strcmp(arg[1], "exit") == 0 || strcmp(arg[1], "list") == 0){
			printf("[%s] is a built-in command.\n", arg[1]);
			goto nextprompt; 
	  	  }	 

		 struct pathelement *p, *tmp;
		 char *cmd;
 		 printf("Executing built-in [where]\n");


		 
		 p = get_path();
		 tmp = p;
		
		 cmd = where(arg[1], p);
		 if(cmd) {
		   printf("%s\n", cmd);
 		   free(cmd);

	 	 }
 		 else{
 		   printf("%s: Command not found\n", arg[1]);
		 }
		}
		else
                if (strcmp(arg[0], "which") == 0) { // built-in command which
		  if(strcmp(arg[1],"where") == 0 || strcmp(arg[1], "cd") == 0 || strcmp(arg[1], "printtenv") == 0 || strcmp(arg[1], "setenv") == 0 || strcmp(arg[1], "pid") == 0
		     || strcmp(arg[1], "pwd") ==0 || strcmp(arg[1], "prompt") == 0 || strcmp(arg[1], "kill") == 0 || strcmp(arg[1], "exit") == 0 || strcmp(arg[1], "list") == 0){
			printf("[%s] is a built-in command.\n", arg[1]);
			goto nextprompt; 
	  	  }	 


	
		  struct pathelement *p, *tmp;
                  char *cmd;
                    
		  printf("Executing built-in [which]\n");

		  if (arg[1] == NULL) {  // "empty" which
		    printf("which: Too few arguments.\n");
		    goto nextprompt;
                  }

		  p = get_path();
           


                  cmd = which(arg[1], p);
                  if (cmd) {
		    printf("%s\n", cmd);
                    free(cmd);
                  }
		  else               // argument not found
		    printf("%s: Command not found\n", arg[1]);

		  while (p) {   // free list of path values
		     tmp = p;
		     p = p->next;
		     free(tmp->element);
		     free(tmp);
                 }
		}
		
		else
		if(arg[0][0] == '/'  || arg[0][0] == '.') { // Checking for absolute path
			pid_t child;

			if(access(arg[0], X_OK) == 0) {
				if((child = fork()) > 0) {
					waitpid(child, NULL, 0);
					goto nextprompt;
				} 
				else if(!child){
					execve(arg[0],&arg[1], envp);
				}
			}
	
		}


		else {
			struct pathelement *p;
			p = get_path();
			char *path;
			path = which(arg[0], p);
			

		  if ((pid = fork()) < 0) {
			printf("fork error");
		  } else if (pid == 0) {
							/* child */
			execve(path, &arg[0], envp);
			printf("couldn't execute: %s", buf);
			goto nextprompt;
		  }

		  /* parent */
		  if (pid){
			waitpid(-1,&status,WNOHANG);
			goto nextprompt;}
		 goto nextprompt;
/**		
                  if (WIFEXITED(status)) S&R p. 239 
                    printf("child terminates with (%d)\n", WEXITSTATUS(status));
**/
                }


	



           nextprompt:
		printf("%s", prompt_message);
	}

	if(stat == NULL){
		printf("^D\n");
		printf("Use \"exit\" to leave shell.\n");
		goto nextprompt;
	}


	exit(0);
}
