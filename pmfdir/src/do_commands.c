/*
 *  do_commands.c -- doing the different PMF commands
 *
 *  PMF -- Padrone's MudFrontend, a frontend for (maybe mostly LP-)mud
 *  Thomas Padron-McCarthy (Email: padrone@lysator.liu.se), 1990, 1991
 *  Share and enjoy, but be nice: don't steal my program! Hugo is watching!
 *  This file latest updated: Sept 21, 1991
 *
 */

#include <strings.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <ctype.h>
#include <errno.h>
#include "safe_malloc.h"
#include "config.h"
#include "pmf.h"
#include "globals.h"

extern char *get_input_line(), *expand_alias(),
    *find_alias_string(),
    *expand_history(), *get_statstring(), *getmud(),
    *expand_variables(),
    *find_robot_action_string();
#ifdef SOUND
    char *find_sound_action_string();
#endif

extern char version[];
extern char *getenv();

extern char *get_now_date_string();

/*---------------------------------------------------------------------------*/

cmd_connect(host, port)
char *host, *port;
{
    if (connected)
	error("You cannot connect when already connected.");

    /*  If arguments were given to the "connect" command, use those.
     *  Else, if a host/or and port were given as arguments to pmf, use those.
     *  Else, if the variables "host" and/or "port" are set, use those.
     *  Otherwise, use the defaults from config.h.
     */

    if (host)
	set_variable("host", host);
    else if (arg_host_string)
	set_variable("host", arg_host_string);
    else if (!host_string)
	set_variable("host", DEFAULT_HOST_STRING);

    if (port)
	set_variable("port", port);
    else if (arg_port_string)
	set_variable("port", arg_port_string);
    else if (!port_string) {
	char *ps = safe_malloc(10);
	sprintf(ps, "%d", DEFAULT_PORT_NUMBER);
	set_variable("port", ps);
    }

    host = host_string;	/* The value of the variable "host" */
    port = port_string;	/* The value of the variable "port" */

    ldisplay("Connecting to host %s port number %s...", host, port);
    fflush(stdout);
    connect_to_mud(host, port);
    ldisplay(" Connected.\n");
} /* cmd_connect */

cmd_disconnect()
{
    if (!connected)
        error("You cannot disconnect if you are not connected.");
    disconnect();
    ldisplay("PMF: Disconnected from host %s port number %s.\n", host_string, port_string);
    fflush(stdout);
} /* cmd_disconnect */

cmd_perform(cmd, confirm_string)
char *cmd, *confirm_string;
{
    
    ASSERT(cmd);

    if (!confirm_string)
	confirm_string = "";
    queue_mudline(cmd, confirm_string);
} /* cmd_perform */

int is_comment_line(cp)
register char *cp;
{

    /* First, skip leading white space. */
    while (isspace(*cp))
        ++cp;

    /* Comment lines are lines that start with # (even after white space) */
    return (*cp == '#');
} /* is_comment_line */

cmd_source(arg)
char *arg;
{
    FILE *fp, *old_source_file;
    char *filename, *this_line, *old_source_file_name;
    int old_sourcing;

    ASSERT(arg);

    if (sourcing >= MAX_SOURCING_LEVELS)
	error("Source command loop. Too many levels (%d) nested.", sourcing + 1);

    if ((fp = fopen(arg, "r")) == NULL) {
	uerror("Couldn't open the file \"%s\".", arg);
    }
    else {

	filename = strcpy(safe_malloc(strlen(arg) + 1), arg);

	/*  Remember the file and its name so we can close it if an error occurs.
	 *  Save the old ones first, so we can restore them after this command.
	 *  We will also save the old "sourcing" state,
	 *  to permit nested source commands.
	 */
	old_source_file = the_source_file;
	old_source_file_name = the_source_file_name;
	the_source_file = fp;
	the_source_file_name = filename;
	old_sourcing = sourcing;
	sourcing += 1;

	if (verbose)
	    ldisplay("\nReading commands from \"%s\" (%s).\n",
		     filename, get_statstring(filename));

	this_line = get_input_line(fp);
	while (! feof(fp)) {
	    if (!is_comment_line(this_line))
		handle_command(this_line, 0);
	    if (verbose)
		ldisplay(".");
	    this_line = get_input_line(fp);
	} /* while */

	if (verbose) {
	    ldisplay("\n");
	    ldisplay("Done \"%s\".\n", filename);
	}

	/* Close the file, free the file name buffer, and restore all states. */
	fclose(fp);
	safe_free(filename);
	the_source_file = old_source_file;
	the_source_file_name = old_source_file_name;
	sourcing = old_sourcing;
    }
} /* cmd_source */


