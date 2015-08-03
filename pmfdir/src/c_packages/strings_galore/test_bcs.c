/*  test_bcs.c -- a program to test the build_const_string package
 *  Thomas Padron-McCarthy (padrone@lysator.liu.se), 1991
 *  This file latest updated: Sept 21, 1991
 */

#include <stdio.h>
#include "str_galore.h"

#define NR_TESTS 17000

char *ptrs[NR_TESTS];
char tempbuf[1000];

main()
{
    int i, j, l;
    char *cp;
    struct const_string_being_built *csbb;

    srand(0);
    for (i = 0; i < NR_TESTS; ++i) {
	l = rand() % 170;
	csbb = begin_const_string();
	for (j = 0; j < l; ++j)
	    bcs_add_char(csbb, 'a' + j % 28);
	ptrs[i] = end_const_string(csbb);
    }

    srand(0);
    for (i = 0; i < NR_TESTS; ++i) {
	l = rand() % 170;
	printf("%d: (%d, %d) %x \"%20.20s\"...\n",
	       i, strlen(ptrs[i]), l, ptrs[i], ptrs[i]);
    }

} /* main */
