# Makefile for the copy_bytes package
# Thomas Padron-McCarthy (padrone@lysator.liu.se), 1991
# This file latest updated: Sept 21, 1991

# Here is a rule for making .o files from .c files that doesn't force
# the type of the machine (like -sun3) into the flags.
# And best of all: it works on Nanny's wrong-installed make!
.c.o:
	$(CC) -c $(CFLAGS) $(INCLUDEDIRS) $(CPPFLAGS) $*.c

CFILES = copy_bytes.c
OBJECTS = copy_bytes.o
HFILES = copy_bytes.h bs_config.h
INCLUDEDIRS = -I../safe_malloc

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

copy_bytes.o: copy_bytes.h
