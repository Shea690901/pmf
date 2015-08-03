/*  build_string.c
 *	-- builds UNIX path names
 *  Thomas Padron-McCarthy (padrone@lysator.liu.se), 1991
 *  This file latest updated: Sept 21, 1991
 */

#include <string.h>
#include "safe_malloc.h"
#include "sg_config.h"
#include "str_galore.h"

/* Returnes a malloced path name: */
char *make_path(dir, file)
  char *dir, *file;
{
    char *buf_start, *buf_cp;
    int dirlen, filelen;

    if (file[0] == '/')
	return copy_string(file);

    dirlen = strlen(dir);
    filelen = strlen(file);

    /* 2 extra bytes here: '\0' and (possibly) '/' */
    buf_start = safe_malloc(dirlen + filelen + 2);

    strcpy(buf_start, dir);
    buf_cp = buf_start + dirlen - 1;

    if (*buf_cp != '/')
	*++buf_cp = '/';

    ++buf_cp;
    strcpy(buf_cp, file);
    return buf_start;
} /* make_path */