cmd_send(arg, confirm_string, shutup)
char *arg, *confirm_string;
int shutup;
{
    FILE *fp;
    char *filename;
    static char this_line[MAX_LINE_LENGTH + 1];
    int line_length;
    
    ASSERT(arg);

    if (!confirm_string)
	confirm_string = "";
    if (sending || receiving || getfiling) {
	message("Cannot do more than one send or receive command at the same time.");
    }
    else if ((fp = fopen(arg, "r")) == NULL) {
	uerror("Couldn't open the file \"%s\".", arg);
    }
    else {

	filename = strcpy(safe_malloc(strlen(arg) + 1), arg);

	/*  Remember the file and its name so we can close it if an error occurs.
	 *  We don't need to save the old ones first, nor the old value of
	 *  "sending", since send and restore commands cannot nest.
	 */
	the_open_file = fp;
	the_open_file_name = filename;
	sending = 1;

	if (!shutup)
	    ldisplay("Sending file \"%s\" (%s).\n",
		     filename, get_statstring(filename));

	while (fgets(this_line, MAX_LINE_LENGTH + 1, fp) && !feof(fp)) {
	    USER_DEBUG(("going to send: %s", this_line));
	    line_length = strlen(this_line);
	    if (this_line[line_length - 1] == '\n')
		this_line[line_length - 1] = '\0';
	    else if (this_line[line_length - 1])
		message("Long line (%d characters) split when sending %s.",
			MAX_LINE_LENGTH + 1 , filename);
	    queue_mudline(this_line, confirm_string);
	    ldisplay(".");
	    while (!ok_to_send()) {
		communicate_with_mud();
		/* handle_the_player(); */
	    }
	} /* while */
	fclose(fp);
	if (!shutup) {
	    ldisplay("\n");
	    ldisplay("Finished sending \"%s\".\n", filename);
	}

	/* Close the file, free the file name buffer, and restore all states. */
	fclose(fp);
	safe_free(filename);
	the_open_file = NULL;
	the_open_file_name = NULL;
	sending = 0;
    }
} /* cmd_send */

static saved_shutup;

cmd_receive(filename_parameter, stop_string, shutup)
char *filename_parameter, *stop_string;
int shutup;
{
    char *filename;
    int stop_string_length;

    ASSERT(filename_parameter);
    ASSERT(stop_string);

    if (sending || receiving || getfiling) {
	message("Cannot do more than one send or receive command at the same time.");
    }
    else if (backup_file(filename_parameter) == -1)
	uerror("Couldn't backup.");
    else if ((the_open_file = fopen(filename_parameter, "w")) == NULL) {
	uerror("Couldn't open the file \"%s\".", filename_parameter);
    }
    else {
	filename = strcpy(safe_malloc(strlen(filename_parameter) + 1),
			  filename_parameter);
	stop_string_length = strlen(stop_string);
	receive_stop_string = strcpy(safe_malloc(stop_string_length + 1),
				     stop_string);

	/* Remember the file and its name so we can close it if an error occurs: */
	/* We already set "the_open_file". */
	the_open_file_name = filename;
	receiving = 1;

	if (verbose && !shutup)
	    ldisplay("\n");
	if (!shutup)
	    ldisplay("Receiving into file \"%s\".\n", filename);
	saved_shutup = shutup;
    }
} /* cmd_receive */

stop_receiving()
{
    /* Close the file, free the file name buffer, and restore all states. */
    fclose(the_open_file);
    if (!saved_shutup)
	ldisplay("Done. Received \"%s\" (%s).\n", the_open_file_name,
		 get_statstring(the_open_file_name));
    safe_free(the_open_file_name);
    the_open_file = NULL;
    the_open_file_name = NULL;
    receiving = 0;
} /* stop_receiving */

cmd_last(arg)
char *arg;
{
    int nr_lines;

    if (arg)
	nr_lines = atoi(arg);
    else
	nr_lines = 0;
    show_last(nr_lines);
} /* cmd_last */

