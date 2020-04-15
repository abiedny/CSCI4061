// runner_nosem.c: works through a list of 'jobs' in a file; runs them
// as shell commands via the system() convenience function. Uses
// mmap() to read from the file which has the first character on each
// line indicating the status of each job
// 
// * 'D' means the job is Done running
// * 'R' means the job is Running
// * '-' means the job has not been run yet
// 
// EXAMPLE JOBS FILE:
// D seq 100000
// D gcc --version
// R du . -h
// R ls
// - ls -l
// - ls -la
//
// Since the file is memory mapped, it can be shared with several
// instances of runners allowing for parallel execution. Try the
// following to watch how several jobs can be handled by parallel
// runners.
//
// shell1> make
// shell1> cp jobs.txt.1 jobs.txt
// shell1> watch -n 0.1 'cat jobs.txt'
//
// shell2> for i in $(seq 3); do ./runner_nosem jobs.txt & done

#include "headers.h"

int main(int argc, char *argv[]) {
  if(argc < 2){
    printf("usage: %s <jobfile.txt>\n",argv[0]);
    exit(1);
  }

  if(strcmp(argv[1],"-init")==0){            // initialize the output directory and exit
    system("mkdir -p out");
    return 0;
  }

  size_t size;
  char *file_chars =
    mmap_file(argv[1],&size);                // call utility function to mmap() entire job file

  int line_num = 1;
  int file_pos = 0;
  while(file_pos < size){
    char command[MAXLINE];
    char status;
    sscanf(file_chars+file_pos,              // snaky bit of code to parse one
           "%c %1024[^\n]",                  // line from the mmap()'d file 
           &status, command);                // into status/command

    if(status == '-'){                       // if first char is -, job hasn't been run yet
      file_chars[file_pos] = 'R';            // mark as being run
      printf("%03d: %d RUN '%s'\n",line_num,getpid(),command);
      fflush(stdout);
      char call[1024];                        // form the command to run
      sprintf(call,"%s > /dev/null",command); // redirect output so it isn't shown
      system(call);                           // run the command 
      file_chars[file_pos] = 'D';             // mark job as done
    }
    file_pos += strlen(command)+3;            // move to next line by advancing file position
    line_num++;
  }

  return 0;
}
