#include "get_path.h"

char *where(char *command, struct pathelement *p)
{
  char cmd[128], *ch;
  int  found;
  ch = malloc(128*7);
  found = 0;
  again: 
  while (p) {       
    sprintf(cmd, "%s/%s", p->element, command);
    if (access(cmd, X_OK) == 0) {
     	p = p->next;
	 goto f;
      
    }
    p = p->next;
  }
  if (found) {
    f:
   // ch = malloc(strlen(cmd)*7);
    strcat(ch, "\n");
    strcat(ch, cmd);
    goto again;
  }
  return ch;
}

