/*  generic_set.c -- generic set handling
 *  Thomas Padron-McCarthy (padrone@lysator.liu.se), 1991
 *  Share and enjoy, but be nice: don't steal my program! Hugo is watching!
 *  This file latest updated: Sept 21, 1991
 *
 *	This is a package for handling sets of something.
 *	We call these somethings "objects" (or sometimes "elements").
 *	The objects are referred to by pointers, and this package
 *	manipulates those pointers, never the objects themselves.
 *
 *	Each object must have an "orderable" property that can be compared by
 *	a function. The objects in the set are sorted with respect to that
 *	property, and a function to compare objects must be supplied.
 */

#include "generic_set.h"
#include "safe_malloc.h"

#define NULL 0
#ifdef DEBUG
#   define GS_ASSERT(where, what)	\
	{				\
		if (!(what))		\
			fatal("%s: ASSERT failed in \"%s\" line %d: %s",	\
			      where, __FILE__, __LINE__, "what");		\
	}
#else
#   define GS_ASSERT(where, what)	(what)
#endif

/*---------------------------------------------------------------------------*/

static int find_index(the_set, object_to_search_for_p)
  register generic_set the_set;
  any_pointer object_to_search_for_p;
{
    register int begin, end, i, last_i, r;
    int (*f)();
    any_pointer *b;

    GS_ASSERT("find_index", the_set != NULL);
    GS_ASSERT("find_index", object_to_search_for_p != NULL);
    GS_ASSERT("find_index", the_set->compare_function != NULL);

    /* The set is sorted, so we'll do a binary search. */

    f = the_set->compare_function;
    b = the_set->buffer_of_pointers;
    begin = 0;
    end = the_set->current_number;
    i = -1;	/* For the last_i check! */
    while (end != begin) {
	last_i = i;
	i = begin + ((end - begin) / 2);
	if (i == last_i)
	    return -1;
	r = (*f)(b[i], object_to_search_for_p);
	if (r < 0)
	    begin = i;
	else if (r > 0)
	    end = i;
	else
	    return i;
    }
    return -1;
} /* find_index */

/*---------------------------------------------------------------------------*/

/*  If this was the last object in the set, and the buffer of pointers
 *  previously has grown and is bigger than the minimum size,
 *  then free that buffer.
 */
static void check_empty_buffer(the_set)
generic_set the_set;
{

    if (   !the_set->current_number
	&& the_set->current_max_number > the_set->increment) {
	safe_free(the_set->buffer_of_pointers);
	the_set->current_max_number = 0;
	the_set->buffer_of_pointers = NULL;
    }
} /* check_empty_buffer */

/*---------------------------------------------------------------------------*/

/*  To create a generic set, call the function create_generic_set with
 *  a compare function (a pointer to a function returning integer) as
 *  argument. This function should take as arguments pointers to two
 *  somethings, compare them, and return negative, 0, or positive.
 *  The new generic set, of type generic_set, is returned.
 *  It is possible to give NULL as compare_function,
 *  but then you can not search the set.
 */
generic_set create_generic_set(compare_function)
int (*compare_function)();
{
    generic_set this_set;

    GS_ASSERT("create_generic_set", compare_function != NULL);

    this_set = (generic_set)create_generic_list(compare_function);
    return this_set;
} /* create_generic_set */

/*---------------------------------------------------------------------------*/

/*  Destroy a generic set. It frees the generic_set structure and the buffer
 *  of pointers used to point to the objects in the set, but the objects
 *  themselves, if there are any in the set, are not affected.
 */
void kill_generic_set(the_set)
  register generic_set the_set;
{

    GS_ASSERT("kill_generic_set", the_set != NULL);

    kill_generic_list((generic_list)the_set);
} /* kill_generic_set */

/*---------------------------------------------------------------------------*/

/*  Remove all elements from a generic set. Works as empty_generic_list.
 */
any_pointer empty_generic_set(the_set, function_to_call)
  register generic_set the_set;
  any_pointer (*function_to_call)();
{

    GS_ASSERT("empty_generic_set", the_set != NULL);

    return empty_generic_list((generic_list)the_set, function_to_call);
} /* empty_generic_set */

/*---------------------------------------------------------------------------*/

/*  This function searches in a generic set for an object matching
 *  the object pointed to by object_to_search_for.
 *  A pointer to the (first) found object is returned, or NULL is returned.
 */
any_pointer find_in_generic_set(the_set, object_to_search_for_p)
  register generic_set the_set;
  any_pointer object_to_search_for_p;
{
    int i;

    GS_ASSERT("find_in_generic_set", the_set != NULL);
    GS_ASSERT("find_in_generic_set", object_to_search_for_p != NULL);
    GS_ASSERT("find_in_generic_set", the_set->compare_function != NULL);

    i = find_index(the_set, object_to_search_for_p);
    if (i == -1)
	return NULL;
    else
	return the_set->buffer_of_pointers[i];
} /* find_in_generic_set */

