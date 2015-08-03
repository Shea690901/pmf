/*
 *  handle_cmd.c -- handling commands to PMF (local and remote)
 *
 *  PMF -- Padrone's MudFrontend, a frontend for (maybe mostly LP-)mud
 *  Thomas Padron-McCarthy (Email: padrone@lysator.liu.se), 1990, 1991
 *  Share and enjoy, but be nice: don't steal my program! Hugo is watching!
 *  This file latest updated: Sept 22, 1991
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
    *apply_definition();

extern int handle_command();
extern void do_remote_command();

/*---------------------------------------------------------------------------*/

/*  This function takes care of a command given by the player.
 *  It is supposed to be in a malloc'ed string,
 *  with the trailing '\n' removed.
 *  It will not be free'd in this function or in functions called by it.
 */
void player_command(command_line)
char *command_line;
{
    char *cp, *cmd_subst = NULL;
    int retval;

    USER_DEBUG(("player_command(\"%s\")", command_line));

    cmd_subst = NULL;

    /* First, skip leading white space. */
    cp = command_line;
    while (isspace(*cp))
	++cp;

    /*  We don't check for comment lines here, since it's only when
     *  reading commands from files that lines starting with #
     *  should be skipped.
     */

    /*  Blank lines are simply sent to the Mud process.
     *  All the white space is sent.
     */
    if (*cp == 0) {
	do_remote_command(command_line, 0);
	return;
    }

    /* If this is a "quote" command, simply send the rest of the line to Mud. */
    if (   (slash_commands && !strncmp("/quote", cp, 6))
	|| (!slash_commands && !strncmp("quote", cp, 5))) {
	do_remote_command(cp + (slash_commands ? 7 : 6), 0);
	add_mud_history(cp);
    	return;
    }

    if (substitute_history) {

	/* Not a quote command. Substitue any history references (!! and !number). */
	cmd_subst = expand_history(cp);
	if (cmd_subst) {
	    /*  The "cmd_subst" string is a new, malloc'ed, string.
	     *  Don't forget to free it later!
	     */
	    USER_DEBUG(("History substitution yielded: %s", cmd_subst));
	    cp = cmd_subst;
	}

	if (cmd_subst && verbose)
	    ldisplay("History substitution yielded: %s\n", cp);
    }

    add_mud_history(cp);

    /*  History substitution might have yielded a new "quote" command,
     *  but that will be handled in handle_command.
     */
    if (cmd_subst)
	retval = handle_command(cp, 0);
    else
	retval = handle_command(command_line, 0);

    if (cmd_subst)
        safe_free(cmd_subst);

    if (retval && pmf_prompt && !sourcing)
	ldisplay(pmf_prompt, query_latest_event_nr() + 1);

} /* player_command */


/*  Handle a command line from the user (possibly form a source'd file).
 *  It is called AFTER any history substitution, and AFTER storing the
 *  command on the history list. Also, comments and "quote" commands have
 *  already been taken care of.
 *  If the line is expanded to several lines, by use of the $n notation,
 *  this function is called recursively for each of those new lines.
 *  If expressflag is true, the remote command(s) will be added at
 *  the HEAD of the queue of commands to send to the game
 *  (but not directly -- they are put on a special express list first).
 *  The number of executed local commands is returned.
 */
