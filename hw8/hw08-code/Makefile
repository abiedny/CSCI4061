CC = gcc -Wall -g

PROGRAMS = \
	create-subdir \
	nftw_listing \
	recursive_listing \

all : $(PROGRAMS)

%.o : %.c
	$(CC) -c $<

create-subdir :
	chmod u+x make-subdir-small.sh
	./make-subdir-small.sh

nftw_listing : nftw_listing.o print_file_info.o
	$(CC) -o $@ $^

recursive_listing : recursive_listing.o print_file_info.o
	$(CC) -o $@ $^

qsort_demo : qsort_demo.c
	$(CC) -o $@ $^

clean :
	rm -f *.o $(PROGRAMS)
