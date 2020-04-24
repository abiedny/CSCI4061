// proc_v_thread.c: compares behavior of child process vs thread wrt
// to data in parent. Compile and run via:
//
// > gcc -o proc_v_thread proc_v_thread.c -lpthread
// > ./a.out proc
// ...
// > ./a.out thread
// ...
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <ctype.h>
#include <pthread.h>
#include <string.h>
#include <wait.h>

int the_int = 0;
void *inc_the_int(void *null){                  // function that increments
  the_int++;                                    // global var the_int
  printf("Increment finished, press enter to continue\n");
  getchar();
}

int main(int argc, char *argv[]){
  if(argc < 2){
    printf("usage: %s {proc|thread}\n",argv[0]);
    return 1;
  }

  char *mode = argv[1];
  if(0){}
  else if( strcmp(mode, "proc")==0 ){           // run a child process
    printf("Child PROCESS running\n");
    pid_t pid = fork();                         // fork a child process
    if(pid == 0){                               
      inc_the_int(NULL);                        // child process increments
      exit(0);                                  // then exits
    }
    waitpid(pid, NULL, 0);                      // parent waits for child
    printf("Child PROCESS complete\n");
  }

  else if( strcmp(mode, "thread")==0 ){         // run a "child" thread
    printf("Child THREAD running\n");
    pthread_t child_thread;
    pthread_create(&child_thread, NULL,         // start child thread
                   inc_the_int, NULL);          // running inc_the_int()
    pthread_join(child_thread, NULL);           // wait for child to complete function
    printf("Child THREAD complete\n");
  }

  else{                                         // error case
    printf("unknown mode: '%s'\n", mode);
    exit(1);
  }
  
  printf("Value of the_int: %d\n",the_int);   // report what the "parent" sees
  return 0;
}
