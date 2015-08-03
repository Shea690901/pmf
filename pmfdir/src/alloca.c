/*
 *  alloca.c - use it if you have no alloca on your system
 *	
 *	alloca() is like malloc(), except that it allocates memory
 *	on the stack. This means that you don't have to free it
 *	explicitly using free(). From the manual:
 *	
 *	     #include <alloca.h>
 *	     char *alloca(size)
 *	     int size;
 *	
 *	     alloca() allocates size bytes of space in the stack frame of
 *	     the  caller,  and  returns a pointer to the allocated block.
 *	     This temporary space is automatically freed when the  caller
 *	     returns.   Note  that  if  the allocated block is beyond the
 *	     current stack limit, the resulting behavior is undefined.
 *	
 *	Unfortunately, alloca isn't available on all systems.
 *	All the calls to alloca are in the GNU readline library
 *	(in other words: It's not my fault! I didn't write it!).
 *	
 *	In the next version of PMF (2.0, scheduled for 1999 or so)
 *	I'll try to fix this. If you want to play mud before that
 *	you can try to either compile PMF without GNU readline,
 *	or change all the calls to alloca to malloc (which means
 *	the memory will not be freed, so the PMF process will slowly grow,
 *	but that is probably not a big problem (I hope)).
 *	
 *	One way to do this is to write an alloca, like this, that just
 *	calls malloc:
 *	
 *	        char *alloca(size) int size; { return malloc(size); }
 *	
 *	and then compile and link it with the rest of the program.
 *	This is what I have done in this file!
 *	
 */

#include <malloc.h>

char *alloca(size)
int size;
{
    return malloc(size);
}
