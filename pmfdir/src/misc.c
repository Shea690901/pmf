/*
 *  misc.c -- some extra things I didn't know where to put
 *
 *  PMF -- Padrone's MudFrontend, a frontend for (maybe mostly LP-)mud
 *  Thomas Padron-McCarthy (Email: padrone@lysator.liu.se), 1990, 1991
 *  Share and enjoy, but be nice: don't steal my program! Hugo is watching!
 *  This file latest updated: Sept 21, 1991
 *
 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <ctype.h>
#include "pmf.h"
#include "config.h"
#include "globals.h"

extern long time();
extern char *ctime();

/*---------------------------------------------------------------------------*/

char *get_statstring(filename)
char *filename;
{
    FILE *fp;
    struct stat statbuf;
    int this_char, nr_lines;
    static char resbuffer[40];

    if (stat(filename, &statbuf))
	uerror("Couldn't stat the file \"%s\" in print_stats.\n", filename);
    else if ((fp = fopen(filename, "r")) == NULL)
	uerror("Couldn't open the file \"%s\" in print_stats.\n", filename);
    else {
	nr_lines = 0;
	while ((this_char = getc(fp)) != EOF)
	    if (this_char == '\n')
		++nr_lines;
	fclose(fp);
	sprintf(resbuffer, "%ld lines, %ld bytes", nr_lines, statbuf.st_size);
	return resbuffer;
    }
} /* get_statstring */

void print_debug_message(fmt, a1, a2, a3, a4, a5)
char *fmt, *a1, *a2, *a3, *a4, *a5;
{
    ldisplay("DEBUG: ");
    ldisplay(fmt, a1, a2, a3, a4, a5);
    ldisplay("\n");
} /* print_debug_message */

void print_internal_debug_message(fmt, a1, a2, a3, a4, a5)
char *fmt, *a1, *a2, *a3, *a4, *a5;
{
    ldisplay("DEBUG: INTERNAL: ");
    ldisplay(fmt, a1, a2, a3, a4, a5);
    ldisplay("\n");
} /* print_internal_debug_message */

void print_ipc_debug_message(fmt, a1, a2, a3, a4, a5)
char *fmt, *a1, *a2, *a3, *a4, *a5;
{
    ldisplay("DEBUG: IPC: ");
    ldisplay(fmt, a1, a2, a3, a4, a5);
    ldisplay("\n");
} /* print_ipc_debug_message */

int quotes_in_line(the_line)
register char *the_line;
{
    while (*the_line) {
	if (*the_line == '"' || *the_line == '\\')
	    return 1;
	++the_line;
    } /* while */
    return 0;
} /* quotes_in_line */

int contains_newline(the_command)
register char *the_command;
{
    while (*the_command) {
	if (*the_command == '\n')
	    return 1;
	++the_command;
    } /* while */
    return 0;
} /* contains_newline */

char *lower_string(start_cp)
  char *start_cp;
{
    register char *cp;

    cp = start_cp;
    while (*cp) {
	if (isupper(*cp))
	    *cp = tolower(*cp);
	++cp;	/* Never trust a macro! */
    }
    return start_cp;
} /* lower_string */

void skip_trailing_space(str)
char *str;
{
    register char *cp;

    cp = str;
    while (*cp)
	++cp;
    --cp;
    while (cp >= str && isspace(*cp))
	--cp;
    cp[1] = '\0';
} /* skip_trailing_space */

quote_and_print_string(outfile, str)
FILE *outfile;
register char *str;
{

    ASSERT(outfile);
    ASSERT(str);

    putc('"', outfile);
    while (*str) {
	if (*str == '"') {
	    putc('$', outfile);
	    putc(*str, outfile);
	}
	else {
	    putc(*str, outfile);
	}
	++str;
    }
    putc('"', outfile);
} /* quote_and_print_string */

/* Move the local file "foo" to "foo.bak" */
int backup_file(local_filename)
char *local_filename;
{
    struct stat dummy_statbuf;
    char filename_buffer[MAX_LINE_LENGTH + 1];

    ASSERT(local_filename);
    ASSERT(local_filename[0]);

    if (stat(local_filename, &dummy_statbuf) == 0) {
	sprintf(filename_buffer, "%s.bak", local_filename);

	if (stat(filename_buffer, &dummy_statbuf) == 0) {
	    message("The file \"%s\" exists. Remove it first.", filename_buffer);
	    return -1;
	}
	else if (   link(local_filename, filename_buffer)
		 || unlink(local_filename)) {
	    message("Couldn't move local file %s to %s.",
		    local_filename, filename_buffer);
	    return -1;
	}
	ldisplay("Moved local file \"%s\" to \"%s\".\n", local_filename, filename_buffer);
    }
    return 0;
} /* backup_file */

/* Make a name for the named pipe to use in -X mode */
void make_npname(bufferp)
char *bufferp;
{

    sprintf(bufferp, "/usr/tmp/pmf%d", getpid());
} /* make_npname */

int nr_lines_in_text(cp)
register char *cp;
{
    register int n = 0;

    while (*cp) {
	if (*cp == '\n')
	    ++n;
	++cp;
    }
    return n;
} /* nr_lines_in_text */

char *get_now_date_string()
{
    long time_now;

    time_now = time(NULL);
    return ctime(&time_now);
} /* get_now_date_string */
