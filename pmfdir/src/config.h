/*
 *  config.h -- configuration and version definitions
 *
 *  PMF -- Padrone's MudFrontend, a frontend for (maybe mostly LP-)mud
 *  Thomas Padron-McCarthy (Email: padrone@lysator.liu.se), 1990, 1991
 *  Share and enjoy, but be nice: don't steal my program! Hugo is watching!
 *  This file latest updated: Oct 29, 1993
 *
 */


/*  This is the version number. Please change it if you change the code.
 *  Don't simply increment it, add something instead!
 */
#define VERSION			"1.13.1 (Oct 29, 1993)"

/*  LOCAL_PMF_GURU is the person responsible for the installation of PMF.
 *  You don't have to #define LOCAL_PMF_GURU.
 *
 *	#define LOCAL_PMF_GURU		"Padrone"
 */

/*  If host and/or port is not given as arguments to PMF, and not given in
 *  the init file either, these defaults will be used.
 *  "mud.lysator.liu.se" will connect you to NannyMUD.
 *  (When I'm writing this, "mud.lysator.liu.se" translates to the
 *  numerical address 130.236.254.159, but this is subject to change.)
 */
#define DEFAULT_HOST_STRING	"mud.lysator.liu.se"
#define DEFAULT_PORT_NUMBER	2000

/*  SYSTEM_DIR is a directory some files that are used by PMF are stored.
 *  These files are used by everbody who uses PMF on the system,
 *  so don't change them to customize your own PMF. Edit your own
 *  files instead.
 *  Note that "~" is not expanded in this directory name.
 */
#define SYSTEM_DIR              "/home/diagnostix3/ida/tompa/pmf-1.13.1/pmfdir/system_dir"

/*  These are the names of some files, usually in the system directory
 *  SYSTEM_DIR that is #defined above.
 *  If these names start with a "/" they are considered
 *  relative to the root directory, otherwise to SYSTEM_DIR.
 */
#define SYSTEM_DEFAULT_INIT_FILE        ".pmfrc.default"
#define SYSTEM_NEWS_FILE                "NEWS"
#define SYSTEM_HELP_DIR                 "helpfiles"
#define SYSTEM_SOUND_DIR                "soundfiles"
#define SYSTEM_PLAY_PROGRAM             "soundfiles/play"
/* Not used yet: */
#define SYSTEM_LOG_FILE			"logfile"

/*  The file INIT_FILE_NAME in the player's HOME directory is read at startup.
 *  If not found, or it it exists and cannot be open,
 *  SYSTEM_DEFAULT_INIT_FILE is read.
 */
#define INIT_FILE_NAME		".pmfrc"

/*  This is the name of the program used to display MUD output
 *  when the 'X' option is used (if you use X-windows, of course).
 *  Basically, it replaces "tail -f tempfile".
 */
#define X_OUTPUT_PROGRAM	"cat"

/* What system? Define one either BSD or SYSV:*/
#define BSD
/* #define SYSV */

/* What ioctl do we use for the ttys? */
#ifdef BSD
#  define USE_TIOCGETP
#else
#  define USE_TCGETA
#endif

/*  The prompt to print after doing a builtin command. 0 means no prompt.
 *  The prompt variable is initially set to the value of DEFAULT_PROMPT,
 *  but it can of course later be changed with the command "set prompt".
 */
#define DEFAULT_PROMPT		0

/*  Lots of constants. Might be changed for tuning.
 *  Be careful.
 */
#define MAX_LINE_LENGTH			1000
#define MAX_WORD_LENGTH			100
#define MAX_WORDS_PER_LINE		1000
#define DEFAULT_MAX_HISTORY		20
#define DEFAULT_LINES_TO_SAVE		30
#define DEFAULT_SCREEN_LENGTH		0	/* Never stop! */
#define ABSOLUTE_MAX_HISTORY		1000
#define MAX_ALIAS_EXPANSIONS		10
#define MAX_SOURCING_LEVELS		5
#define MAX_CONFIRM_STRING_LENGTH	100
#define QUEUE_TO_MUD_INCREMENT		30
#define EXPRESS_LIST_INCREMENT		10
#define ROBOT_ACTIONS_INCREMENT		30
#define SOUND_ACTIONS_INCREMENT		30
#define ALIASES_INCREMENT		30
#define GAG_INCREMENT			10
#define LAST_INCREMENT			DEFAULT_LINES_TO_SAVE
#define GETFILE_CHUNKSIZE		10
