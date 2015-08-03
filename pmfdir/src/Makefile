#
# Makefile for pmf -- Padrone's MudFrontend
# Thomas Padron-McCarthy (padrone@lysator.liu.se), 1990, 1991
# This file latest updated: May 23, 1993
#
# Instructions for compiling (but read the file "pmf.doc" too):
#
#	Usually it should be enough just to edit "config.h",
#	and then type "make", but if you want to install pmf
#	in a non-standard way (i e no GNU readline or no sounds),
#	you'll have to change the variables OPTIONS, SOUND_OBJECT
#	and HISTORY_OBJECT below. Then you can type "make".
#
#	If the linker complains, saying it cannot find usleep,
#	define the variable USLEEP_OBJECT below as usleep.o.
#	If it still doesn't work, try usleep2.o instead of usleep.o.
#
#	If the linker complains, saying it cannot find alloca,
#	define the variable ALLOCA_OBJECT below as alloc.o.
#
#	On IBM RT's you might have to add the option -ma to OPTIONS.
#
#	You might have to link without -lresolv. Pmf will still work,
#	but you will only be able to use numerical host addresses.
#
######################################################################

# This variable, OPTIONS, should be changed if you want to compile
# pmf without GNU readline or without sound.
# Remove one (or both) of "-DGNU_READLINE" and "-DSOUND".
#
OPTIONS = -DGNU_READLINE -DSOUND

# This variable, SOUND_OBJECT, should be changed if you want to
# compile pmf without sound: remove this definition of SOUND_OBJECT.
#
SOUND_OBJECT = sound.o

# This variable, HISTORY_OBJECT, should be changed if you want to
# compile pmf without GNU readline: define HISTORY_OBJECT as
# history.o instead of rl_history.o.
#
HISTORY_OBJECT = rl_history.o
# HISTORY_OBJECT = history.o

# If it doesn't work otherwise: try defining USLEEP_OBJECT here:
#
# USLEEP_OBJECT = usleep.o
# USLEEP_OBJECT = usleep2.o
# USLEEP_OBJECT = usleep3.o

# If it doesn't work otherwise: try defining ALLOCA_OBJECT here:
#
# ALLOCA_OBJECT = alloca.o

# If you want to use the GNU C compiler,
# change the variable CC to "gcc -traditional" instead:
#
CC = cc
# CC = gcc -traditional

######################################################################

# This applies only here
#	-- the Makefiles in the subdirs have to be changed separately.
# Distribution: CFLAGS = -O
# Maximum debug: CFLAGS = -g -pg -DDEBUG -pipe
CFLAGS = -O

######################################################################

# Here is a rule for making .o files from .c files that doesn't force
# the type of the machine (like -sun3) into the flags.
# And best of all: it works on Nanny's wrong-installed make!
#
.c.o:
	$(CC) -c $(CFLAGS) $(OPTIONS) $(INCLUDEDIRS) $(CPPFLAGS) $*.c

######################################################################

BASE_OBJECTS = main.o ipc.o use_ipc.o \
	alias.o error.o get_input.o do_commands.o \
	misc.o display.o \
	signals.o help.o tty.o variables.o robot.o \
	word_split.o line_split.o globals.o apply.o \
	handle_cmd.o dollar_match.o gag.o cryptsay.o putget.o xmode.o \
	compile_time.o

OBJECTS = $(BASE_OBJECTS) $(SOUND_OBJECT) $(HISTORY_OBJECT) \
	$(USLEEP_OBJECT) $(ALLOCA_OBJECT)

HFILES = pmf.h config.h globals.h
CFILES = alias.c apply.c do_commands.c \
	dollar_match.c error.c \
	get_input.c globals.c \
	handle_cmd.c help.c ipc.c main.c misc.c display.c robot.c sound.c \
	signals.c word_split.c tty.c use_ipc.c variables.c usleep.c \
	usleep2.c usleep3.c line_split.c gag.c cryptsay.c putget.c xmode.c \
	history.c rl_history.c \
	alloca.c

READLINE_DIR = readline
C_PACKAGES_DIR = c_packages
READLINE_LIB = $(READLINE_DIR)/libreadline.a
C_PACKAGES_LIB = $(C_PACKAGES_DIR)/libc_packs.a

LIBS = $(READLINE_LIB) $(C_PACKAGES_LIB)

INCLUDEDIRS = -I$(C_PACKAGES_DIR)/safe_malloc \
	-I$(C_PACKAGES_DIR)/generic_list \
	-I$(C_PACKAGES_DIR)/strings_galore

SOURCES = $(CFILES) $(HFILES) Makefile \
	$(READLINE_FILES) $(C_PACKAGES_FILES)
DOCUMENTATION = doc
SYSTEM_FILES = system_dir/.pmfrc.default system_dir/NEWS \
	system_dir/helpfiles/* system_dir/soundfiles
EXTRAS = FILES NeXT examples

######################################################################

all: pmf

pmf: $(OBJECTS) $(LIBS)
	rm -f compile_time.c
	$(CC) -o pmf $(CFLAGS) $(OBJECTS) $(LIBS) -ltermcap -lresolv
	strip pmf
	rm -f compile_time.o

compile_time.c:
	echo 'char compile_time[] = "'`date`'";' > compile_time.c

######################################################################

$(READLINE_LIB):
	( cd $(READLINE_DIR); make )

$(C_PACKAGES_LIB):
	( cd $(C_PACKAGES_DIR); make )

READLINE_FILES = \
	$(READLINE_DIR)/*

C_PACKAGES_FILES = \
	$(C_PACKAGES_DIR)/Makefile \
	$(C_PACKAGES_DIR)/*/Makefile $(C_PACKAGES_DIR)/*/*.h $(C_PACKAGES_DIR)/*/*.c

