// runner_sem1.c: like runner_nosem.c but uses semaphore sem_wait()
// and sem_post() calls to protect the critical region.
#include "headers.h"

int main(int argc, char *argv[]) {
  if(argc < 2){
    printf("usage: %s <jobfile.txt>\n",argv[0]);
    exit(1);
  }

  char sem_name[MAXLINE];
  sprintf(sem_name,"/runner_lock_%s",getenv("USER")); 

  if(strcmp(argv[1],"-init")==0){             // initialize the semaphore and exit
    sem_t *sem = sem_open(sem_name, O_CREAT, S_IRUSR | S_IWUSR);
    sem_init(sem, 1, 1);
    sem_close(sem);
    system("mkdir -p out");
    return 0;
  }

  size_t size;
  char *file_chars =
    mmap_file(argv[1],&size);                // call utility function to mmap() entire job file

  sem_t *sem = sem_open(sem_name, O_CREAT, S_IRUSR | S_IWUSR);

  int line_num = 1;
  int file_pos = 0;
  int jobs_run = 0;
  while(file_pos < size){                    // loop to end of file
    sem_wait(sem);                           // LOCK semaphore

    char status, command[MAXLINE];           // holds first char and remainder of line
    sscanf(file_chars+file_pos,              // snaky bit of code to parse one
           "%c %1024[^\n]",                  // line from the mmap()'d file 
           &status, command);                // into status/command

    if(status == '-'){                       // if first char is -, job hasn't been run yet
      file_chars[file_pos] = 'R';            // mark as being run
      sem_post(sem);                         // UNLOCK semaphore
      printf("%03d: %d RUN '%s'\n",line_num,getpid(),command);
      fflush(stdout);
      char call[1024];                        // form the command to run
      sprintf(call,"%s > /dev/null",command); // redirect output so it isn't shown
      system(call);                           // run the command 
      file_chars[file_pos] = 'D';             // mark job as done
    }
    else{
      sem_post(sem);                          // UNLOCK semaphore
    }
    file_pos += strlen(command)+3;            // move to next line by advancing file position
    line_num++;
  }
  sem_close(sem);
  return 0;
}
