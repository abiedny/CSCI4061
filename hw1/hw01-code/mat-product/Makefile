CFLAGS = -Wall -g
CC     = gcc $(CFLAGS)

op_main : op_main.o outer_product.o free_matrix.o mat.h
	$(CC) -o op_main op_main.o outer_product.o free_matrix.o
	@echo op_main built!

op_main.o : op_main.c mat.h
	$(CC) -c op_main.c

outer_product.o : outer_product.c mat.h
	$(CC) -c outer_product.c

free_matrix.o : free_matrix.c mat.h
	$(CC) -c free_matrix.c

test : op_main
	@chmod u+x testy
	./testy test_outer_product.org $(testnum)

clean:
	@echo Cleaning...
	rm -f *.o op_main