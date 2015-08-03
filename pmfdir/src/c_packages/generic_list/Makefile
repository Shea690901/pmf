# Makefile for the generic lists, sets and fifos package
# Thomas Padron-McCarthy (padrone@lysator.liu.se), 1991
# This file latest updated: Sept 21, 1991

# Here is a rule for making .o files from .c files that doesn't force
# the type of the machine (like -sun3) into the flags.
# And best of all: it works on Nanny's wrong-installed make!
.c.o:
	$(CC) -c $(CFLAGS) $(INCLUDEDIRS) $(CPPFLAGS) $*.c

CFILES = generic_list.c generic_set.c generic_fifo.c test_lists.c
OBJECTS = generic_list.o generic_set.o generic_fifo.o
HFILES = generic_list.h generic_set.h generic_fifo.h
INCLUDEDIRS = -I../safe_malloc

# CC = gcc -traditional
CC = cc

# Distribution: CFLAGS = -O
# Maximum debug: CFLAGS = -g -pg -DDEBUG -pipe
CFLAGS = -O

all: $(OBJECTS)

lib: libgeneric_list.a

libgeneric_list.a: $(OBJECTS)
	ar rv $@ $?
	ranlib $@

lint:
	lint $(INCLUDEDIRS) $(CFILES)

clean:
	rm -f *.o core *~ *.bak libgeneric_list.a test_lists

depend:
	makedepend -w1 $(INCLUDEDIRS) $(CFILES)

test_lists: test_lists.o $(OBJECTS)
	cc -o test_lists -g test_lists.o \
		$(OBJECTS) \
		../safe_malloc/safe_malloc.o ../safe_malloc/fatal.o

# DO NOT DELETE THIS LINE -- make depend depends on it.

generic_list.o: generic_list.h
generic_list.o: ../safe_malloc/safe_malloc.h
generic_set.o: generic_set.h
generic_set.o: generic_list.h
generic_set.o: ../safe_malloc/safe_malloc.h
generic_fifo.o: generic_fifo.h
generic_fifo.o: generic_list.h
generic_fifo.o: ../safe_malloc/safe_malloc.h
test_lists.o: ../safe_malloc/safe_malloc.h
test_lists.o: generic_list.h
test_lists.o: generic_set.h
test_lists.o: generic_fifo.h
