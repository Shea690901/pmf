/*  add_strings.c -- uses safe_malloc
 *  Thomas Padron-McCarthy (padrone@lysator.liu.se), 1991
 *  This file latest updated: Sept 21, 1991
 */

#include <string.h>
#include "safe_malloc.h"
#include "sg_config.h"
#include "str_galore.h"

#ifndef NULL
#  define NULL 0
#endif

char *add_strings(s1, s2)
  char *s1, *s2;
{
    char *buf_start, *buf_cp;
    int len1, len2;

    len1 = strlen(s1);
    len2 = strlen(s2);

    buf_start = safe_malloc(len1 + len2 + 1);

    strcpy(buf_start, s1);
    buf_cp = buf_start + len1;
    strcpy(buf_cp, s2);
    return buf_start;
} /* add_strings */

/* Ok, this should use varargs, but how do I make that portable? */
char *add_many_strings(n, s1, s2, s3, s4, s5)
  int n;
  char *s1, *s2, *s3, *s4, *s5;
{

    return NULL;
} /* add_many_strings */
