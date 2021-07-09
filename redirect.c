#include "redirect.h"

int const REDIR_OW = 1;
int const REDIR_ER=2;
int const REDIR_AP=4;

void change_out(char *file, int closed, int flags){
	int fid = open(file, flags, 0666);
	if(fid < 0){
		perror("open error");
	}
	close(closed);
	dup(fid);
	close(fid);
}

void redirect(char ** cmd_argv, char *filename, int options){
	pid_t pid;
	int foptions = O_CREAT | O_RDWR;


	if(options & REDIR_AP)
		foptions |= O_APPEND;
	
	if(!(options & REDIR_OW) && !access(filename, F_OK)) {
		char ans, c;
		printf("%s: File exits.\n", filename);
		return;
	}
	if(!access(filename, F_OK) && !(options & REDIR_AP)) {
		foptions |= O_TRUNC;
	}
	if((pid = fork()) < 0) {
		perror("waitpid error");
		exit(0);
	}
	else if (pid > 0) {
		if(waitpid(pid, NULL, 0) < 0){
			perror("waitpid error");
			exit(1);
		}
	}
	else if(pid == 0) {
		change_out(filename, 1, foptions|O_APPEND);
		if(options & REDIR_ER)
			change_out(filename, 2, foptions);
		if(execvp(cmd_argv[0], cmd_argv) == -1) {
			execv(cmd_argv[0],cmd_argv);
		}
		exit(0);
	}
	if(options & REDIR_ER)
		change_out("/dev/tty", 2, O_WRONLY);
	change_out("/dev/tty", 1, O_WRONLY);
}
