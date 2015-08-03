/*
 *  main.c -- the function "main", some initializations, welcome-messages etc
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
#include <sgtty.h>
#include <signal.h>
#include <setjmp.h>
#include "safe_malloc.h"
#include "str_galore.h"
#include "config.h"
#include "pmf.h"
#include "globals.h"

extern char *getenv();

extern char *getmud();
extern char *get_input_line();

extern int communicate_with_mud();
#ifdef GNU_READLINE
    extern char *rl_readline_name;
    extern (*rl_event_hook)();
    extern int rl_clear_message();
    extern char *rl_line_buffer;
    extern int rl_point, rl_end;

    extern int stop_printing();
    extern int continue_printing();

    extern int last_c_pos;
#endif

/* Longjmp buffer, jump there after error() or CTRL-C */
jmp_buf home_sweet_home;	/* Initialized in main() */
static char *initfile_string;

char version[] = VERSION;
int no_news = 0;

extern FILE *popen_x_out();

/*---------------------------------------------------------------------------*/

#ifdef GNU_READLINE
    int pmf_rl_redisplay_line(count, key)
    int count, key;
    {
	ldisplay("\n");
	ldisplay("%s", rl_line_buffer);
	rl_redisplay(count, key);
	last_c_pos = rl_end;
    }
#endif


void init_variables()
{
    char buf[10];

    /* Set history list */
    sprintf(buf, "%d", DEFAULT_MAX_HISTORY);
    set_variable("history", buf);
    /* set_variable("substitute_history", "true"); */
    set_variable("show_receive", "true");
    set_variable("slash_commands", "true");
    sprintf(buf, "%d", DEFAULT_LINES_TO_SAVE);
    set_variable("lines_to_save", buf);
    set_variable("verbose", "true");
    sprintf(buf, "%d", DEFAULT_SCREEN_LENGTH);
    set_variable("screen_length", buf);
} /* init_variables */

#ifdef GNU_READLINE
    init_readline()
    {

	rl_event_hook = communicate_with_mud;
	rl_bind_key('\022', pmf_rl_redisplay_line);	/* CTRL-R */
	rl_bind_key('\023', stop_printing);		/* CTRL-S */
	rl_bind_key('\021', continue_printing);		/* CTRL-Q */
	rl_readline_name = "pmf";	/* For conditional parsing of the ~/.inputrc file. */

    } /* init_readline */
#endif

initialize()
{

    setup_signals();

    /* We don't want any stdio-buffering of stdin and stdout. */
    /* setbuf(stdin, NULL); */
    /* setbuf(stdout, NULL); */

    /* S{tt cbreak-mode, with no line buffering of stdin. */
    /* set_cbreak(); */

    init_variables();

    init_queue_to_mud();
    init_queue_of_lines();
    init_set_of_aliases();
    init_set_of_robot_actions();
    init_set_of_gags();

#ifdef SOUND
    init_set_of_sound_actions();
#endif

#ifdef GNU_READLINE
    init_readline();
    pmf_save_terminal();
#endif    
} /* initialize */

say_hello()
{
    printf("\n");
    printf("Welcome to PMF -- Padrone's MudFrontend version %s.\n", version);
    if (verbose) {
	printf("\n");
	printf("The PMF program connects to the MUD server in the same way as telnet, but\n");
	printf("some of the lines you type are interpreted as commands to the PMF program.\n");
	printf("Type \"/help\" or \"help pmf\" for more information.\n");
	printf("\n");
	printf("No warranty, except that I guarantee that there ARE bugs in this program.\n");

#ifdef LOCAL_PMF_GURU
	printf("\n");
	printf("If you have any problems with PMF, talk to %s!\n", LOCAL_PMF_GURU);
#endif

    }

    printf("\n");
} /* say_hello */

int print_news()
{
    char *news_file;
    FILE *fp;
    int c;

    news_file = make_path(SYSTEM_DIR, SYSTEM_NEWS_FILE);

    fp = fopen(news_file, "r");
    safe_free(news_file);

    if (fp == NULL) {
	return -1;
    }
    else {
	printf("Local news about PMF:\n");
	while ((c = getc(fp)) != EOF)
	    putc(c, stdout);
	printf("\n");
	return 0;
    }
} /* print_news */

say_goodbye_and_exit(exitval)
int exitval;
{
    printf("\n");
    printf("PMF -- Padrone's MudFrontend version %s.\n", version);
    printf("Goodbye. Have a nice day. Enjoy Coke. Be back soon!\n");
    printf("\n");
    disconnect();

#ifdef GNU_READLINE
    pmf_restore_terminal();
#endif

    if (x_outfile)
	fclose(x_outfile);
    if (named_pipe_name[0])
	unlink(named_pipe_name);

#ifdef DEBUG_MALLOC
    dump_malloc();
#endif

    if (debug && exitval) {
	signal(SIGIOT, SIG_DFL);
	abort();
    }
    else
	exit(exitval);
} /* say_goodbye_and_exit */

/*---------------------------------------------------------------------------*/