######################################################################

lint:
	lint $(OPTIONS) $(INCLUDEDIRS) $(CFILES) > lint.out

tags: $(CFILES) $(HFILES)
	etags -t $(CFILES) $(HFILES)

clean:
	rm -f *.o core *~ *.bak pmf
#	(cd $(READLINE_DIR); make clean)
#	(cd $(C_PACKAGES_DIR); make clean)

depend:
	makedepend -w1 $(OPTIONS) $(INCLUDEDIRS) $(CFILES)

######################################################################

# DO NOT DELETE THIS LINE -- make depend depends on it.

alias.o: c_packages/safe_malloc/safe_malloc.h
alias.o: c_packages/strings_galore/str_galore.h
alias.o: config.h
alias.o: pmf.h
alias.o: globals.h
alias.o: c_packages/generic_list/generic_set.h
alias.o: c_packages/generic_list/generic_list.h
apply.o: c_packages/safe_malloc/safe_malloc.h
apply.o: config.h
apply.o: pmf.h
apply.o: globals.h
do_commands.o: c_packages/safe_malloc/safe_malloc.h
do_commands.o: config.h
do_commands.o: pmf.h
do_commands.o: globals.h
dollar_match.o: pmf.h
dollar_match.o: config.h
dollar_match.o: globals.h
error.o: config.h
error.o: pmf.h
get_input.o: config.h
get_input.o: pmf.h
get_input.o: globals.h
globals.o: config.h
globals.o: pmf.h
handle_cmd.o: c_packages/safe_malloc/safe_malloc.h
handle_cmd.o: config.h
handle_cmd.o: pmf.h
handle_cmd.o: globals.h
help.o: c_packages/safe_malloc/safe_malloc.h
help.o: c_packages/strings_galore/str_galore.h
help.o: config.h
help.o: pmf.h
help.o: globals.h
ipc.o: c_packages/safe_malloc/safe_malloc.h
ipc.o: pmf.h
ipc.o: config.h
ipc.o: globals.h
main.o: c_packages/safe_malloc/safe_malloc.h
main.o: c_packages/strings_galore/str_galore.h
main.o: config.h
main.o: pmf.h
main.o: globals.h
misc.o: pmf.h
misc.o: config.h
misc.o: globals.h
display.o: c_packages/safe_malloc/safe_malloc.h
display.o: pmf.h
display.o: config.h
display.o: globals.h
display.o: c_packages/generic_list/generic_list.h
display.o: c_packages/generic_list/generic_fifo.h
robot.o: c_packages/safe_malloc/safe_malloc.h
robot.o: c_packages/strings_galore/str_galore.h
robot.o: config.h
robot.o: pmf.h
robot.o: globals.h
robot.o: c_packages/generic_list/generic_set.h
robot.o: c_packages/generic_list/generic_list.h
sound.o: c_packages/safe_malloc/safe_malloc.h
sound.o: c_packages/strings_galore/str_galore.h
sound.o: config.h
sound.o: pmf.h
sound.o: globals.h
sound.o: c_packages/generic_list/generic_set.h
sound.o: c_packages/generic_list/generic_list.h
signals.o: config.h
signals.o: pmf.h
word_split.o: pmf.h
tty.o: config.h
tty.o: pmf.h
use_ipc.o: c_packages/safe_malloc/safe_malloc.h
use_ipc.o: c_packages/strings_galore/str_galore.h
use_ipc.o: pmf.h
use_ipc.o: config.h
use_ipc.o: globals.h
use_ipc.o: c_packages/generic_list/generic_fifo.h
use_ipc.o: c_packages/generic_list/generic_list.h
variables.o: config.h
variables.o: c_packages/safe_malloc/safe_malloc.h
variables.o: pmf.h
variables.o: globals.h
usleep2.o: usleep.c
alloca.o: alloca.c
line_split.o: pmf.h
gag.o: c_packages/safe_malloc/safe_malloc.h
gag.o: c_packages/strings_galore/str_galore.h
gag.o: config.h
gag.o: pmf.h
gag.o: globals.h
gag.o: c_packages/generic_list/generic_set.h
gag.o: c_packages/generic_list/generic_list.h
cryptsay.o: c_packages/safe_malloc/safe_malloc.h
cryptsay.o: config.h
cryptsay.o: pmf.h
cryptsay.o: globals.h
putget.o: c_packages/safe_malloc/safe_malloc.h
putget.o: config.h
putget.o: pmf.h
putget.o: globals.h
xmode.o: pmf.h
xmode.o: config.h
xmode.o: globals.h
history.o: c_packages/safe_malloc/safe_malloc.h
history.o: config.h
history.o: pmf.h
history.o: globals.h
rl_history.o: c_packages/safe_malloc/safe_malloc.h
rl_history.o: config.h
rl_history.o: pmf.h
rl_history.o: globals.h
rl_history.o: readline/history.h
