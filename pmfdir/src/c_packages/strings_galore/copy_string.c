/*  copy_string.c -- like strdup(), but using safe_malloc
 *  Thomas Padron-McCarthy (padrone@lysator.liu.se), 1991
 *  This file latest updated: Sept 21, 1991
 */

#include <string.h>
#include "safe_malloc.h"
#include "sg_config.h"
#include "str_galore.h"

char *copy_string(s)
char *s;
{
    return strcpy(safe_malloc(strlen(s) + 1), s);
} /* copy_string */
