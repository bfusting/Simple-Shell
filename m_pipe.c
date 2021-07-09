#include "m_pipe.h"

void open_pipe(char **right_argv, char **left_argv, char err){
	int pipefd[2];
	pipe(pipefd);
	int pid;
	if ((pid = fork()) < 0) {
		perror("fork error(1)");
		exit(0);
	}
	else if (pid > 0) {
		if(waitpid(pid, NULL, 0) < 0) {
			perror("Waitpid error");
			exit(1);
		}
		if ((pid = fork()) < 0) {
			perror("fork error");
			exit(1);
		}
		else if(pid > 0) {
			if(waitpid(pid, NULL, 0) < 0) {
				perror("waitpid error");
				exit(1);
			}
			return;
		}
		else if(pid == 0){
			close(pipefd[1]);
			close(0);
			dup2(pipefd[0], 0);
			if(execvp(right_argv[0], right_argv) < 0){
				execv(right_argv[0], right_argv);
			}
			exit(0);
		}
	}
	else if(pid == 0) {
		close(1);
		if(err){close(2);}
		close(pipefd[0]);
		dup2(pipefd[1], 1);
		if(err){dup2(pipefd[1], 2);}
		if(execvp(left_argv[0], left_argv) < 0){
			execv(left_argv[0], left_argv);
		}
		exit(0);
	}
}
