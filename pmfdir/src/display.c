/*
 *  display.c -- show things to the user: ldisplay() and rdisplay()
 *
 *  PMF -- Padrone's MudFrontend, a frontend for (maybe mostly LP-)mud
 *  Thomas Padron-McCarthy (Email: padrone@lysator.liu.se), 1990, 1991
 *  Share and enjoy, but be nice: don't steal my program! Hugo is watching!
 *  This file latest updated: Sept 23, 1993
 *
 */

#include <stdio.h>
#include <ctype.h>
#include <strings.h>
#include "safe_malloc.h"
#include "pmf.h"
#include "config.h"
#include "globals.h"

#define ELEMENT_POINTER_TYPE	struct line_from_mud_entry *
#include "generic_list.h"
struct line_from_mud_entry {
    char *the_line;
    int newline_flag;
};
#include "generic_fifo.h"

/* This is a queue of the lines received from the Mud game. */
static generic_fifo queue_of_lines = NULL;
static lines_printed;	/* On this screen */

/*---------------------------------------------------------------------------*/

/* "Remote-display" -- show text from the mud game to the user: */
rdisplay(fmt, a1, a2, a3, a4, a5)
  char *fmt, *a1, *a2, *a3, *a4, *a5;
{
    static char buf[2000];

    sprintf(buf, fmt, a1, a2, a3, a4, a5);
    show_to_user(buf);
} /* rdisplay */

/* "Local-display" -- show text from pmf itself to the user: */
ldisplay(fmt, a1, a2, a3, a4, a5)
  char *fmt, *a1, *a2, *a3, *a4, *a5;
{

    printf(fmt, a1, a2, a3, a4, a5);
} /* ldisplay */

/*----------------------------------------------------------------------------*/

init_queue_of_lines()
{
    queue_of_lines = create_generic_fifo(NULL);
    set_generic_fifo_increment(queue_of_lines, LAST_INCREMENT);
} /* init_queue_of_lines */


static maybe_add_newline_to_queue()
{
    struct line_from_mud_entry *p;

    p = find_last_in_generic_fifo(queue_of_lines);
    if (p != NULL)
	p->newline_flag = 1;
} /* maybe_add_newline_to_queue */
 
show_to_user(str)
char *str;
{
    char *cp;
    FILE *outfile;
    struct line_from_mud_entry *p;
    int this_length;

    INTERNAL_DEBUG(("show_to_user(\"%s\")", str));

    this_length = strlen(str);
    if (screen_length > 0 && str[this_length - 1] == '\n') {
	++lines_printed;
	if (lines_printed >= screen_length) {
	    printf("\007");
	    printf("\n--- Press RETURN to continue! ---\n");
	    while (getchar() != '\n')
		;
	    continue_printing();
	}
    }

    if (str[this_length - 1] != '\n' && this_length <= MAX_LINE_LENGTH) {

#ifdef GNU_READLINE
	/* strcpy(readline_prompt, str); */
	/* strcpy(readline_prompt, "GNU readline> "); */
	/* rl_redisplay(); */
#endif

    }

    if (achtung)
	printf("\007\007");
    if (x_windows)
	outfile = x_outfile;
    else
	outfile = stdout;
    cp = str;
    while (*cp) {
	if ((isascii(*cp) && isprint(*cp)) || !replace_control || *cp == '\n' || *cp == '\t')
	    fputc(*cp, outfile);
	else
	    fprintf(outfile, "\\%3.3o", *cp&255);
	++cp;
    }
    fflush(outfile);

    if (str[0] == '\n' && str[1] == '\0') 
	maybe_add_newline_to_queue();
    else {
	if (query_generic_fifo_size(queue_of_lines) > lines_to_save) {
	    struct line_from_mud_entry *ep;
	    ep = (struct line_from_mud_entry *)get_from_generic_fifo(queue_of_lines);
	    safe_free(ep->the_line);
	    safe_free(ep);
	    /* A memory leak in pmf 1.9! */
	}
	p = (struct line_from_mud_entry *)
	    safe_malloc(sizeof(struct line_from_mud_entry));
	p->the_line = strcpy(safe_malloc(strlen(str) + 1), str);
	queue_on_generic_fifo(queue_of_lines, p);
	ASSERT(p->the_line[strlen(p->the_line) - 1] != '\n');
    }
} /* show_to_user */

struct line_from_mud_entry *print_one_line(this_line_struct)
  struct line_from_mud_entry *this_line_struct;
{
    printf("%s", this_line_struct->the_line);
    if (this_line_struct->newline_flag)
	printf("\n");
    return NULL;
} /* print_one_line */

show_last(nr_lines)
int nr_lines;
{
    struct line_from_mud_entry **p;
    int n, i;

    p = (struct line_from_mud_entry **)
	get_array_inside_generic_list(get_list_inside_generic_fifo(queue_of_lines));
    n = query_generic_fifo_size(queue_of_lines);

    if (nr_lines > n || nr_lines <= 0)
	nr_lines = n;

    for (i = n - nr_lines; i < n; ++i)
	print_one_line(p[i]);
} /* show_last */

/* Always call this function when the user typed a newline */
user_newline()
{

    /* So the show_to_user doesn't think the screen is full: */
    continue_printing();

    if (x_windows)
	show_to_user("\n");
    else
	maybe_add_newline_to_queue();
    continue_printing();
} /* user_newline */

stop_printing()
{
    lines_printed = screen_length + 1;
} /* stop_printing */

continue_printing() {
    lines_printed = 0;
} /* continue_printing */
