/*  generic_fifo.c -- generic FIFO queue handling
 *  Thomas Padron-McCarthy (padrone@lysator.liu.se), 1991
 *  Share and enjoy, but be nice: don't steal my program! Hugo is watching!
 *  This file latest updated: Sept 21, 1991
 *
 *	This is a package for handling FIFO queues of something.
 *	We call these somethings "objects" (or sometimes "elements").
 *	The objects are referred to by pointers, and this package
 *	manipulates those pointers, never the objects themselves.
 */

#include "generic_fifo.h"
#include "safe_malloc.h"

#define NULL 0
#ifdef DEBUG
#   define GF_ASSERT(where, what)	\
	{				\
		if (!(what))		\
			fatal("%s: ASSERT failed in \"%s\" line %d: %s",	\
			      where, __FILE__, __LINE__, "what");		\
	}
#else
#   define GF_ASSERT(where, what)	(what)
#endif

/*---------------------------------------------------------------------------*/

/*  To create a generic FIFO queue, call the function create_generic_fifo with
 *  a compare function (a pointer to a function returning integer) as
 *  argument. This function should take as arguments pointers to two
 *  somethings, compare them, and return negative, 0, or positive.
 *  The new generic FIFO queue, of type generic_fifo, is returned.
 *  It is possible to give NULL as compare_function,
 *  but then you can not search the queue.
 */
generic_fifo create_generic_fifo(compare_function)
int (*compare_function)();
{
    generic_fifo this_fifo;

    this_fifo = (generic_fifo)create_generic_list(compare_function);
    return this_fifo;
} /* create_generic_fifo */

/*---------------------------------------------------------------------------*/

/*  Destroy a generic FIFO queue. It frees the generic_fifo structure and the buffer
 *  of pointers used to point to the objects in the FIFO queue, but the objects
 *  themselves, if there are any in the FIFO queue, are not affected.
 */
void kill_generic_fifo(the_fifo)
  register generic_fifo the_fifo;
{

    GF_ASSERT("kill_generic_fifo", the_fifo != NULL);

    kill_generic_list((generic_list)the_fifo);
} /* kill_generic_fifo */

/*---------------------------------------------------------------------------*/

/*  This function searches in a generic FIFO queue for an object matching
 *  the object pointed to by object_to_search_for.
 *  A pointer to the (first) found object is returned, or NULL is returned.
 */
any_pointer find_in_generic_fifo(the_fifo, object_to_search_for_p)
  register generic_fifo the_fifo;
  any_pointer object_to_search_for_p;
{

    GF_ASSERT("find_in_generic_fifo", the_fifo != NULL);
    GF_ASSERT("find_in_generic_fifo", object_to_search_for_p != NULL);
    GF_ASSERT("find_in_generic_fifo", the_fifo->compare_function != NULL);

    return find_in_generic_list((generic_list)the_fifo, object_to_search_for_p);
} /* find_in_generic_fifo */

/*---------------------------------------------------------------------------*/

/*  This function searches in a generic FIFO queue for an object matching
 *  the object pointed to by object_to_search_for, and removes it.
 *  It returns a pointer to the object that was removed from the FIFO queue.
 */
any_pointer remove_from_generic_fifo(the_fifo, object_to_remove)
  register generic_fifo the_fifo;
  any_pointer object_to_remove;
{

    GF_ASSERT("remove_from_generic_fifo", the_fifo != NULL);
    GF_ASSERT("remove_from_generic_fifo", object_to_remove != NULL);

    return remove_from_generic_list((generic_list)the_fifo, object_to_remove);
} /* remove_from_generic_fifo */

/*---------------------------------------------------------------------------*/

/*  Qeue an object, pointed to by the_object_p, on a generic FIFO queue.
 */
void queue_on_generic_fifo(the_fifo, the_object_p)
  register generic_fifo the_fifo;
  any_pointer the_object_p;
{
    GF_ASSERT("queue_on_generic_fifo", the_fifo != NULL);
    GF_ASSERT("queue_on_generic_fifo", the_object_p != NULL);

    add_last_to_generic_list((generic_list)the_fifo, the_object_p);
} /* queue_on_generic_fifo */

/*---------------------------------------------------------------------------*/

/*  Get and unque the first object from a generic FIFO queue.
 *  If the queue is empty, NULL is returned.
 */
