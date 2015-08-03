/*  copy_bytes.c
 *	 -- three functions for safe byte copying
 *  Thomas Padron-McCarthy (padrone@lysator.liu.se), 1991
 *  This file latest updated: Sept 21, 1991
 *
 *  We don't know how memcpy handles overlapping data,
 *  so we use the two functions "copy_bytes_up" and "copy_bytes_down"
 *  to copy data between overlapping areas.
 *  "copy_bytes_down" should be used when copying from an area
 *  to an overlapping area with a LOWER address.
 *  "copy_bytes_up" should be used when copying from an area
 *  to an overlapping area with a HIGHER address.
 *  The function "copy_bytes" can decides for itself which of
 *  those two functions to use.
 *
 *  I have tried to write efficient code. Don't laugh.
 *  I use pointers in register variables,
 *  and use the pointers themselves to count,
 *  i. e. I don't use an extra integer counter.
 *  When possible I use integer copying instead of char copying,
 *  hoping that this will be faster.
 */

#include "copy_bytes.h"

void copy_bytes_up(to_cp, from_cp, nr_bytes)
register char *to_cp, *from_cp;
register int nr_bytes;
{
    if (   (int)to_cp % sizeof(int) == 0
	&& (int)from_cp % sizeof(int) == 0
	&& nr_bytes % sizeof(int) == 0) {
	register int *to_ip, *from_ip, *end_ip;

	to_ip = (int *)to_cp;
	from_ip = (int *)from_cp;
	end_ip = from_ip + nr_bytes / sizeof(int);
	while (from_ip != end_ip)
	    *to_ip++ = *from_ip++;
    }
    else {
	register char *end_cp;

	end_cp = from_cp + nr_bytes;
	while (from_cp != end_cp)
	    *to_cp++ = *from_cp++;
    }
} /* copy_bytes_up */

void copy_bytes_down(to_cp, from_cp, nr_bytes)
register char *to_cp, *from_cp;
register int nr_bytes;
{
    if (   (int)to_cp % sizeof(int) == 0
	&& (int)from_cp % sizeof(int) == 0
	&& nr_bytes % sizeof(int) == 0) {
	register int *to_ip, *from_ip, *end_ip;

	nr_bytes /= sizeof(int);
	to_ip = (int *)to_cp;
	from_ip = (int *)from_cp;

	end_ip = from_ip - 1;
	from_ip = from_ip + nr_bytes - 1;
	to_ip = to_ip + nr_bytes - 1;
	while (from_ip != end_ip)
	    *to_ip-- = *from_ip--;
    }
    else {
	register char *end_cp;

	end_cp = from_cp - 1;
	from_cp = from_cp + nr_bytes - 1;
	to_cp = to_cp + nr_bytes - 1;
	while (from_cp != end_cp)
	    *to_cp-- = *from_cp--;
    }
} /* copy_bytes_down */

void copy_bytes(to_cp, from_cp, nr_bytes)
register char *to_cp, *from_cp;
register int nr_bytes;
{
    if (to_cp < from_cp)
	copy_bytes_up(to_cp, from_cp, nr_bytes);
    else
	copy_bytes_down(to_cp, from_cp, nr_bytes);
} /* copy_bytes */
