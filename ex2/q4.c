// printqueue_print.c: send jobs to a printer.
// Run one process for each printer.
#include "headers.h"
int main(int argc, char *argv[]) {
    //initializing and creating semaphore, should be done before program is used normally
  if(strcmp(argv[1],"-init")==0){
    sem_t *sem = sem_open("/printqueue_sem", O_CREAT, S_IRUSR | S_IWUSR);
    sem_init(sem, 1, 1);
    sem_close(sem);
    return 0;
  }
  if(argc < 2){
    printf("usage: %s <printqueue.txt>\n",
           argv[0]);
    exit(1);
  }

  size_t size;
  char *file_chars =
    mmap_file(argv[1],&size);
  
  int line_num = 1;
  int file_pos = 0;
  sem_t *sem = sem_open(sem_name, O_CREAT, S_IRUSR | S_IWUSR); //open semaphore

  while(file_pos < size){
    
    sem_wait(sem); //wait until we can read the file without anyone modifying it

    char status, filename[MAXLINE];
    sscanf(file_chars+file_pos,
           "%c %1024[^\n]",   
           &status, filename);

    if(status == '*'){
      file_chars[file_pos] = 'P';
      sem_post(sem); //Unlock the file now that we have "claimed" this print job by changing its status
      send_to_printer(filename,MY_PRINTER);
      file_chars[file_pos] = 'D';
    }
    else{
      sem_post(sem); //unlock file
    }
    file_pos += strlen(filename)+3;
    line_num++;
  }

  sem_close(sem); //cleanup by closing the semaphore
  return 0;
}