int handle_command(command_line, expressflag)
char *command_line;
int expressflag;
{
    char *cp, *the_definition, *this_expanded, *latest_expanded;
    int nr_words, nr_alias_expansions, i, result;
    char *command_line_words[MAX_WORDS_PER_LINE],
          command_line_words_buffer[2 * MAX_LINE_LENGTH];
    int use_command_line, recursive_retval;
    int was_slash_command;

    USER_DEBUG(("handle_command(\"%s\", %d)", command_line, expressflag));

    use_command_line = 1;

    /* First, skip leading white space. It will still be there! */
    cp = command_line;
    while (isspace(*cp))
	++cp;

    /*  Comment lines, i. e. lines that start with # (even after white space) are ignored.
     *  If this was a player command, those lines are skipped, but not if it was source'd.
     */
    if (*cp == '#') {
	return 0;
    }

    /* Blank lines are simply sent to the Mud process. All the white space is sent. */
    if (*cp == 0) {
	do_remote_command(command_line, expressflag);
	return 0;
    }

    was_slash_command = slash_commands && *cp == '/';

    /* If this is a "quote" command, simply send the rest of the line to Mud. */
    if (   (slash_commands && !strncmp("/quote", cp, 6))
	|| (!slash_commands && !strncmp("quote", cp, 5))) {
	do_remote_command(cp + (slash_commands ? 7 : 6), 0);
    	return 0;
    }

    /*  Now, do alias substitution! The function "expand_one_alias" will return NULL
     *  if no substitutions were done, or a malloc'ed string that we must free later!
     *  "cp" still points to the first non-blank char, so we can do "alias /foo fum".
     */
    nr_alias_expansions = 0;
    latest_expanded = NULL;
    nr_words = word_split(cp, command_line_words, command_line_words_buffer);

    /*  If word_split failed (it returned 0),
     *  we simply send the line to the Mud process.
     */
    if (nr_words == 0) {
	do_remote_command(command_line, expressflag);
	if (verbose)
	    ldisplay("The line is not interpreted by PMF, but is sent directly to MUD.\n");
	return 0;
    }

    /*  If the command line contained at least one un-quoted $n, it was
     *  split into lines instead of words. Each line is a new command!
     */
    if (nr_words < 0) {
	use_command_line = 0;
	nr_words = -nr_words;
	if (verbose) {
	    ldisplay("The command line was split into %d new command lines.\n", nr_words);
	    for (i=0; i<nr_words; ++i)
		ldisplay("      %d: '%s'\n", i, command_line_words[i]);
	}
	if (debug) {
	    int i;
	    ldisplay("DEBUG: The line was split (1) at $n into %d new lines:\n", nr_words);
	    for (i=0; i<nr_words; ++i)
		ldisplay("      %d: '%s'\n", i, command_line_words[i]);
	}
	recursive_retval = 0;
	for (i=0; i<nr_words; ++i)
	    recursive_retval += handle_command(command_line_words[i], expressflag);
	return recursive_retval;
    }

    if (debug && nr_words > 0) {
	int i;
	ldisplay("DEBUG: First splitting (1) of the command line: %d words:\n", nr_words);
	for (i=0; i<nr_words; ++i)
	    ldisplay("      %d: '%s'\n", i, command_line_words[i]);
    }

    while ((the_definition = find_alias_string(command_line_words[0])) != NULL) {
	use_command_line = 0;
	if (++nr_alias_expansions > MAX_ALIAS_EXPANSIONS)
	    error("Alias loop. Too many levels (%d) substituted.",
		  nr_alias_expansions);
	this_expanded = apply_definition(the_definition, nr_words,
					 command_line_words, 1);
	USER_DEBUG(("This alias substitution yielded: %s", this_expanded));
	if (latest_expanded)
	    safe_free(latest_expanded);
	latest_expanded = this_expanded;
	cp = this_expanded;
	nr_words = word_split(cp, command_line_words, command_line_words_buffer);

	/*  If word_split failed (it returned 0),
	 *  we simply send the line to the Mud process.
	 */
	if (nr_words == 0) {
	    do_remote_command(cp, expressflag);	/* This alias! */
	    if (verbose)
		ldisplay("The line is not interpreted by PMF, but is sent directly to MUD.\n");
	    return 0;
	}

	/*  If the command line contained at least one un-quoted $n, it was
	 *  split into lines instead of words. Each line is a new command!
	 */
	if (nr_words < 0) {
	    nr_words = -nr_words;
	    if (verbose) {
		ldisplay("The command line was split into %d new command lines:\n", nr_words);
		for (i=0; i<nr_words; ++i)
		    ldisplay("      %d: '%s'\n", i, command_line_words[i]);
	    }
	    if (debug) {
		int i;
		ldisplay("DEBUG: The line was split (2) at $n into %d new lines:\n", nr_words);
		for (i=0; i<nr_words; ++i)
		    ldisplay("      %d: '%s'\n", i, command_line_words[i]);
	    }

	    recursive_retval = 0;
	    for (i=0; i<nr_words; ++i)
		recursive_retval += handle_command(command_line_words[i], expressflag);
	    return recursive_retval;
	}

	if (debug && nr_words > 0) {
	    int i;
	    ldisplay("DEBUG: This splitting (2) of the command line: %d words:\n", nr_words);
	    for (i=0; i<nr_words; ++i)
		ldisplay("      %d: '%s'\n", i, command_line_words[i]);
	}
	
    } /* While it's still an alias. */
    
    if (latest_expanded && verbose && !sourcing)
	ldisplay("Alias substitution yielded: %s\n", cp);
    
    for (i = nr_words; i < MAX_WORDS_PER_LINE; ++i)
	command_line_words[i] = NULL;
    
    ASSERT(!isspace(*cp));

    /* Can be a local OR a remote command */

    result = do_local_command(cp, nr_words, command_line_words, expressflag);
    if (result == -1) {

	/* No such local PMF command */

	if (slash_commands && *cp == '/') {
	    message("No such local PMF command: \"%s\".", command_line_words[0]);
	    ldisplay("    The line is sent to MUD instead. Try \"/help /quote\"!\n");
	}

	if (use_command_line)
	    do_remote_command(command_line, expressflag);
	else
	    do_remote_command(cp, expressflag);
    }

    if (latest_expanded)
	safe_free(latest_expanded);
    if (result < 0)
	return 0;
    else
	return result;
} /* handle_command */


