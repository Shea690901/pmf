/*
 *  variables.c -- handling pmf's user-accessible variables
 *
 *  PMF -- Padrone's MudFrontend, a frontend for (maybe mostly LP-)mud
 *  Thomas Padron-McCarthy (Email: padrone@lysator.liu.se), 1990, 1991
 *  Share and enjoy, but be nice: don't steal my program! Hugo is watching!
 *  This file latest updated: Sept 21, 1991
 *
 */

#include <strings.h>
#include <stdio.h>
#include "config.h"
#include "safe_malloc.h"
#include "pmf.h"
#include "globals.h"

/* The array "variables" contains the variables and their values. */
struct variable_struct {
    char *the_name, *the_value;
};
static struct variable_struct variables[] = {
    { "debug", NULL },
    { "internal_debug", NULL },
    { "ipc_debug", NULL },
    { "prompt", NULL },
    { "history", NULL },
    { "achtung", NULL },
    { "ignoreeof", NULL },
    { "robot", NULL },
    { "sound", NULL },
    { "verbose", NULL },
    { "slash_commands", NULL },
    { "host", NULL },
    { "port", NULL },
    { "substitute_history", NULL },
    { "replace_control", NULL },
    { "show_receive", NULL },
    { "lines_to_save", NULL },
    { "screen_length", NULL },
    { "cryptkey", NULL }
};
#define NUMBER_OF_VARIABLES 19

/*---------------------------------------------------------------------------*/

int get_varnr(var_name)
char *var_name;
{
    int i;

    for (i = 0; i < NUMBER_OF_VARIABLES; ++i)
	if (!strcmp(var_name, variables[i].the_name))
	    return i;
    return -1;
} /* get_varnr */

char *find_variable_value(this_var_name)
  char *this_var_name;
{
    int varnr;

    varnr = get_varnr(this_var_name);
    if (varnr == -1)
	error("There is no variable called \"%s\".", this_var_name);
    else
	return variables[varnr].the_value;
} /* find_variable_value */

/*  Set a variable value.
 *  The string "this_var_value" is copied.
 */
set_variable(this_var_name, this_var_value)
  char *this_var_name, *this_var_value;
{
    int varnr, int_val;

    USER_DEBUG(("set_variable(\"%s\", \"%s\")", this_var_name, this_var_value));

    varnr = get_varnr(this_var_name);
    if (varnr == -1)
	error("There is no variable called \"%s\".", this_var_name);
    int_val = atoi(this_var_value);
    set_builtin_var(this_var_name, this_var_value);
    if (variables[varnr].the_value)
	safe_free(variables[varnr].the_value);
    variables[varnr].the_value = strcpy(safe_malloc(strlen(this_var_value) + 1), this_var_value);
} /* set_variable */

/*  Set a variable value to false, effectively deleting that variable definition.
 *  The strings, "this_var_name" and "this_var_value" are copied.
 */
int unset_variable(this_var_name)
  char *this_var_name;
{
    int varnr;

    USER_DEBUG(("unset_variable(\"%s\")", this_var_name));

    varnr = get_varnr(this_var_name);
    if (varnr == -1)
	error("There is no variable called \"%s\".", this_var_name);
    set_builtin_var(this_var_name, NULL);
    variables[varnr].the_value = NULL;
} /* unset_variable */

void list_variables(outfile)
FILE *outfile;
{
    int i;

    for (i = 0; i < NUMBER_OF_VARIABLES; ++i) {
	fprintf(outfile, "%-19.19s ", variables[i].the_name);
	if (variables[i].the_value)
	    fprintf(outfile, "\"%s\"\n", variables[i].the_value);
	else
	    fprintf(outfile, "FALSE\n");
    }
} /* list_variables */