/*---------------------------------------------------------------------------*/

/*  This function searches in a generic set for an object matching
 *  the object pointed to by object_to_search_for, and removes it.
 *  It returns a pointer to the object that was removed from the set.
 */
any_pointer remove_from_generic_set(the_set, object_to_remove)
  register generic_set the_set;
  any_pointer object_to_remove;
{
    int i;
    any_pointer old_object_p;

    GS_ASSERT("find_in_generic_set", the_set != NULL);
    GS_ASSERT("find_in_generic_set", object_to_remove != NULL);
    GS_ASSERT("find_in_generic_set", the_set->compare_function != NULL);

    i = find_index(the_set, object_to_remove);
    if (i == -1)
	return NULL;
    old_object_p = the_set->buffer_of_pointers[i];
    copy_pointers_up(&the_set->buffer_of_pointers[i],
		     &the_set->buffer_of_pointers[i + 1],
		     the_set->current_number - i - 1);

    --the_set->current_number;
    check_empty_buffer(the_set);
    return old_object_p;
} /* remove_from_generic_set */

/*---------------------------------------------------------------------------*/

/*  Add an object, pointed to by the_object_p, to a generic set.
 *  If there was an old object, matching the new one, in the set,
 *  it is removed from the set, and a pointer to it is returned.
 *  Otherwise, NULL is returned.
 */
any_pointer add_to_generic_set(the_set, the_object_p)
  register generic_set the_set;
  any_pointer the_object_p;
{
    register int i, n;
    any_pointer old_object_p;
    int (*f)();
    any_pointer *b;

    GS_ASSERT("add_to_generic_set", the_set != NULL);
    GS_ASSERT("add_to_generic_set", the_object_p != NULL);

    i = find_index(the_set, the_object_p);
    if (i != -1) {
        old_object_p = the_set->buffer_of_pointers[i];
	the_set->buffer_of_pointers[i] = the_object_p;
        return old_object_p;
    }
    else {
	/* Mostly to make sure there is place for it: */
	add_last_to_generic_list(get_list_inside_generic_set(the_set), the_object_p);

	n = the_set->current_number;
	b = the_set->buffer_of_pointers;
	f = the_set->compare_function;

	i = 0;
	while ((*f)(b[i], the_object_p) < 0)
	    ++i;
	copy_pointers_down(&b[i + 1], &b[i], n - i - 1);

	b[i] = the_object_p;
        return NULL;
    }
} /* add_to_generic_set */

/*---------------------------------------------------------------------------*/

/*  This function calls the function function_to_call once for each object
 *  in the set, with a pointer to that object as first argument.
 *  This is done as long as the function_to_call returns NULL; if it returns
 *  another value, traverse_generic_set will stop, and return that value.
 *  If that never happens, traverse_generic_set returns NULL.
 *  WARNING: Do not change the set in the function function_to_call!
 *  The third argument to this function, optional_second_argument, will be
 *  passed to function_to_call as its second argument.
 */
any_pointer traverse_generic_set(the_set, function_to_call,
				  optional_second_argument)
  generic_set the_set;
  any_pointer (*function_to_call)();
  char *optional_second_argument;
{

    GS_ASSERT("traverse_generic_set", the_set != NULL);
    GS_ASSERT("traverse_generic_set", function_to_call != NULL);

    return traverse_generic_list((generic_list)the_set, function_to_call,
				 optional_second_argument);
} /* traverse_generic_set */

/*---------------------------------------------------------------------------*/

int query_generic_set_size(the_set)
  generic_set the_set;
{

    GS_ASSERT("query_generic_set_size", the_set != NULL);

    return the_set->current_number;
} /* query_generic_set_size */

/*---------------------------------------------------------------------------*/

/*  Set the increment. The old increment is returned.
 */
int set_generic_set_increment(the_set, new_increment)
  generic_set the_set;
  int new_increment;
{
    int old_increment;

    GS_ASSERT("set_generic_set_increment", the_set != NULL);
    GS_ASSERT("set_generic_set_increment", new_increment > 0);

    old_increment = the_set->increment;
    the_set->increment = new_increment;
    return old_increment;
} /* set_generic_set_increment */

/*---------------------------------------------------------------------------*/

/*  Get the list in the set, that is, the generic_list that is used to
 *  implement the generic_set.
 */
generic_list get_list_inside_generic_set(the_set)
  generic_set the_set;
{
    GS_ASSERT("get_list_inside_generic_set", the_set != NULL);

    return (generic_list)the_set;
} /* get_list_inside_generic_set */
