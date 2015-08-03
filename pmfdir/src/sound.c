/*
 *  sound.c -- the sound mode for Sun SPARCstations
 *
 *  PMF -- Padrone's MudFrontend, a frontend for (maybe mostly LP-)mud
 *  Thomas Padron-McCarthy (Email: padrone@lysator.liu.se), 1990, 1991
 *  Share and enjoy, but be nice: don't steal my program! Hugo is watching!
 *  This file latest updated: Sept 22, 1991
 *
 */

#include <stdio.h>
#include <strings.h>
#include "safe_malloc.h"
#include "str_galore.h"
#include "config.h"
#include "pmf.h"
#include "globals.h"

#include "generic_set.h"

/* The generic set "sound_actions" contains the sound_action definitions. */
struct sound_action_struct {
    char *the_pattern, *filename;
};
static generic_set sound_actions = NULL;

/*---------------------------------------------------------------------------*/

static void play_soundfile(soundname)
char *soundname;
{
    static char *sounddir = NULL, *play_program;
    char *filename, *command_buf;
    FILE *fp;

    if (sounddir == NULL) {
	sounddir = make_path(SYSTEM_DIR, SYSTEM_SOUND_DIR);
	play_program = make_path(SYSTEM_DIR, SYSTEM_PLAY_PROGRAM);
    }

    filename = make_path(sounddir, soundname);

    /* Just check that the file exists first! */
    fp = fopen(filename, "r");
    if (fp == NULL) {
	safe_free(filename);
	error("Couldn't open sound file \"%s\".", filename);
	return;
    }
    fclose(fp);

    command_buf = safe_malloc(sizeof(play_program) + strlen(filename) + 2);
    sprintf(command_buf, "%s %s", play_program, filename);

    USER_DEBUG(("Performing a sound command: %s", command_buf));
    system(command_buf);

    safe_free(filename);
    safe_free(command_buf);
} /* play_soundfile */

/*---------------------------------------------------------------------------*/

static int compare_sound_actions(struct1, struct2)
  struct sound_action_struct *struct1, *struct2;
{
    return strcmp(struct1->the_pattern, struct2->the_pattern);
} /* compare_sound_actions */

void init_set_of_sound_actions() {

    sound_actions = create_generic_set(compare_sound_actions);
    set_generic_set_increment(sound_actions, SOUND_ACTIONS_INCREMENT);
} /* init_set_of_sound_actions */

static struct sound_action_struct *find_sound_action_pointer(this_pattern)
  char *this_pattern;
{
    struct sound_action_struct dummy;

    dummy.the_pattern = this_pattern;
    return (struct sound_action_struct *)find_in_generic_set(sound_actions, &dummy);
} /* find_sound_action_pointer */

char *find_sound_action_string(this_pattern)
  char *this_pattern;
{
    struct sound_action_struct *it;

    it = find_sound_action_pointer(this_pattern);
    return it ? it->filename : NULL;
} /* find_sound_action_string */

/*  Add a sound_action definition.
 *  The strings, "this_pattern" and "this_action" are copied.
 */
void add_sound_action(this_pattern, this_action)
  char *this_pattern, *this_action;
{
    struct sound_action_struct *new_structp, *old_structp;

    new_structp = (struct sound_action_struct*)
	safe_malloc(sizeof(struct sound_action_struct));
    new_structp->the_pattern = strcpy(safe_malloc(strlen(this_pattern) + 1), this_pattern);
    new_structp->filename = strcpy(safe_malloc(strlen(this_action) + 1), this_action);

    old_structp = (struct sound_action_struct *)
	add_to_generic_set(sound_actions, new_structp);
    if (old_structp) {
	safe_free(old_structp->the_pattern);
	safe_free(old_structp->filename);
	safe_free(old_structp);
    }
} /* add_sound_action */

remove_sound_action(this_pattern)
  char *this_pattern;
{
    struct sound_action_struct dummy, *old_structp;

    dummy.the_pattern = this_pattern;
    old_structp = (struct sound_action_struct *)
	remove_from_generic_set(sound_actions, &dummy);
    if (old_structp) {
	safe_free(old_structp->the_pattern);
	safe_free(old_structp->filename);
	safe_free(old_structp);
    }
    else
	message("No such sound action defined: %s", this_pattern);
} /* remove_sound_action */

static void kill_one_sound_action(this_sound_action_p)
  struct sound_action_struct *this_sound_action_p;
{
    safe_free(this_sound_action_p->the_pattern);
    safe_free(this_sound_action_p->filename);
    safe_free(this_sound_action_p);
} /* kill_one_sound_action */

static struct sound_action_struct *list_one_sound_action(this_sound_action_p, outfile)
struct sound_action_struct *this_sound_action_p;
FILE *outfile;
{
    if (slash_commands)
	fprintf(outfile, "/");
    fprintf(outfile, "sound ");
    quote_and_print_string(outfile, this_sound_action_p->the_pattern);
    fprintf(outfile, " ");
    quote_and_print_string(outfile, this_sound_action_p->filename);
    fprintf(outfile, "\n");
    return NULL;
} /* list_one_sound_action */

void list_sound_actions(outfile)
FILE *outfile;
{
    traverse_generic_set(sound_actions, list_one_sound_action, outfile);
} /* list_sound_actions */

void dump_sound_actions(outfile)
FILE *outfile;
{

    list_sound_actions(outfile);
} /* dump_sound_actions */

int query_nr_sound_actions() {
    return query_generic_set_size(sound_actions);
}

static char *try_one_sound_action(this_action, the_line)
struct sound_action_struct *this_action;
char *the_line;
{
    static char *mudline_words[MAX_WORDS_PER_LINE];
    static char mudline_words_buffer[2 * MAX_LINE_LENGTH];
    int nr_words;

    INTERNAL_DEBUG(("try_one_sound_action (in sound): %s", this_action->the_pattern));

    nr_words = dollar_match(this_action->the_pattern,
			    the_line, mudline_words + 1, mudline_words_buffer);
    if (nr_words != -1) {
	/* Ok, this was a match. Do this sound action. */
	USER_DEBUG(("Sound action triggered: %s", this_action->filename));
	return this_action->filename;
    }
    else
	return NULL;
} /* try_one_sound_action */

void sound_handle_line(the_line)
char *the_line;
{
    char *response;

    USER_DEBUG(("sound_handle_line: %s", the_line));

    response = traverse_generic_set(sound_actions, try_one_sound_action, the_line);
    if (response != NULL && response[0] != '\0') {
	USER_DEBUG(("Sound action triggered: %s", response));
	play_soundfile(response);
	/* safe_free(response); -- NO! */
    }
} /* sound_handle_line */
