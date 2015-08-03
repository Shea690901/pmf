/*  str_galore.h -- #include file for the strings_galore package
 *  This file is called "str_galore.h" and not "strings_galore.h"
 *  because som Unix systems still think file names should be limited
 *  to 14 characters. It is sad, but that is the way it is.
 *  Thomas Padron-McCarthy (padrone@lysator.liu.se), 1991
 *  This file latest updated: Sept 21, 1991
 */

/* Copying strings (like strdup, but uses safe_malloc): */
extern char *copy_string();

/* Copy a string and convert it's case: */
extern char *lower_string_copy();
extern char *upper_string_copy();

/* Dynamic building of strings: */
struct string_being_built {
    char *string_buffer;
    int buffer_length;
    int string_length;
};
extern struct string_being_built *begin_string();
extern void bs_add_char();
extern char *end_string();

/* Dynamic building of constant strings, i. e. they cannot be freed: */
struct const_string_being_built {
    char *start_cp, *cp;
    int length;
};
extern struct const_string_being_built *begin_const_string();
extern void bcs_add_char();
extern char *end_const_string();

/* Build a UNIX path name: */
extern char *make_path();

/* Add strings: */
extern char *add_strings();