handle_the_player()
{
    char *player_line;

    while ((player_line = get_input_line(stdin)) != NULL)
	player_command(player_line);
} /* handle_the_player */


/*  Do the local command "the_local_command".
 *  0 is returned if a local command was executed, even if it failed
 *  or was syntactically incorrect (wrong number of arguments).
 *  The expressflag is used for remote commands.
 *  1 is returned if a local command and a remote command was executed.
 *  -1 is returned if "the_local_command" wasn't a valid local command.
 */
int do_local_command(the_local_command, nr_words, words, expressflag)
char *the_local_command, *words[];
int nr_words;
int expressflag;
{
    char *restargs;



#define LOCAL_COMMAND(cmd, slashcmd) (slash_commands ? slashcmd : cmd)

    if (!strcmp(words[0], LOCAL_COMMAND("quote", "/quote"))) {
	restargs = the_local_command;
	while (isspace(*restargs))
	    ++restargs;
	/* Now we are at the beginning of the word "quote". */
	restargs += 5;
	/* Now we are immediately after the word "quote"! */
	if (isspace(*restargs))
	    ++restargs;
	do_remote_command(restargs, expressflag);
    }
    else if (!strcmp(words[0], LOCAL_COMMAND("connect", "/connect"))) {
	if (nr_words > 3)
	    message("Too many arguments to the \"connect\" command.");
	else
	    cmd_connect(words[1], words[2]);
    }
    else if (!strcmp(words[0], LOCAL_COMMAND("disconnect", "/disconnect"))) {
	if (nr_words > 1)
            message("You cannot give any arguments to the \"disconnect\" command.");
	else
	    cmd_disconnect();
    }
    else if (!strcmp(words[0], LOCAL_COMMAND("help", "/help"))) {
	if (nr_words > 2)
	    message("Usage:  /help  [ SUBJECT ].");
	else
	    cmd_help(words[1]);
    }
    else if (!strcmp(words[0], "help")) {
	if (nr_words == 2 && (!strcmp(lower_string(words[1]), "pmf") || !strcmp(words[1], "front")))
	    cmd_help(0);
	else {
	    if (verbose) {
		ldisplay("(Note: PMF is passing on this \"help\" command to MUD.\n");
		ldisplay("       Use the command \"/help\" for help about the PMF program.)\n");
	    }
	    do_remote_command(the_local_command, expressflag);
	    return 1;
	}
    }
    else if (!strcmp(words[0], LOCAL_COMMAND("status", "/status"))) {
	if (nr_words > 1)
	    message("You cannot give any arguments to the \"/status\" command.");
	else
	    cmd_status();
    }
    else if (!strcmp(words[0], LOCAL_COMMAND("history", "/history"))) {
	if (nr_words > 2)
	    message("Too many arguments to the \"history\" command.");
	else
	    cmd_history(words[1]);
    }
    else if (!strcmp(words[0], LOCAL_COMMAND("log", "/log"))) {
	if (nr_words > 2)
	    message("Usage:  /log  [ FILENAME ]\n");
	else
	    cmd_log(words[1], 0);
    }
    else if (!strcmp(words[0], LOCAL_COMMAND("debuglog", "/debuglog"))) {
	if (nr_words > 2)
	    message("Usage:  /debuglog  [ FILENAME ]\n");
	else
	    cmd_log(words[1], 1);
    }
    else if (!strcmp(words[0], LOCAL_COMMAND("perform", "/perform"))) {
	if (nr_words < 2)
	    message("You must give a MUD command as argument to the \"perform\" command.");
	else if (nr_words > 3)
	    message("Too many arguments to the \"perform\" command.");
	else
	    cmd_perform(words[1], words[2]);
    }
    else if (!strcmp(words[0], LOCAL_COMMAND("send", "/send"))) {
	if (nr_words < 2)
	    message("You must give a file name as argument to the \"send\" command.");
	else if (nr_words > 3)
	    message("Too many arguments to the \"send\" command.");
	else
	    cmd_send(words[1], words[2], 0);
    }
    else if (!strcmp(words[0], LOCAL_COMMAND("receive", "/receive"))) {
	if (nr_words < 2)
	    message("You must give a file name as argument to the \"receive\" command.");
	else if (nr_words < 3)
	    message("You must give a stop string as second argument to the \"receive\" command.");
	else if (nr_words > 3)
	    message("Too many arguments to the \"receive\" command.");
	else
	    cmd_receive(words[1], words[2], 0);
    }
    else if (!strcmp(words[0], LOCAL_COMMAND("putfile", "/putfile"))) {
	if (nr_words < 2)
	    message("You must give at least one file name as argument to \"putfile\".");
	else if (nr_words > 3)
	    message("Too many arguments to \"putfile\".");
	else
	    cmd_putfile(words[1], words[2]);
    }
    else if (!strcmp(words[0], LOCAL_COMMAND("getfile", "/getfile"))) {
	if (nr_words < 2)
	    message("You must give at least one file name as argument to \"getfile\".");
	else if (nr_words > 4)
	    message("Too many arguments to \"getfile\".");
	else
	    cmd_getfile(words[1], words[2]);
    }
    else if (!strcmp(words[0], LOCAL_COMMAND("last", "/last"))) {
	if (nr_words > 2)
	    message("Too many arguments to the \"last\" command.");
	else
	    cmd_last(words[1]);
    }
    else if (!strcmp(words[0], LOCAL_COMMAND("source", "/source"))) {
	if (nr_words < 2)
	    message("You must give a file name as argument to the \"source\" command.");
	else if (nr_words > 2)
	    message("Too many arguments to \"source\" command.");
	else
	    cmd_source(words[1]);
    }
    else if (!strcmp(words[0], LOCAL_COMMAND("dump", "/dump"))) {
	if (nr_words < 2)
	    message("You must give a file name as argument to the \"dump\" command.");
	else if (nr_words > 2)
	    message("Too many arguments to the \"dump\" command.");
	else
	    cmd_dump(words[1]);
    }
    else if (!strcmp(words[0], LOCAL_COMMAND("alias", "/alias"))) {
	if (nr_words == 3)
	    restargs = words[2];
	else if (nr_words > 3 && quotes_in_line(the_local_command)) {
	    message("No quoting allowed if you have more than two arguments to \"alias\".");
	    return 0;
	}
	else if (nr_words > 3) {
	    restargs = the_local_command;
	    while (isspace(*restargs))
		++restargs;
	    /* Now we are at the beginning of the first word. */
	    while (!isspace(*restargs))
		++restargs;
	    while (isspace(*restargs))
		++restargs;
	    /* Now we are at the beginning of the second word. */
	    while (!isspace(*restargs))
		++restargs;
	    while (isspace(*restargs))
		++restargs;
	    /* Now we are at the beginning of the third word. */
	}
	else
	    restargs = NULL;
	cmd_alias(words[1], restargs);
    }
    else if (!strcmp(words[0], LOCAL_COMMAND("unalias", "/unalias"))) {
	if (nr_words < 2)
	    message("You must give an argument to the \"unalias\" command.");
	else if (nr_words > 2)
	    message("Too many arguments to the \"unalias\" command.");
	else
	    cmd_unalias(words[1]);
    }
    else if (!strcmp(words[0], LOCAL_COMMAND("action", "/action"))) {
	if (nr_words == 3)
	    restargs = words[2];
	else if (nr_words > 3 && quotes_in_line(the_local_command)) {
	    message("No quoting allowed if you have more than two arguments to \"action\".");
	    return 0;
	}
	else if (nr_words > 3) {
	    restargs = the_local_command;
	    while (isspace(*restargs))
		++restargs;
	    /* Now we are at the beginning of the first word. */
	    while (!isspace(*restargs))
		++restargs;
	    while (isspace(*restargs))
		++restargs;
	    /* Now we are at the beginning of the second word. */
	    while (!isspace(*restargs))
		++restargs;
	    while (isspace(*restargs))
		++restargs;
	    /* Now we are at the beginning of the third word. */
	}
	else
	    restargs = NULL;
	cmd_action(words[1], restargs);
    }
    else if (!strcmp(words[0], LOCAL_COMMAND("unaction", "/unaction"))) {
	if (nr_words < 2)
	    message("You must give an argument to the \"unaction\" command.");
	else if (nr_words > 2)
	    message("Too many arguments to the \"unaction\" command.");
	else
	    cmd_unaction(words[1]);
    }
    else if (!strcmp(words[0], LOCAL_COMMAND("unactionall", "/unactionall"))) {
	if (nr_words != 1)
	    message("You can't give any arguments to \"unactionall\".");
	else
	    cmd_unactionall();
    }
    else if (!strcmp(words[0], LOCAL_COMMAND("sound", "/sound"))) {
	if (nr_words == 3)
	    restargs = words[2];
	else if (nr_words > 3 && quotes_in_line(the_local_command)) {
	    message("No quoting allowed if you have more than two arguments to \"sound\".");
	    return 0;
	}
	else if (nr_words > 3) {
	    restargs = the_local_command;
	    while (isspace(*restargs))
		++restargs;
	    /* Now we are at the beginning of the first word. */
	    while (!isspace(*restargs))
		++restargs;
	    while (isspace(*restargs))
		++restargs;
	    /* Now we are at the beginning of the second word. */
	    while (!isspace(*restargs))
		++restargs;
	    while (isspace(*restargs))
		++restargs;
	    /* Now we are at the beginning of the third word. */
	}
	else
	    restargs = NULL;

#ifdef SOUND
	cmd_sound(words[1], restargs);
#endif

    }
    else if (!strcmp(words[0], LOCAL_COMMAND("unsound", "/unsound"))) {
	if (nr_words < 2)
	    message("You must give an argument to the \"unsound\" command.");
	else if (nr_words > 2)
	    message("Too many arguments to the \"unsound\" command.");

#ifdef SOUND
	else
	    cmd_unsound(words[1]);
#endif

    }
    else if (!strcmp(words[0], LOCAL_COMMAND("beep", "/beep"))) {
	if (nr_words != 1)
	    message("You can not give any arguments to the \"beep\" command.");
	else
	    cmd_beep();
    }
    else if (!strcmp(words[0], LOCAL_COMMAND("echo", "/echo"))) {
	if (nr_words == 1)
	    restargs = NULL;
	else if (nr_words == 2)
	    restargs = words[1];
	else {
	    ASSERT(nr_words > 2);
	    restargs = the_local_command;
	    while (isspace(*restargs))
		++restargs;
	    /* Now we are at the beginning of the first word. */
	    while (!isspace(*restargs))
		++restargs;
	    while (isspace(*restargs))
		++restargs;
	    /* Now we are at the beginning of the second word. */
	}
	cmd_echo(restargs);
    }
    else if (!strcmp(words[0], LOCAL_COMMAND("cd", "/cd"))) {
	if (nr_words > 2)
	    message("Usage:  /cd  [ DIRECTORY-NAME ]");
	else
	    cmd_cd(words[1]);
    }
    else if (!strcmp(words[0], LOCAL_COMMAND("cryptsay", "/cryptsay"))) {
	if (nr_words == 1) {
	    message("You must give an argument to the \"cryptsay\" command.");
	    return 0;
	}
	else if (nr_words == 2)
	    restargs = words[1];
	else {
	    ASSERT(nr_words > 2);
	    restargs = the_local_command;
	    while (isspace(*restargs))
		++restargs;
	    /* Now we are at the beginning of the first word. */
	    while (!isspace(*restargs))
		++restargs;
	    while (isspace(*restargs))
		++restargs;
	    /* Now we are at the beginning of the second word. */
	}
	cmd_cryptsay(restargs);
    }
    else if (!strcmp(words[0], LOCAL_COMMAND("crypttell", "/crypttell"))) {
	if (nr_words < 3) {
	    message("You must give at least two arguments to the \"crypttell\" command.");
	    return 0;
	}
	else if (nr_words == 3)
	    restargs = words[2];
	else {
	    ASSERT(nr_words > 3);
	    restargs = the_local_command;
	    while (isspace(*restargs))
		++restargs;
	    /* Now we are at the beginning of the first word. */
	    while (!isspace(*restargs))
		++restargs;
	    while (isspace(*restargs))
		++restargs;
	    /* Now we are at the beginning of the second word. */
	    while (!isspace(*restargs))
		++restargs;
	    /* Now we are just after the second word. */
	    ++restargs;
	    /* Now we are after the second word, plus one space or tab. */
	}
	cmd_crypttell(words[1], restargs);
    }
    else if (!strcmp(words[0], LOCAL_COMMAND("system", "/system"))) {
	if (nr_words < 2)
	    restargs = NULL;
	else if (nr_words == 2)
	    restargs = words[1];
	else if (nr_words > 2 && quotes_in_line(the_local_command)) {
	    message("No quoting allowed if you have more than two arguments to \"system\".");
	    return 0;
	}
	else {
	    ASSERT(nr_words > 2);
	    restargs = the_local_command;
	    while (isspace(*restargs))
		++restargs;
	    /* Now we are at the beginning of the first word. */
	    while (!isspace(*restargs))
		++restargs;
	    while (isspace(*restargs))
		++restargs;
	    /* Now we are at the beginning of the second word. */
	}
	cmd_system(restargs);
    }
    else if (!strcmp(words[0], LOCAL_COMMAND("gag", "/gag"))) {
	if (nr_words > 2)
	    message("You cannot give more than one person or pattern as argument to the \"gag\" command.");
	else
	    cmd_gag(words[1]);
    }
    else if (!strcmp(words[0], LOCAL_COMMAND("ungag", "/ungag"))) {
	if (nr_words < 2)
	    message("Yes, but who dou you want to ungag?");
	else if (nr_words > 2)
	    message("You cannot give more than one person as argument to the \"ungag\" command.");
	else
	    cmd_ungag(words[1]);
    }
    else if (!strcmp(words[0], LOCAL_COMMAND("set", "/set"))) {
	if (nr_words > 3)
	    message("Usage:  /set  VARIABLE-NAME  [ VALUE ]");
	else
	    cmd_set(words[1], words[2]);
    }
    else if (!strcmp(words[0], LOCAL_COMMAND("unset", "/unset"))) {
	if (nr_words != 2)
	    message("Usage:  /unset  VARIABLE-NAME");
	else {
	    cmd_unset(words[1]);
	}
    }
    else if (!strcmp(words[0], LOCAL_COMMAND("quit", "/quit"))) {
	if (nr_words > 1)
	    message("You cannot give any arguments to the command \"/quit\".");
	else
	    cmd_quit();
    }
    else
	return -1;

    return 0;
} /* do_local_command */


void do_remote_command(the_remote_command, expressflag)
char *the_remote_command;
int expressflag;
{

    if (!connected)
	message("You cannot give commands to MUD until you are connected to MUD.");
    else if (sending)
	message("You cannot give commands to MUD while sending a file.");
    else if (expressflag)
	express_queue_mudline(the_remote_command, "");
    else
	queue_mudline(the_remote_command, "");
} /* do_remote_command */
