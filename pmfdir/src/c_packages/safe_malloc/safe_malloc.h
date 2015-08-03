/*  safe_malloc.h -- #include file for the safe_malloc package
 *  Thomas Padron-McCarthy (padrone@lysator.liu.se), 1991
 *  This file latest updated: Sept 20, 1991
 */

#define SAFE_MALLOC

extern char *safe_malloc();
extern char *safe_realloc();
extern void safe_free();
