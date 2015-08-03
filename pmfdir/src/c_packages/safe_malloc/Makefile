# Makefile for the generic hash table package
# Thomas Padron-McCarthy (padrone@lysator.liu.se), 1991
# This file latest updated: Sept 21, 1991

# Here is a rule for making .o files from .c files that doesn't force
# the type of the machine (like -sun3) into the flags.
# And best of all: it works on Nanny's wrong-installed make!
.c.o:
	$(CC) -c $(CFLAGS) $(INCLUDEDIRS) $(CPPFLAGS) $*.c

CFILES = safe_malloc.c fatal.c
OBJECTS = safe_malloc.o fatal.o
HFILES = safe_malloc.h
INCLUDEDIRS =

# CC = gcc -traditional
CC = cc

# Distribution: CFLAGS = -O
# Maximum debug: CFLAGS = -g -pg -DDEBUG -pipe
CFLAGS = -O

all: $(OBJECTS)

lint:
	lint $(INCLUDEDIRS) $(CFILES)

clean:
	rm -f *.o core *~ *.bak

depend:
	makedepend -w1 $(INCLUDEDIRS) $(CFILES)

# DO NOT DELETE THIS LINE -- make depend depends on it.

safe_malloc.o: safe_malloc.h
fatal.o: safe_malloc.h
