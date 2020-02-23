// parent_listen.c: demonstrate use of pipe() to allow communication
// between child and parent proceses.
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>
#include <string.h>

#define PREAD 0                                       // index of read end of pipe
#define PWRITE 1                                      // index of write end of pipe
#define BUFSIZE 1024

int main(int argc, char *argv[]) {

  int par_child_pipe[2];                              // file descriptors for read/write ends of pipe
  int pipe_result = pipe(par_child_pipe);             // create a pipe
  if(pipe_result != 0) {                              // check that pipe creation succeeded
    perror("Failed to create pipe");                  // report errors if pipe creation failed
    exit(1);
  }

  printf("0. Parent creating child process\n");
  pid_t child_pid = fork();                           // create a child process
  if(child_pid < 0){                                  // check if fork failed
    perror("Failed to fork");                         // report errors if forking failed
    exit(1);
  }
                                                      // CHILD CODE
  if(child_pid == 0){
    char *msg = "Send $$$ please!";
    int msg_len = strlen(msg)+1;               
    int bytes_written =                               // capture how many bytes were written
      write(par_child_pipe[PWRITE], msg, msg_len);    // write into write end of the pipe
    printf("1. Child wrote %d bytes\n",bytes_written);
    fflush(stdout);                                   // ensure that screen messages are printed

    close(par_child_pipe[PWRITE]);                    // in child, close both ends of the pipe
    close(par_child_pipe[PREAD]);
    exit(0);                                          // child exits
  }

  // PARENT CODE
  char buffer[BUFSIZE];                               // copy message into this array of chars
  int bytes_read =                                    // read data from read end of pipe into buffer
    read(par_child_pipe[PREAD], buffer, BUFSIZE);     // capture number of bytes read
  close(par_child_pipe[PWRITE]);                      // in parent, close both ends of the pipe
  close(par_child_pipe[PREAD]);
  wait(NULL);
  printf("2. Parent read %d bytes\n",bytes_read);
  printf("3. Child said: '%s'\n",buffer);                                         // wait for the child to complete

  return 0;
}
