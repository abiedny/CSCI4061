CFLAGS = -g
CC     = gcc $(CFLAGS)


PROGRAMS = \
	runner_nosem \
	runner_sem1 \
	runner_sem2 \

all : $(PROGRAMS)

%.o : %.c
	$(CC) -c $<


runner_nosem : runner_nosem.o mmap_file.o
	$(CC) -o $@ $^ -lpthread

runner_sem1 : runner_sem1.o mmap_file.o
	$(CC) -o $@ $^ -lpthread

runner_sem2 : runner_sem2.o mmap_file.o
	$(CC) -o $@ $^ -lpthread



clean:
	rm -f *.o $(PROGRAMS)
