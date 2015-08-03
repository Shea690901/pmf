/*
 *  alias.c -- handling the alias definitions
 *  
 *  PMF -- Padrone's MudFrontend, a frontend for (maybe mostly LP-)mud
 *  Thomas Padron-McCarthy (Email: padrone@lysator.liu.se), 1990, 1991
 *  Share and enjoy, but be nice: don't steal my program! Hugo is watching!
 *  This file latest updated: Sept 21, 1991
 *
 */

#include <strings.h>
#include <ctype.h>
#include <stdio.h>
#include "safe_malloc.h"
#include "str_galore.h"
#include "config.h"
#include "pmf.h"
#include "globals.h"

#define ELEMENT_POINTER_TYPE    struct alias_struct *
#include "generic_set.h"

/* The generic set "set_of_aliases" contains the alias definitions. */
struct alias_struct {
    char *short_form, *expansion;
};
static generic_set set_of_aliases = NULL;

/*---------------------------------------------------------------------------*/

static int compare_aliases(struct1, struct2)
  struct alias_struct *struct1, *struct2;
{
    return strcmp(struct1->short_form, struct2->short_form);
} /* compare_aliases */

void init_set_of_aliases() {

    set_of_aliases = create_generic_set(compare_aliases);
    set_generic_set_increment(set_of_aliases, ALIASES_INCREMENT);
} /* init_set_of_aliases */

static struct alias_struct *find_alias_pointer(this_short)
  char *this_short;
{
    struct alias_struct dummy;

    dummy.short_form = this_short;
    return (struct alias_struct *)find_in_generic_set(set_of_aliases, &dummy);
} /* find_alias_pointer */

char *find_alias_string(this_short)
  char *this_short;
{
    struct alias_struct *it;

    it = find_alias_pointer(this_short);
    return it ? it->expansion : NULL;
} /* find_alias_string */

/*  Add an alias definition.
 *  The strings, "this_short" and "this_expanded" are copied.
 */
void add_alias(this_short, this_expanded)
  char *this_short, *this_expanded;
{
    struct alias_struct *new_structp, *old_structp;

    new_structp = (struct alias_struct *)
	safe_malloc(sizeof(struct alias_struct));
    new_structp->short_form = copy_string(this_short);
    new_structp->expansion = copy_string(this_expanded);
    old_structp = (struct alias_struct *)
	add_to_generic_set(set_of_aliases, new_structp);
    if (old_structp) {
	safe_free(old_structp->short_form);
	safe_free(old_structp->expansion);
	safe_free(old_structp);
    }
} /* add_alias */

void remove_alias(this_short)
  char *this_short;
{
    struct alias_struct dummy, *old_structp;

    dummy.short_form = this_short;
    old_structp = (struct alias_struct *)
	remove_from_generic_set(set_of_aliases, &dummy);
    if (old_structp) {
	safe_free(old_structp->short_form);
	safe_free(old_structp->expansion);
	safe_free(old_structp);
    }
    else
	message("Not defined: %s", this_short);
} /* remove_alias */

static struct alias_struct *list_one_alias(this_alias_p, outfile)
struct alias_struct *this_alias_p;
FILE *outfile;
{
    if (slash_commands)
	fprintf(outfile, "/");
    fprintf(outfile, "alias ");
    quote_and_print_string(outfile, this_alias_p->short_form);
    fprintf(outfile, " ");
    quote_and_print_string(outfile, this_alias_p->expansion);
    fprintf(outfile, "\n");
    return NULL;
} /* list_one_alias */

void list_alias(outfile)
FILE *outfile;
{

    traverse_generic_set(set_of_aliases, list_one_alias, outfile);
} /* list_alias */

void dump_alias(outfile)
FILE *outfile;
{
    list_alias(outfile);
} /* dump_alias */

int query_nr_aliases() {
    return query_generic_set_size(set_of_aliases);
}
