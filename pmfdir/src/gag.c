/*
 *  gag.c -- functions for gagging
 *
 *  PMF -- Padrone's MudFrontend, a frontend for (maybe mostly LP-)mud
 *  Thomas Padron-McCarthy (Email: padrone@lysator.liu.se), 1990, 1991
 *  Share and enjoy, but be nice: don't steal my program! Hugo is watching!
 *  This file latest updated: Sept 21, 1991
 *
 */

#include <stdio.h>
#include <strings.h>
#include "safe_malloc.h"
#include "str_galore.h"
#include "config.h"
#include "pmf.h"
#include "globals.h"

#define ELEMENT_POINTER_TYPE    struct gag_struct *
#include "generic_set.h"

/* The generic set "gags" contains the gag definitions. */
struct gag_struct {
    char *pattern;
};
static generic_set gags = NULL;

/*---------------------------------------------------------------------------*/

static int compare_gags(struct1, struct2)
  struct gag_struct *struct1, *struct2;
{
    return strcmp(struct1->pattern, struct2->pattern);
} /* compare_gags */

void init_set_of_gags() {

    gags = create_generic_set(compare_gags);
    set_generic_set_increment(gags, GAG_INCREMENT);
} /* init_set_of_gags */

static struct gag_struct *find_gag_pointer(this_pattern)
  char *this_pattern;
{
    struct gag_struct dummy;

    dummy.pattern = this_pattern;
    return (struct gag_struct *)find_in_generic_set(gags, &dummy);
} /* find_gag_pointer */

char *find_gag_string(this_pattern)
  char *this_pattern;
{
    struct gag_struct *it;

    it = find_gag_pointer(this_pattern);
    return it ? it->pattern : NULL;
} /* find_gag_string */

/*  Add a gag definition.
 *  The string, "this_pattern", is copied.
 */
void add_gag(this_pattern)
  char *this_pattern;
{
    struct gag_struct *new_structp, *old_structp;

    new_structp = (struct gag_struct*)
	safe_malloc(sizeof(struct gag_struct));
    new_structp->pattern = copy_string(this_pattern);

    old_structp = (struct gag_struct *)
	add_to_generic_set(gags, new_structp);
    if (old_structp) {
	safe_free(old_structp->pattern);
	safe_free(old_structp);
    }
} /* add_gag */

remove_gag(this_pattern)
  char *this_pattern;
{
    struct gag_struct dummy, *old_structp;

    dummy.pattern = this_pattern;
    old_structp = (struct gag_struct *)
	remove_from_generic_set(gags, &dummy);
    if (old_structp) {
	safe_free(old_structp->pattern);
	safe_free(old_structp);
    }
    else
	message("No such gag pappern: %s", this_pattern);
} /* remove_gag */

static struct gag_struct *list_one_gag(this_gag_p, outfile)
struct gag_struct *this_gag_p;
FILE *outfile;
{
    if (slash_commands)
	fprintf(outfile, "/");
    fprintf(outfile, "gag ");
    quote_and_print_string(outfile, this_gag_p->pattern);
    fprintf(outfile, "\n");
    return NULL;
} /* list_one_gag */

void list_gags(outfile)
FILE *outfile;
{
    traverse_generic_set(gags, list_one_gag, outfile);
} /* list_gags */

void dump_gags(outfile)
FILE *outfile;
{

    list_gags(outfile);
} /* dump_gags */

int query_nr_gags() {
    return query_generic_set_size(gags);
}

static struct gag_struct *match_one_gag(this_gag, the_line)
struct gag_struct *this_gag;
int the_line;
{
    static char *mudline_words[MAX_WORDS_PER_LINE];
    static char mudline_words_buffer[2 * MAX_LINE_LENGTH];
    int nr_words;

    USER_DEBUG(("match_one_gag: %s", this_gag->pattern));

    nr_words = dollar_match(this_gag->pattern,
			    the_line, mudline_words + 1, mudline_words_buffer);
    if (nr_words != -1) {
	/* Ok, this was a match. This gag matched. Return it. */

	USER_DEBUG(("Gag matched: %s", this_gag->pattern));
	return this_gag;
    }
    else
	return NULL;
} /* match_one_gag */

int is_gag_line(the_line)
char *the_line;
{
    struct gag_struct *the_hit;

    USER_DEBUG(("Checking if this line should be gagged: \"%s\"", the_line));

    the_hit = (struct gag_struct *)traverse_generic_set(gags, match_one_gag, the_line);
    if (the_hit) {
	USER_DEBUG(("Gag! The line matched a gag: %s", the_hit->pattern));
    }

    return (int)the_hit;
} /* is_gag_line */

int fighting_line(line_from_mud)
char *line_from_mud;
{
    char *cp;

    cp = line_from_mud;
    while (*cp) {
        if (   strncmp(cp, " hit", 4) == 0
            || strncmp(cp, " tickle", 7) == 0
            || strncmp(cp, " graze", 6) == 0
            || strncmp(cp, " massacre", 9) == 0
            || strncmp(cp, " die", 4) == 0)
            return 1;
        ++cp;
    }
    return 0;
} /* fighting_line */
