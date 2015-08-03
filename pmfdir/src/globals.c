/*
 *  globals.c -- some global variables
 *
 *  PMF -- Padrone's MudFrontend, a frontend for (maybe mostly LP-)mud
 *  Thomas Padron-McCarthy (Email: padrone@lysator.liu.se), 1990, 1991
 *  Share and enjoy, but be nice: don't steal my program! Hugo is watching!
 *  This file latest updated: May 23, 1993
 *
 */

#include <stdio.h>
#include "config.h"
#include "pmf.h"

/*  mf-variables, that are used internally. They are all initialized to
 *  false (or NULL) here, but may be changed first in initialize(), and
 *  then when reading the init file.
 */
int achtung, debug, ignoreeof, robot_mode, sound_mode, verbose,
    slash_commands, substitute_history, internal_debug, ipc_debug,
    show_receive, replace_control, lines_to_save, screen_length,
    can_gag_fight;
char *pmf_prompt;
char *host_string = NULL, *port_string = NULL;
char *cryptkey = NULL;

/* Some global state variables */
int sending = 0, receiving = 0, sourcing = 0, getfiling = 0;
int echo_is_off = 0;
int x_windows, two_windows;

/*  When a file is open, as when sending or receiving, we remember it
 *  so we can close it if there is an error or the player presses CTRL-C.
 *  The send and receive commands share one such remembered file, since
 *  the cannot nest.
 */
FILE *log_file = NULL, *the_open_file = NULL, *the_source_file = NULL;
char *log_file_name = NULL, *the_open_file_name = NULL, *the_source_file_name = NULL;
int debug_log = 0;

/* This variable is set by do_receive(), and used in communicate_with_mud(). */
char *receive_stop_string = NULL;

FILE *x_outfile = NULL;

int connected = 0;

#ifdef GNU_READLINE
    char readline_prompt[MAX_LINE_LENGTH + 1];
    int readline_prompt_first_char = 0;
#endif

int was_feof = 0;

/* These command-line host and port override those in the init file: */
char *arg_host_string = NULL, *arg_port_string = NULL;

/* The name of the named pipe used in -X mode */
char named_pipe_name[100];

/*  The number of lines received from the MUD game so far,
 *  and the number of entries queued on the "receive" queue.
 */
int nr_received_lines = 0;