cmd_dump(arg)
char *arg;
{
    FILE *fp;

    ASSERT(arg);
    if (backup_file(arg) == -1)
	uerror("Couldn't backup.");
    else if ((fp = fopen(arg, "w")) == NULL) {
	uerror("Couldn't open the file \"%s\".", arg);
    }
    else {
	dump_variables(fp);
	dump_alias(fp);
	dump_robot_actions(fp);	
#ifdef SOUND
	dump_sound_actions(fp);	
#endif
	dump_gags(fp);
	fclose(fp);
#ifdef SOUND
	ldisplay("Alias definitions, robot actions, sounds, gags and variables dumped\n");
#else
	ldisplay("Alias definitions, robot actions, gags and variables dumped\n");
#endif
	ldisplay("to the file \"%s\" (%s).\n", arg, get_statstring(arg));
    }
} /* cmd_dump */

cmd_gag(pattern)
char *pattern;
{
    if (!pattern)
	list_gags(stdout);
    else
	add_gag(pattern);
} /* cmd_gag */

cmd_ungag(name)
char *name;
{

    remove_gag(name);
} /* cmd_ungag */

cmd_system(cmd)
char *cmd;
{
    int retval;

    if (!cmd)
	cmd = getenv("SHELL");
    if (!cmd)
	cmd = "/bin/sh";
    retval = system(cmd);
    if (retval != 0)
	message("system(\"%s\") returned %d.", cmd, retval);
} /* cmd_system */

cmd_log(arg, isdebug)
char *arg;
int isdebug;
{
    struct stat dummy_statbuf;

    if (log_file && arg)
	message("A log file is already open. You cannot open another!");
    else if (!log_file && !arg)
	message("But on what file should I write the log?");
    else if (log_file) {
	fprintf(log_file, "\n----------\n");
	fprintf(log_file, "%24.24s: Finished writing MUD dialogue.\n",
		get_now_date_string());
	ldisplay("Closing log file \"%s\".\n", log_file_name);
	fclose(log_file);
	safe_free(log_file_name);
	log_file = NULL;
    }
    else {
	if (stat(arg, &dummy_statbuf) == 0)
	    ldisplay("The file \"%s\" exists. I will appended the log at its end.\n",
		     arg);
	if ((log_file = fopen(arg, "a")) == NULL)
	    uerror("Couldn't open log file \"%s\".", arg);
	else {
	    char *user_name;

	    log_file_name = strcpy(safe_malloc(strlen(arg) + 1), arg);
	    debug_log = isdebug;
	    ldisplay("Saving MUD dialogue %s on the file \"%s\".\n",
		     isdebug ? " and debug info " : "", log_file_name);
	    fprintf(log_file, "This log file, \"%s\",\n", log_file_name);
	    user_name = getenv("USER");
	    fprintf(log_file, "was written by PMF version %s,\n", version);
	    fprintf(log_file, "and it was opened %24.24s by user %s\n",
		    get_now_date_string(),
		    user_name ? user_name : "(unknown)");
	    fprintf(log_file, "when connected to %s %s.\n",
		    host_string, port_string);
	    fprintf(log_file, "----------\n\n");
	}
    }
} /* cmd_log */

cmd_set(var, value)
char *var, *value;
{
    if (!var)
	list_variables(stdout);
    else if (!value)
	set_variable(var, "true");
    else if (   !strcmp(value, "false")
	     || !strcmp(value, "FALSE")
	     || !strcmp(value, "nil"))
	message("Not done. You probably want to use \"frontunset %s\" instead.", var);
    else
	set_variable(var, value);
} /* cmd_set */

cmd_unset(var)
char *var;
{
    unset_variable(var);
} /* cmd_unset */

