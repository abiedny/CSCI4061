// run_child.c: run a child process. This version needs fixin'.
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

int main(void){
  char *child_argv[] = {"","--version",NULL,NULL};
  char *child_cmd = "gcc";
  printf("Running command '%s'\n",child_cmd);
  printf("------------------\n");

  pid_t child = fork();
  if (child == 0) {
    execvp(child_cmd,child_argv);
  }
  int status;
  wait(&status);
  printf("------------------\n");
  printf("Child Finished\n");         // show that the child process has finished
  return 0;
}
