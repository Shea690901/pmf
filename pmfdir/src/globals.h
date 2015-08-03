/*
 *  globals.h -- "extern" definitions for the global variables in globals.c
 *
 *  PMF -- Padrone's MudFrontend, a frontend for (maybe mostly LP-)mud
 *  Thomas Padron-McCarthy (Email: padrone@lysator.liu.se), 1990, 1991
 *  Share and enjoy, but be nice: don't steal my program! Hugo is watching!
 *  This file latest updated: May 23, 1993
 *
 */

extern int achtung, debug, ignoreeof, robot_mode, sound_mode, verbose,
           slash_commands, substitute_history, internal_debug, ipc_debug,
           show_receive, replace_control, lines_to_save, screen_length,
           can_gag_fight;
extern char *pmf_prompt;
extern char *host_string, *port_string;
extern char *cryptkey;
extern int sending, receiving, sourcing, getfiling;
extern int echo_is_off;
extern int x_windows, two_windows;
extern FILE *log_file, *the_open_file, *the_source_file;
extern char *log_file_name, *the_open_file_name, *the_source_file_name;
extern int debug_log;
extern char *receive_stop_string;
extern FILE *x_outfile;
extern int connected;

#ifdef GNU_READLINE
    extern char readline_prompt[];
    extern int readline_prompt_first_char;
#endif

extern int was_feof;
extern char *arg_host_string, *arg_port_string;

extern char named_pipe_name[];
extern int nr_received_lines;
