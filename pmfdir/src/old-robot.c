/*
 *  robot.c -- the robot mode (Hi there! Why do you look like that?)
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

#define ELEMENT_POINTER_TYPE    struct robot_action_struct *
#include "generic_set.h"

/* The generic set "robot_actions" contains the robot_action definitions. */
struct robot_action_struct {
    char *pattern, *action;
};
static generic_set robot_actions = NULL;

extern char *apply_definition();
extern long int time();

/*---------------------------------------------------------------------------*/

static int compare_robot_actions(struct1, struct2)
  struct robot_action_struct *struct1, *struct2;
{
    return strcmp(struct1->pattern, struct2->pattern);
} /* compare_robot_actions */

void init_set_of_robot_actions() {

    robot_actions = create_generic_set(compare_robot_actions);
    set_generic_set_increment(robot_actions, ROBOT_ACTIONS_INCREMENT);
} /* init_set_of_robot_actions */

static struct robot_action_struct *find_robot_action_pointer(this_pattern)
  char *this_pattern;
{
    struct robot_action_struct dummy;

    dummy.pattern = this_pattern;
    return (struct robot_action_struct *)find_in_generic_set(robot_actions, &dummy);
} /* find_robot_action_pointer */

char *find_robot_action_string(this_pattern)
  char *this_pattern;
{
    struct robot_action_struct *it;

    it = find_robot_action_pointer(this_pattern);
    return it ? it->action : NULL;
} /* find_robot_action_string */

/*  Add a robot_action definition.
 *  The strings, "this_pattern" and "this_action" are copied.
 */
void add_robot_action(this_pattern, this_action)
  char *this_pattern, *this_action;
{
    struct robot_action_struct *new_structp, *old_structp;

    new_structp = (struct robot_action_struct*)
	safe_malloc(sizeof(struct robot_action_struct));
    new_structp->pattern = copy_string(this_pattern);
    new_structp->action = copy_string(this_action);

    old_structp = (struct robot_action_struct *)
	add_to_generic_set(robot_actions, new_structp);
    if (old_structp) {
	safe_free(old_structp->pattern);
	safe_free(old_structp->action);
	safe_free(old_structp);
    }
} /* add_robot_action */

remove_robot_action(this_pattern)
  char *this_pattern;
{
    struct robot_action_struct dummy, *old_structp;

    dummy.pattern = this_pattern;
    old_structp = (struct robot_action_struct *)
	remove_from_generic_set(robot_actions, &dummy);
    if (old_structp) {
	safe_free(old_structp->pattern);
	safe_free(old_structp->action);
	safe_free(old_structp);
    }
    else
	message("No such robot action defined: %s", this_pattern);
} /* remove_robot_action */

struct robot_action_struct *kill_one_robot_action(this_robot_action_p)
  struct robot_action_struct *this_robot_action_p;
{
    safe_free(this_robot_action_p->pattern);
    safe_free(this_robot_action_p->action);
    safe_free(this_robot_action_p);
    return NULL;
} /* kill_one_robot_action */

remove_all_robot_actions() {

    empty_generic_set(robot_actions, kill_one_robot_action);
    ASSERT(query_generic_set_size(robot_actions) == 0);	
} /* remove_all_robot_actions */

static struct robot_action_struct *list_one_robot_action(this_robot_action_p, outfile)
struct robot_action_struct *this_robot_action_p;
FILE *outfile;
{
    if (slash_commands)
	fprintf(outfile, "/");
    fprintf(outfile, "action ");
    quote_and_print_string(outfile, this_robot_action_p->pattern);
    fprintf(outfile, " ");
    quote_and_print_string(outfile, this_robot_action_p->action);
    fprintf(outfile, "\n");
    return NULL;
} /* list_one_robot_action */

void list_robot_actions(outfile)
FILE *outfile;
{
    traverse_generic_set(robot_actions, list_one_robot_action, outfile);
} /* list_robot_actions */

void dump_robot_actions(outfile)
FILE *outfile;
{

    list_robot_actions(outfile);
} /* dump_robot_actions */

int query_nr_robot_actions() {
    return query_generic_set_size(robot_actions);
}

char *try_one_robot_action(this_action, the_line)
struct robot_action_struct *this_action;
char *the_line;
{
    static char *mudline_words[MAX_WORDS_PER_LINE];
    static char mudline_words_buffer[2 * MAX_LINE_LENGTH];
    int nr_words;
    char *response;

    INTERNAL_DEBUG(("try_one_robot_action: %s", this_action->pattern));

    nr_words = dollar_match(this_action->pattern,
			    the_line, mudline_words + 1, mudline_words_buffer);
    if (nr_words != -1) {
	/* Ok, this was a match. Do this robot action. */

	USER_DEBUG(("Robot action triggered: %s", this_action->action));

	if (this_action->action[0] == '\0')	/* Empty string: no action! */
	    return this_action->action;

	mudline_words[0] = this_action->action;
	response = apply_definition(this_action->action,
				    nr_words + 1, mudline_words, 0);
	ASSERT(response != NULL);
	return response;
    }
    else
	return NULL;
} /* try_one_robot_action */

void robot_handle_line(the_line)
char *the_line;
{
    char *response;
    static int time_counter = 0;
    static long int latest_time = 0L;

    USER_DEBUG(("robot_handle_line: %s", the_line));

    response = (char *)
	traverse_generic_set(robot_actions, try_one_robot_action, the_line);
    if (response != NULL && response[0] != '\0') {

	if (verbose) {
	    ldisplay("\007");
	    ldisplay("*** PMF robot action: ");
	    ldisplay("%s\n", response);	/* The response can contain %'s! */
	    ldisplay("\n");
	}

	if (time_counter == 0 && latest_time == 0L) {
	    /* Initialize the robot action timer */
	    latest_time = time(NULL);
	}
	else if (++time_counter >= 10) {
	    /* Ten actions since last check! Do a check! */
	    time_counter = 0;
	    if (time(NULL) - latest_time < 30) {
		latest_time = 0L;
		unset_variable("robot");
		error("Possible robot action loop detected. Robot mode turned off.");
	    }
	    else
		latest_time = time(NULL);
	}

	handle_command(response, 1);	/* Handle with express! */
	/* add_history(response); -- NO! */
	safe_free(response);
    }
} /* robot_handle_line */
