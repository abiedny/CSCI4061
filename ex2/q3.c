// printqueue_add.c: adds the file name
// specified on the command line to the
// print queue by appending to the end
// of the file.
#include "headers.h"
int main(int argc, char *argv[]){
  //initializing and creating semaphore, should be done before program is used normally
  if(strcmp(argv[1],"-init")==0){
    sem_t *sem = sem_open("/printqueue_sem", O_CREAT, S_IRUSR | S_IWUSR);
    sem_init(sem, 1, 1);
    sem_close(sem);
    return 0;
  }
  if(argc < 3){
    printf("usage: <printqueue> <file>\n");
    return 1;
  }

  char *printqueue = argv[1];
  char *filename = argv[2];

  sem_t *sem = sem_open(sem_name, O_CREAT, S_IRUSR | S_IWUSR); //open semaphore

  char buf[1024];
  sprintf(buf,"* %s\n",filename);
  int len = strlen(buf);
  
  int fd = open(printqueue,
                O_CREAT|O_RDWR ,
                S_IRUSR|S_IWUSR);

  sem_wait(sem); //wait your turn to call lseek to find end and then write to file.

  lseek(fd, 0, SEEK_END);
  int n = strlen(buf); //variable word was never declared, assuming it must meant to be buf
  buf[n] = '\n';
  write(fd, buf,n+1);

  sem_post(sem); //done writing, others can access file now

  sem_close(sem); //cleanup by closing the semaphore
  close(fd);
  return 0;
}