void dump_variables(outfile)
FILE *outfile;
{
    int i;
    char *cp;

    for (i = 0; i < NUMBER_OF_VARIABLES; ++i) {
	if (variables[i].the_value) {
	    fprintf(outfile, "%sset %s \"",
		    slash_commands ? "/" : "",
		    variables[i].the_name);
	    cp = variables[i].the_value;
	    while (*cp) {
		if (*cp == '"' || *cp == '$')
		    putc('$', outfile);
		putc(*cp, outfile);
		++cp;
	    }
	    fprintf(outfile, "\"\n");
	}
	else
	    fprintf(outfile, "%sunset %s\n",
		    slash_commands ? "/" : "",
		    variables[i].the_name);
    }
} /* dump_variables */


set_builtin_var(var_name, val)
char *var_name, *val;
{
    int int_val;

    if (!strcmp(var_name, "host")) {
	if (host_string)
	    safe_free(host_string);
	if (val == NULL)
	    host_string = NULL;
	else
	    host_string = strcpy(safe_malloc(strlen((char *)val) + 1), (char *)val);
    }
    else if (!strcmp(var_name, "port")) {
	if (port_string)
	    safe_free(port_string);
	if (val == NULL)
	    port_string = NULL;
	else
	    port_string = strcpy(safe_malloc(strlen((char *)val) + 1), (char *)val);
    }
    else if (!strcmp(var_name, "history")) {
	if (!val)
	    error("The history variable cannot be unset.");
	int_val = atoi(val);
	if (int_val < 1 || int_val > ABSOLUTE_MAX_HISTORY)
	    error("The value \"%s\" is not a legal value for the history variable.");
	set_max_history(int_val);
    }
    else if (!strcmp(var_name, "lines_to_save")) {
	if (!val)
	    error("The lines_to_save variable cannot be unset.");
	int_val = atoi(val);
	if (int_val < 10)
	    error("The value \"%s\" is not a legal value for the lines_to_save variable.", val);
	lines_to_save = int_val;
    }
    else if (!strcmp(var_name, "screen_length")) {
	if (!val)
	    error("The \"screen_length\" variable cannot be unset.");
	int_val = atoi(val);
	if (int_val < 0)
	    error("The value \"%s\" is not a legal value for the screen_length variable.", val);
	screen_length = int_val;
    }
    else if (!strcmp(var_name, "cryptkey")) {
	if (cryptkey)
	    safe_free(cryptkey);
	if (val == NULL)
	    cryptkey = NULL;
	else if (strlen((char *)val) < 5)
	    error("The string \"%s\" is too short as a cryptkey.", val);
	else
	    cryptkey = strcpy(safe_malloc(strlen((char *)val) + 1), (char *)val);
    }
    else if (!strcmp(var_name, "prompt")) {
        if (verbose && !sourcing) {
	    if (val)
		ldisplay("Setting prompt to \"%s\".\n", val);
	    else
		ldisplay("Unsetting the prompt variable.\n");
	}
	if (pmf_prompt)
	    safe_free(pmf_prompt);
	if (val)
	    pmf_prompt = strcpy(safe_malloc(strlen((char *)val) + 1), (char *)val);
	else
	    pmf_prompt = NULL;
    }
    else if (val && strcmp(val, "true")) {
	error("The variable %s is boolean and can not be set with an argument.", var_name);
    }
    
    if (!strcmp(var_name, "achtung"))
	achtung = (int)val;
    else if (!strcmp(var_name, "debug"))
	debug = (int)val;
    else if (!strcmp(var_name, "ipc_debug"))
	ipc_debug = (int)val;
    else if (!strcmp(var_name, "internal_debug"))
	internal_debug = (int)val;
    else if (!strcmp(var_name, "ignoreeof"))
	ignoreeof = (int)val;
    else if (!strcmp(var_name, "robot"))
	robot_mode = (int)val;
    else if (!strcmp(var_name, "sound"))
	sound_mode = (int)val;
    else if (!strcmp(var_name, "verbose"))
	verbose = (int)val;
    else if (!strcmp(var_name, "substitute_history"))
	substitute_history = (int)val;
    else if (!strcmp(var_name, "replace_control"))
	replace_control = (int)val;
    else if (!strcmp(var_name, "show_receive"))
	show_receive = (int)val;
    else if (!strcmp(var_name, "slash_commands"))
	slash_commands = (int)val;
} /* set_builtin_var */