cmd_status()
{
    ldisplay("\n");
    ldisplay("    This is PMF (Padrone's MudFrontend) version %s.\n", version);
    ldisplay("    Achtung: %s.", achtung ? "ON" : "OFF");
    ldisplay(" Robot: %s.", robot_mode ? "ON" : "OFF");
#ifdef SOUND
    ldisplay(" Sound: %s.", sound_mode ? "ON" : "OFF");
#endif
    ldisplay(" Verbose: %s.", verbose ? "ON" : "OFF");
    ldisplay(" Ignore EOF: %s.\n", ignoreeof ? "ON" : "OFF");
    ldisplay("    Slash-commands: %s.", slash_commands ? "ON" : "OFF");
    ldisplay(" Replace control chars: %s.", replace_control ? "ON" : "OFF");
    ldisplay(" Show receive-lines: %s.\n", show_receive ? "ON" : "OFF");
    ldisplay("    Debug: %s.", debug ? "ON" : "OFF");
    ldisplay(" Prompt: ");
    if (pmf_prompt)
	ldisplay("\"%s\".\n", pmf_prompt);
    else
	ldisplay("NONE.\n");
    ldisplay("    Max history length: %d.", query_max_history());
    ldisplay(" Max last list length: %d.", lines_to_save);
    ldisplay(" Screen length: %d.\n", screen_length);
    ldisplay("    You have defined %d aliases, %d robot actions",
	   query_nr_aliases(), query_nr_robot_actions());
#ifdef SOUND
    ldisplay(", %d sounds", query_nr_sound_actions());
#endif
    ldisplay(" and %d gags.\n", query_nr_gags());

    if (log_file)
	ldisplay("    Logging: ON. File is \"%s\".\n", log_file_name);
    else
	ldisplay("    Logging: OFF.\n");
    ldisplay("    Sourcing levels nested: %d.", sourcing);
    if (sourcing)
	ldisplay(" File is \"%s\".\n", the_source_file_name);
    else
	ldisplay("\n");
    ldisplay("    Sending: %s.", sending ? "ON" : "OFF");
    ldisplay(" Receiving: %s.", receiving ? "ON" : "OFF");
    ldisplay(" Getfile: %s.", getfiling ? "ON" : "OFF");
    if (sending || receiving || getfiling)
	ldisplay(" File is \"%s\".\n", the_open_file_name);
    else
	ldisplay("\n");
    ldisplay("    The queue of lines to send to MUD is %d lines long.",
	   query_queue_to_mud_size());
    if (query_waiting_for_response())
	ldisplay(" Waiting for response.\n");
    else
	ldisplay("\n");

    if (internal_debug)
	ldisplay("    *** Internal DEBUG *** is on!\n");
    if (ipc_debug)
	ldisplay("    *** IPC DEBUG *** is on!\n");

    ldisplay("\n");
} /* cmd_status */

cmd_alias(arg1, rest)
char *arg1, *rest;
{
    char *the_expanded;

    if (!arg1)
	list_alias(stdout);
    else if (!rest) {
	the_expanded = find_alias_string(arg1);
	if (the_expanded)
	    ldisplay("%s\t%s\n", arg1, the_expanded);
	else if (verbose)
	    message("Not defined.");
    }
    else if (!strcmp(arg1, "quote"))
	message("It would be pointless to redefine \"quote\", so I won't let you.");
    else
	add_alias(arg1, rest);
} /* cmd_alias */

cmd_unalias(arg)
char *arg;
{

    remove_alias(arg);
} /* cmd_unalias */

cmd_action(arg1, rest)
char *arg1, *rest;
{
    char *the_action;

    if (!arg1)
	list_robot_actions(stdout);
    else if (!rest) {
	the_action = find_robot_action_string(arg1);
	if (the_action)
	    ldisplay("%s\t%s\n", arg1, the_action);
	else if (verbose)
	    message("Not defined.");
    }
    else
	add_robot_action(arg1, rest);
} /* cmd_action */

cmd_unaction(arg)
char *arg;
{

    remove_robot_action(arg);
} /* cmd_unaction */

cmd_unactionall()
{

    remove_all_robot_actions();
} /* cmd_unactionall */

#ifdef SOUND

cmd_sound(arg1, rest)
char *arg1, *rest;
{
    char *the_action;

    if (!arg1)
	list_sound_actions(stdout);
    else if (!rest) {
	the_action = find_sound_action_string(arg1);
	if (the_action)
	    ldisplay("%s\t%s\n", arg1, the_action);
	else if (verbose)
	    message("Not defined.");
    }
    else
	add_sound_action(arg1, rest);
} /* cmd_sound */

cmd_unsound(arg)
char *arg;
{

    remove_sound_action(arg);
} /* cmd_unsound */

#endif SOUND

cmd_beep()
{
    ldisplay("\007");
} /* cmd_beep */

cmd_echo(str)
char *str;
{
    if (str)
	ldisplay("%s\n", str);
    else
	ldisplay("\n");
} /* cmd_echo */

cmd_cd(str)
char *str;
{

    if (str == NULL)
	str = getenv("HOME");
    if (str == NULL)
	error("Cannot find your home directory: $HOME not set.");
    if (chdir(str) == -1)
	uerror("chdir to %s failed.", str);
} /* cmd_cd */

cmd_history(arg)
char *arg;
{
    int nr_lines;

    if (arg)
	nr_lines = atoi(arg);
    else
	nr_lines = 0;
    print_history(stdout, nr_lines);
} /* cmd_history */

cmd_quit()
{
    say_goodbye_and_exit(0);
} /* cmd_quit */