any_pointer get_from_generic_fifo(the_fifo)
  register generic_fifo the_fifo;
{

    GF_ASSERT("get_from_generic_fifo", the_fifo != NULL);

    return remove_first_from_generic_list((generic_list)the_fifo);
} /* get_from_generic_fifo */

/*---------------------------------------------------------------------------*/

/*  Get the first object of a generic FIFO queue, without unqueueing it.
 *  If the queue is empty, NULL is returned.
 */
any_pointer find_first_in_generic_fifo(the_fifo)
  register generic_fifo the_fifo;
{

    GF_ASSERT("find_first_in_generic_fifo", the_fifo != NULL);

    return find_first_in_generic_list((generic_list)the_fifo);
} /* find_first_in_generic_fifo */

/*---------------------------------------------------------------------------*/

/*  Get the last object of a generic FIFO queue, without unqueueing it.
 *  If the queue is empty, NULL is returned.
 */
any_pointer find_last_in_generic_fifo(the_fifo)
  register generic_fifo the_fifo;
{

    GF_ASSERT("find_last_in_generic_fifo", the_fifo != NULL);

    return find_last_in_generic_list((generic_list)the_fifo);
} /* find_last_in_generic_fifo */

/*---------------------------------------------------------------------------*/

/*  This function calls the function function_to_call once for each object
 *  in the FIFO queue, with a pointer to that object as first argument.
 *  This is done as long as the function_to_call returns NULL; if it returns
 *  another value, traverse_generic_fifo will stop, and return that value.
 *  If that never happens, traverse_generic_fifo returns NULL.
 *  WARNING: Do not change the FIFO queue in the function function_to_call!
 *  The third argument to this function, optional_second_argument, will be
 *  passed to function_to_call as its second argument.
 */
any_pointer traverse_generic_fifo(the_fifo, function_to_call,
				  optional_second_argument)
  generic_fifo the_fifo;
  any_pointer (*function_to_call)();
  char *optional_second_argument;
{

    GF_ASSERT("traverse_generic_fifo", the_fifo != NULL);
    GF_ASSERT("traverse_generic_fifo", function_to_call != NULL);

    return traverse_generic_list((generic_list)the_fifo, function_to_call,
				 optional_second_argument);
} /* traverse_generic_fifo */

/*---------------------------------------------------------------------------*/

/*  As traverse_generic_fifo(), but traverses the queue backwards.
 */
any_pointer traverse_generic_fifo_backwards(the_fifo, function_to_call,
					    optional_second_argument)
  generic_fifo the_fifo;
  any_pointer (*function_to_call)();
  char *optional_second_argument;
{

    GF_ASSERT("traverse_generic_fifobackwards", the_fifo != NULL);
    GF_ASSERT("traverse_generic_fifo_backwards", function_to_call != NULL);

    return traverse_generic_list_backwards((generic_list)the_fifo,
					   function_to_call,
					   optional_second_argument);
} /* traverse_generic_fifo_backwards */

/*---------------------------------------------------------------------------*/

int query_generic_fifo_size(the_fifo)
  generic_fifo the_fifo;
{

    GF_ASSERT("query_generic_fifo_size", the_fifo != NULL);

    return the_fifo->current_number;
} /* query_generic_fifo_size */

/*---------------------------------------------------------------------------*/

/*  Set the increment. The old increment is returned.
 */
int set_generic_fifo_increment(the_fifo, new_increment)
  generic_fifo the_fifo;
  int new_increment;
{
    int old_increment;

    GF_ASSERT("set_generic_fifo_increment", the_fifo != NULL);
    GF_ASSERT("set_generic_fifo_increment", new_increment > 0);

    old_increment = the_fifo->increment;
    the_fifo->increment = new_increment;
    return old_increment;
} /* set_generic_fifo_increment */

/*---------------------------------------------------------------------------*/

/*  Get the list in the FIFO queue, that is, the generic_list that is used to
 *  implement the generic_fifo.
 */
generic_list get_list_inside_generic_fifo(the_fifo)
  generic_fifo the_fifo;
{
    GF_ASSERT("get_list_inside_generic_fifo", the_fifo != NULL);

    return (generic_list)the_fifo;
} /* get_list_inside_generic_fifo */