handle_args(argc, argv)
  int argc;
  char *argv[];
{
    int argnr;
    char *cp;
    char next_is_initfile;

    next_is_initfile = 0;
    initfile_string = NULL;
    arg_host_string = NULL;
    arg_port_string = NULL;

    for (argnr = 1; argnr < argc; ++argnr)
	if (argv[argnr][0] == '-') {
	    cp = &argv[argnr][1];
	    while (*cp) {
		switch (*cp) {
		    case 'X' : x_windows = 1; break; 
		    case '2' : two_windows = 1; break; 
		    case 'f' : next_is_initfile = 1; break;
		    case 'd' : debug = 1; set_variable("debug", "true"); break;
#ifdef DEBUG
		    case 'i' : internal_debug = 1; set_variable("internal_debug", "true"); break;
		    case 'c' : ipc_debug = 1; set_variable("ipc_debug", "true"); break;
#endif
		    case 'n' : no_news = 1; break;
		    case 's' : verbose = 0; unset_variable("verbose"); break;
		    default : usage(); break;
		}
		++cp;
	    }
	}
        else {
	    if (next_is_initfile) {
		initfile_string = argv[argnr];
		next_is_initfile = 0;
	    }
	    else if (!arg_host_string)
		arg_host_string = argv[argnr];
	    else if (!arg_port_string)
		arg_port_string = argv[argnr];
	    else
		usage();
	}
} /* handle_args */

usage()
{
    fprintf(stderr, "Usage: pmf [ -dnsX2 ] [ -f startfile ] [ HOST [ PORT ] ]\n");
    exit(1);
} /* usage */

goto_home_sweet_home(arg)
int arg;
{

    longjmp(home_sweet_home, arg);
} /* goto_home_sweet_home */

/*---------------------------------------------------------------------------*/

main(argc, argv)
  int argc;
  char *argv[];
{

    initialize();
    handle_args(argc, argv);

    if (x_windows) {
	x_outfile = popen_x_out();
	if (x_outfile == NULL) {
	    message("Couldn't fix the X output. X mode off.");
	    x_windows = 0;
	}
    }

    if (x_windows && two_windows) {
	message("You cannot use both -2 and -X. Turning off -2.");
	two_windows = 0;
    }

    say_hello();
    if (!no_news)
	print_news();

    if (setjmp(home_sweet_home)) {
	/*  This will not close all files if we have nested source or send/receive
	 *  commands. But maybe we can live with that.
	 */
	if (the_open_file) {
	    fclose(the_open_file);
	    the_open_file = NULL;
	    ldisplay("Closed: \"%s\"\n", the_open_file_name);
	}
	if (the_source_file) {
	    fclose(the_source_file);
	    the_source_file = NULL;
	    ldisplay("Closed: \"%s\"\n", the_source_file_name);
	}
	sending = 0;
	receiving = 0;
	sourcing = 0;
	getfiling = 0;
	flush_queue_to_mud();
	pmf_restore_terminal();
	if (pmf_prompt)
	    ldisplay(pmf_prompt, query_latest_event_nr() + 1);
    }
    else {

	/*  These we must do here, since the longjmp destination must be set
	 *  if there is an error or the user types CTRL-C.
	 */
	do_init_file();
	if (!connected)
	    cmd_connect(NULL, NULL);

    }

    was_feof = 0;

    while (1) {

	while (!feof(stdin) && !was_feof) {
	    handle_the_player();
	    communicate_with_mud();
	} /* while */

	ldisplay("\n");
	ldisplay("PMF has received an End-Of-File character.\n");

	if (ignoreeof) {
	    ldisplay("Since the variable \"ignoreeof\" is set, the End-Of-File character is ignored.\n");
	    ldisplay("Logout from LPmud with \"quit\", or use \"/quit\" just to exit from PMF.\n");
	    ldisplay("\n");
	    clearerr(stdin);
	    was_feof = 0;
	}
	else {
	    if (connected)
		ldisplay("Disconnecting from the game -- but maybe you should \"quit\", too?\n");
	    say_goodbye_and_exit(0);
	}
    }
} /* main */

int do_init_file()
{
    struct stat dummy_statbuf;
    char filename_buf[256], *the_filename, *this_line, *the_home;
    FILE *fp;

    fp = NULL;
    the_home = getenv("HOME");
    if (initfile_string) {
	the_filename = initfile_string;
	if ((fp = fopen(the_filename, "r")) == NULL)
	    message("Couldn't read the init file %s.", the_filename);
    }
    if (!fp && the_home) {
	sprintf(filename_buf, "%s/%s", the_home, INIT_FILE_NAME);
	the_filename = filename_buf;
	if (stat(the_filename, &dummy_statbuf) == 0) {
	    if ((fp = fopen(the_filename, "r")) == NULL)
		message("Couldn't read your init file %s.", the_filename);
	}
    }
    if (!fp) {
	the_filename = make_path(SYSTEM_DIR, SYSTEM_DEFAULT_INIT_FILE);
	if (stat(the_filename, &dummy_statbuf) == 0) {
	    if ((fp = fopen(the_filename, "r")) == NULL)
		message("Couldn't read the default init file %s.", the_filename);
	}
    }
    if (!fp)
	return -1;

    /* Remember the file and its name so we can close it if an error occurs: */
    the_open_file = fp;
    the_open_file_name = the_filename;

    if (verbose)
	ldisplay("Reading init file %s (%s).\n",
		 the_filename, get_statstring(the_filename));
    sourcing = 1;

    this_line = get_input_line(fp);
    while (! feof(fp)) {
	if (!is_comment_line(this_line))
	    handle_command(this_line, 0);
	/* safe_free(this_line); -- Ooops! No no no! */
	if (verbose)
	    ldisplay(".");
	this_line = get_input_line(fp);
    } /* while */
    if (verbose)
	ldisplay("\nInit file %s done.\n", the_filename);
    fclose(fp);
    the_open_file = NULL;

    sourcing = 0;

    return 0;
} /* do_init_file */
