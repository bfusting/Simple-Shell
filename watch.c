#include "watch.h"

pthread_mutex_t lock_user;
struct  Node *userhead;
pthread_t user_id;

void insert(char *name, pthread_t id, char *type){
	struct Node *node1;
	node1 = (struct Node *) malloc(strlen(name) +1);
	strcpy(node1->name, name);
	
	node1->on = 0;
	node1->tty = NULL;
	node1->host = NULL;
	node1->id = id;
	node1->next = NULL;

	if(strcmp(type, "user") == 0) {
		if(userhead == NULL) {
			userhead = node1;
		} else {
			struct Node *temp = userhead;
			while(temp->next){
				temp = temp->next;
			}
			temp->next = node1;
		}
	}
}

void delete(char *name, char *type){
	struct Node *temp, *prev;
	if(strcmp(type, "user") == 0){
		if(userhead == NULL) {
			return;
		}
	}
	while(temp != NULL) {
		if(strcmp(temp->name, name) == 0) {
			if(temp->id != (pthread_t)NULL) {
				if(pthread_cancel(temp->id)!=0) {
					perror("pthread_cancel() error");
				}
				if(pthread_join(temp->id, NULL) != 0) {
					perror("pthread_join() error");
					exit(4);
				}
			}
			if(strcmp(type, "user") == 0) {
				if((temp == userhead) && (userhead->next = NULL)) {
					free(temp->name);
					if(temp->tty != NULL) {
						free(temp->tty);
						free(temp->host);
					}	
					free(temp);
					userhead = NULL;
				}
				else if(temp == userhead) {
					userhead = temp->next;
					free(temp->name);
					if(temp->tty != NULL) {
						free(temp->tty);
						free(temp->host);
					}
					free(temp);
				} else{
					prev->next = temp->next;
					free(temp->name);
					if(temp->tty != NULL){
						free(temp->tty);
						free(temp->host);
					}
					free(temp);
				}
			}
			break;
		}
		prev = temp;
		temp = temp->next;
	}
}

void freeList(char *type) {
	struct Node *temp;
	if(strcmp(type, "user") == 0) {
		while(userhead != NULL) {
			temp = userhead;
			userhead = userhead->next;
			free(temp->name);
			if(temp->tty != NULL){
				free(temp->tty);
				free(temp->host);
			}
			free(temp);
		}
	}
}

int watchuser(char *name, int off, int first) {
	pthread_mutex_init(&lock_user, NULL);
	if(strcmp(name, "exit") == 0) {
		if(pthread_cancel(user_id)!=0){
			perror("pthread_cancel() error");
		}
		if(pthread_join(user_id, NULL) != 0) {
			perror("pthread_join() error");
			exit(4);
		}
		freeList("user");
	} else if (first) {
		insert(name, (pthread_t)NULL, "user");
		if(pthread_create(&user_id, NULL, threaduser, NULL) != 0) {
			perror("pthread_create() error");
			exit(1);
		}
	} else if(off){delete(name, "user");}
	else{
		struct Node *temp = userhead;
		int unique = 1;
		while(temp != NULL){
			if(strcmp(name, temp->name) == 0){
				unique = 0;
				break;
			}
			temp=temp->next;
		}
		if(unique){
			insert(name, (pthread_t)NULL, "user");
		}
	}
	return 0;
}

void *threaduser(void *some){
	while(1){
		pthread_mutex_lock(&lock_user);
		if(userhead != NULL){
			struct Node *temp;
			temp = userhead;
			while(temp != NULL){
				struct utmpx *up;
				setutxent();
				int found = 0;
				while(up = getutxent()){
					if(up->ut_type == USER_PROCESS){
						if(temp->on){
							if(strcmp(temp->name,up->ut_user) == 0){
								printf("\n%s has logged on %s from %s\n", up->ut_user,up->ut_line,up->ut_host);
								temp->tty = (char *) malloc(strlen(up->ut_line) +1);
								strcpy(temp->tty, up->ut_line);
								temp->on = 1;
								found = 1;
								break;
							}
						}
					}
				}
				if(temp->on && !found){
					printf("\n%s has logged off %s from %s\n", temp->name, temp->tty, temp->host);
					free(temp->tty);
					temp->tty = NULL;
					free(temp->host);
					temp->host = NULL;
					temp->on = 0;
				}
				temp = temp->next;
			}
			pthread_mutex_unlock(&lock_user);
		}
		sleep(20);
	}
	return NULL;
}
