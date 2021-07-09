#include <dirent.h>
#include <stdio.h>

void list(char* path){
	DIR *d;
	struct dirent *dir;
	d = opendir(path);
	if(d) {
		while((dir = readdir(d)) != NULL) {
			printf("%s\n", dir->d_name);
		}
		closedir(d);
	}
}






