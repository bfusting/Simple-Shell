/* Some commands such as list and where need to be transfered over into this file  DO NOT FORGET
*/
#include "get_arg.h"
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <glob.h>
#include <sys/wait.h>
#include "sh.h"
#include <utmpx.h>
#include <pthread.h>
#include "redirect.h"
#include <errno.h>
#include "watch.h"
#include "m_pipe.h"

void sig_handler(int sig)
{
  fprintf(stdout, "\n>> ");
  fflush(stdout);
}
  
int main(int argc, char **argv, char **envp)
{
	char *prompt_message = "";
	int nargs;
	char	buf[MAXLINE];
	char    *arg[MAXARGS];
	char 	**args;  // an array of tokens
	char    *ptr;
        char    *pch;
	pid_t	pid;
	int	status, i, arg_no, background, noclobber;
	int     redirection, append, pipe, rstdin, rstdout, rstderr;
        struct pathelement *dir_list, *tmp;
        char *cmd;
	pthread_t thread_id;

        noclobber = REDIR_OW;         // initially default to 0

        signal(SIGINT, sig_handler);
	signal(SIGTERM, sig_handler);
	signal(SIGTSTP, sig_handler);
	fprintf(stdout, ">> ");	/* print prompt (printf requires %% to print %) */
	fflush(stdout);
	while (fgets(buf, MAXLINE, stdin) != NULL) {
	while(waitpid((pid_t)(-1), 0, WNOHANG) > 0) {}
 
		if (strlen(buf) == 1 && buf[strlen(buf) - 1] == '\n')
		  goto nextprompt;  // "empty" command line

		if (buf[strlen(buf) - 1] == '\n')
			buf[strlen(buf) - 1] = 0; /* replace newline with null */

	                       // no redirection or pipe

           /***
                printf("-%d-%d-%d-%d-%d-%d-\n",
		       redirection, append, pipe, rstdin, rstdout, rstderr);
           ***/


		int first_watchuser = 1;  //Indicates if a watchuser thread has been started.


		// parse command line into tokens (stored in buf)
		nargs = get_argc(buf);
 		args = get_argv(buf, nargs);		
            	args[nargs+1] = (char *) NULL;

		if (args[0] == NULL)  // "blank" command line
		  goto nextprompt;

                background = 0;      // not background process
		if (args[nargs-1][0] == '&')
		  background = 1;    // to background this command

		
	        redirection = append = pipe = rstdin = rstdout = rstderr = 0; 
		//This Section creates an array argum containing the command and command options
		int index = 1;
		char *t1 = args[0];
		while(t1 != NULL){

                // check for >, >&, >>, >>&, <, |, and |&
		if (strstr(buf, ">>&")) {
		  redirection = append = rstdout = rstderr = 1; break;}
		else
		if (strstr(buf, ">>")) {
		  redirection = append = rstdout = 1; break;}
		else
		if (strstr(buf, ">&")) {
		  redirection = rstdout = rstderr = 1; break;}
		else
		if (strstr(buf, ">")) {
		  redirection = rstdout = 1; break;}
		else
		if (strstr(buf, "<")) {
		  redirection = rstdin = 1; break;}
		else
		if (strstr(buf, "|&")) {
		  pipe = rstdout = rstderr = 1; break;}
		else
		if (strstr(buf, "|")){ 
		  pipe = rstdout = 1; break;}

		 t1 = args[index];
		 index++;
		}
		

		if(redirection || pipe){
		int t = index-1;
		char **argum = malloc(sizeof(char *) * (index));
		char **argum_2 = malloc(sizeof(char *) * (nargs - index +1));

		for(index = 0; index < t; index++){
			int size = 1 + strlen(args[index]);
			argum[index] = args[index];
		}
		argum[index] = NULL;
		
		for(index = t; index < nargs; index++){
			argum_2[index-t] = (args[index]);
		} 
		argum_2[index - t] = NULL;
		index = 0;

		 if(rstdout && !append){
			redirect(argum, args[2], noclobber);
		 }
		 if(rstdin && !append && !rstderr) redirect(argum_2, argum[2], noclobber);

		 if(rstdout && append && !rstderr) redirect(argum, args[2], noclobber | REDIR_AP);

		 if(rstdout && rstderr && !append) redirect(argum, args[2], noclobber | REDIR_ER);

		 if(rstdout && rstderr && append) redirect(argum, args[2], noclobber | REDIR_AP | REDIR_ER);
	
		 if(pipe && !rstderr){
			open_pipe(argum_2, argum, 0);}
		 if(pipe && rstderr){open_pipe(argum_2, argum, 1);}
		

		}
		
		// print tokens of this command line
            /***
		for (i = 0; i < nargs; i++)
		  printf("arg[%d] = %s\n", i, arg[i]);
            ***/

                if (strcmp(args[0], "exit") == 0) { // built-in command exit
		  printf("Executing built-in [exit]\n");
		  for (int i = 0; i < nargs; i++) {
       			 free(args[i]);
     		 }
      		 free(args);
                  exit(0);
	        }
		else
		if (strcmp(args[0], "list") == 0) { //built-in command list
			printf("Executing built-in [list].\n");
			if(args[1] == NULL){
				list(getcwd(NULL, 0));
			}
			else list(args[1]);
		}
		else
		if (strcmp(args[0], "prompt") == 0) { //built-in command prompt
			printf("Executing built-in [prompt].\n");
			if(args[1] == ""){
					printf("Enter a prompt: ");
					char str[100];
					fgets(str, 100, stdin);
					prompt_message = prompt(str);
					goto nextprompt;
			}
			else{
				prompt_message = prompt(args[1]);
				goto nextprompt;
			}
		}
		else
		if (strcmp(args[0], "kill") == 0) { //built-in command kill
			 
		 	printf("Executing built-in [kill]\n");
			if(args[1] == NULL){
				printf("[kill] not enough arguments");
				goto nextprompt;
				}
			if(args[2] == NULL){
				int p = *args[1] - '0';
				kill_s(p, SIGTERM);
				goto nextprompt;
				}
			else{
			  int p = *args[2] - '0';
			  int s = *args[1] - '0';
			  kill_s(p, s);
			  goto nextprompt;
			}
		}

		

		else
		  if(strcmp(args[0], "cd") == 0) { //built-in command cd
			if(args[1] == NULL){
				cd(getenv("HOME"));
				goto nextprompt;
			}
		  if(strcmp(args[1], "-") == 0){
			cd("./..");
		  }
		  else{cd(args[1]);}
		}
		else
                if (strcmp(args[0], "pwd") == 0) { // built-in command pwd 
		  printf("Executing built-in [pwd]\n");
	          ptr = getcwd(NULL, 0);
		  printf("%s\n", ptr);
		  free(ptr);
		}
                if (strcmp(args[0], "noclobber") == 0) { // built-in command noclobber
		  printf("Executing built-in [noclobber]\n");
                  noclobber = 1 - noclobber; // switch value
		  printf("%d\n", noclobber);
	        }
		else
                if (strcmp(args[0], "echo") == 0) { // built-in command echo
		  printf("Executing built-in [echo]\n");
		  for (i = 0; i < nargs +1; i++){
		    printf("arg[%d] = %s\n", i, args[i]);}
	        }
		else
                if (strcmp(args[0], "which") == 0) { // built-in command which
		  printf("Executing built-in [which]\n");

		  if (args[1] == NULL) {  // "empty" which
		    printf("which: Too few arguments.\n");
		    goto nextprompt;
                  }

		  dir_list = get_path();

                  cmd = which(args[1], dir_list);
                  if (cmd) {
		    printf("%s\n", cmd);
                    free(cmd);
                  }
		  else               // argument not found
		    printf("%s: Command not found\n", args[1]);

		  while (dir_list) {   // free list of path values
		     tmp = dir_list;
		     dir_list = dir_list->next;
		     free(tmp->element);
		     free(tmp);
                  }
	        } 
                // add other built-in commands here
		else
		if (strcmp(args[0], "where") == 0) {  //built-in command where
			printf("Executing built-in command [where].\n");
			if(args[1] == NULL){
				printf("where: Too few arguments.\n");
				goto nextprompt;
			}
			dir_list = get_path();
			cmd = where(args[1], dir_list);
			

			if(cmd){
				printf("%s\n", cmd);
			}
			else{
				printf("%s: Command not found\n", args[1]);}

			while(dir_list){
				tmp = dir_list;
				dir_list = dir_list->next;
				free(tmp->element);
				free(tmp);
			}
		}
		else
		if (strcmp(args[0], "pid") == 0) { //built-in command pid
			printf("Executing built-in [pid]\n");
			printf("%ld\n",(long)getpid());
		}

		else
		if (strcmp(args[0], "printenv") == 0) {
			printf("Executing built-in [printenv].\n");
			if(args[1] == NULL){
			printenv();
			goto nextprompt;
			}
			
			if(args[2] != NULL){
				printf("printenv: too many arguments.\n");
				goto nextprompt;
			}
			else{
				printf("%s\n", getenv(args[1]));
				goto nextprompt;
			}
		}
		else
		if (strcmp(args[0], "setenv") == 0) { //built-in command setenv
			printf("Executing built-in [setenv].\n");
			
			/*if(args[3] != NULL){
				printf("setenv: too many arguments.\n");
				goto nextprompt;
			}
			else{*/
				if(strcmp(args[1], "PATH") == 0){
					char *p = getenv("PATH");
					p = strcat(p, args[1]);
					setenv_s(p, args[2]);
					goto nextprompt;
				}
				if(args[2] == NULL){
					setenv_s(args[1], "");
					goto nextprompt;
				}
				else{
				setenv_s(args[1], args[2]);
				goto nextprompt;
				}
			//}

		}
		
		else if (strcmp(args[0], "watchuser") == 0) {
      			printf("Executing built-in command watchuser\n");
			printf("%s", args[1]);
     			if (args[1] == NULL) {
        			printf("Not enough arguments\n");
      			} 
			else if (args[1] != NULL) {
        			printf("Searching for user\n");
      			  	watchuser(args[1], 0, first_watchuser);
        			if (first_watchuser) {
          				first_watchuser = 0;
        			}
     			 } 
			 else if (strcmp(args[2], "off") == 0) {
       				if (strcmp(args[2], "off") == 0) {
          				if (!first_watchuser) {
            					watchuser(args[1], 1, first_watchuser);
          				}
       				}
			 }
			else {
          			printf("Invalid argument\n");
       			}
     		 	
		 	
    		}					
			




		if(strcmp(args[0], "cd") != 0 && strcmp(args[0], "exit") != 0 && strcmp(args[0], "printenv") != 0 && strcmp(args[0], "setenv") != 0 &&
		strcmp(args[0], "kill") != 0 && strcmp(args[0], "list") != 0 && strcmp(args[0], "watchuser") != 0 && strcmp(args[0], "pid") != 0 &&
		strcmp(args[0], "prompt") != 0 && strcmp(args[0], "noclobber") != 0 && strcmp(args[0], "echo") != 0 && strcmp(args[0], "which") != 0 &&
		strcmp(args[0], "where") != 0 && strcmp(args[0], "pwd") != 0){  // external command
		  if ((pid = fork()) < 0) {
			printf("fork error");
		  } else if (pid == 0) {		/* child */
			                // an array of aguments for execve()
	                char    *execargs[MAXARGS]; 
		        glob_t  paths;
                        int     csource, j;
			char    **p;

                        if (args[0][0] != '/' && strncmp(args[0], "./", 2) != 0 && strncmp(args[0], "../", 3) != 0) {  // get absoulte path of command
		          dir_list = get_path();      
                          cmd = which(args[0], dir_list);
                          if (cmd) 
		            printf("Executing [%s]\n", cmd);
		          else {              // argument not found
		            printf("%s: Command not found\n", args[1]);
			    goto nextprompt;
                          }

		          while (dir_list) {   // free list of path values
		             tmp = dir_list;
		             dir_list = dir_list->next;
		             free(tmp->element);
		             free(tmp);
                          }
			  execargs[0] = malloc(strlen(cmd)+1);
			  strcpy(execargs[0], cmd); // copy "absolute path"
			  free(cmd);
			}
			else {
			  execargs[0] = malloc(strlen(args[0])+1);
			  strcpy(execargs[0], args[0]); // copy "command"
                        }

		        j = 1;
		        for (i = 1; i < nargs; i++) { // check arguments
			  if (strchr(args[i], '*') != NULL) { // wildcard!
			    csource = glob(args[i], 0, NULL, &paths);
                            if (csource == 0) {
                              for (p = paths.gl_pathv; *p != NULL; ++p) {
                                execargs[j] = malloc(strlen(*p)+1);
				strcpy(execargs[j], *p);
				j++;
                              }
                              globfree(&paths);
                            }
			    else
                            if (csource == GLOB_NOMATCH) {
                              execargs[j] = malloc(strlen(args[i])+1);
			      strcpy(execargs[j], args[i]);
			      j++;
			    }
                          }
			  else {
                            execargs[j] = malloc(strlen(args[i])+1);
			    strcpy(execargs[j], args[i]);
			    j++;
			  }
                        }
                        execargs[j] = NULL;

			if (background) { // get rid of argument "&"
			  j--;
			  free(execargs[j]);
                          execargs[j] = NULL;
                        }

		        // print arguments into execve()
                     /***
		        for (i = 0; i < j; i++)
		          printf("exec args[%d] = %s\n", i, execargs[i]);
                     ***/

			execve(execargs[0], execargs, NULL);
			printf("couldn't execute: %s", buf);
			exit(127);
		  }

		  /* parent */
		  if (! background) { // foreground process
		    if ((pid = waitpid(pid, &status, 0)) < 0)
	     	      printf("waitpid error");
                  }
		  else {              // no waitpid if background
		    background = 0;
                  }
/**
                  if (WIFEXITED(status)) S&R p. 239 
                    printf("child terminates with (%d)\n", WEXITSTATUS(status));
**/
                }


           nextprompt:
		printf("%s", prompt_message);
		fprintf(stdout, " >>");
		fflush(stdout);
	}
	
	if(status==0){
		printf("Use 'exit' to leave shell.\n");
		goto nextprompt;
	}

	
	exit(0);
}

