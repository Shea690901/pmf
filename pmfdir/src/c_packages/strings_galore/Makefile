# Makefile for the strings_galore package
# Thomas Padron-McCarthy (padrone@lysator.liu.se), 1991
# This file latest updated: Sept 22, 1991

# Here is a rule for making .o files from .c files that doesn't force
# the type of the machine (like -sun3) into the flags.
# And best of all: it works on Nanny's wrong-installed make!
.c.o:
	$(CC) -c $(CFLAGS) $(INCLUDEDIRS) $(CPPFLAGS) $*.c

CFILES = copy_string.c cases.c build_string.c bcs.c \
	make_path.c add_strings.c \
	test_bcs.c
OBJECTS = copy_string.o cases.o build_string.o bcs.o \
	make_path.o add_strings.o
HFILES = str_galore.h sg_config.h
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
	rm -f *.o core *~ *.bak test_bcs

depend:
	makedepend -w1 $(INCLUDEDIRS) $(CFILES)

test_bcs: test_bcs.o bcs.o
	cc -o test_bcs -g test_bcs.o \
		bcs.o \
		../safe_malloc/safe_malloc.o ../safe_malloc/fatal.o

# DO NOT DELETE THIS LINE -- make depend depends on it.

copy_string.o: ../safe_malloc/safe_malloc.h
copy_string.o: sg_config.h
copy_string.o: str_galore.h
cases.o: ../safe_malloc/safe_malloc.h
cases.o: sg_config.h
cases.o: str_galore.h
build_string.o: ../safe_malloc/safe_malloc.h
build_string.o: sg_config.h
build_string.o: str_galore.h
bcs.o: ../safe_malloc/safe_malloc.h
bcs.o: sg_config.h
bcs.o: str_galore.h
make_path.o: ../safe_malloc/safe_malloc.h
make_path.o: sg_config.h
make_path.o: str_galore.h
add_strings.o: ../safe_malloc/safe_malloc.h
add_strings.o: sg_config.h
add_strings.o: str_galore.h
test_bcs.o: str_galore.h
