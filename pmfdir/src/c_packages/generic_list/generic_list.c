/*  generic_list.c -- generic list handling
 *  Thomas Padron-McCarthy (padrone@lysator.liu.se), 1991
 *  Share and enjoy, but be nice: don't steal my program! Hugo is watching!
 *  This file latest updated: Sept 21, 1991
 *
 *	This is a package for handling lists of something.
 *	We call these somethings "objects" (or sometimes "elements").
 *	The objects are referred to by pointers, and this package
 *	manipulates those pointers, never the objects themselves.
 *
 *	Each object must have a "comparable" property. It is not
 *	necessary to be able to know if two objects are smaller or greater
 *	than each other, only if they are equal or not.
 */

#include "generic_list.h"
#include "safe_malloc.h"

#define NULL 0
#ifdef DEBUG
#   define GL_ASSERT(where, what)	\
	{				\
		if (!(what))		\
			fatal("%s: ASSERT failed in \"%s\" line %d: %s",	\
			      where, __FILE__, __LINE__, "what");		\
	}
#else
#   define GL_ASSERT(where, what)	(what)
#endif

/*---------------------------------------------------------------------------*/

/*  We don't know how memcpy handles overlapping data,
 *  so we do our own copying of pointers using the two functions
 *  "copy_pointers_up" and "copy_pointers_down".
 *  "copy_pointers_down" should be used when copying from an area
 *  to an overlapping area with a LOWER address.
 *  "copy_pointers_up" should be used when copying from an area
 *  to an overlapping area with a HIGHER address.
 *  The function "copy_pointers" can decides for itself which of
 *  those two functions to use.
 *  (I was trying to write efficient code. Don't laugh.)
 */

void copy_pointers_up(to_pp, from_pp, nr_pointers)
register any_pointer *to_pp, *from_pp;
register int nr_pointers;
{
    register any_pointer *end_pp;

    GL_ASSERT("copy_pointers_up", to_pp != NULL);
    GL_ASSERT("copy_pointers_up", from_pp != NULL);
    GL_ASSERT("copy_pointers_up", to_pp < from_pp);

    end_pp = from_pp + nr_pointers;
    while (from_pp != end_pp)
	*to_pp++ = *from_pp++;
} /* copy_pointers_up */

void copy_pointers_down(to_pp, from_pp, nr_pointers)
register any_pointer *to_pp, *from_pp;
register int nr_pointers;
{
    register any_pointer *end_pp;
    
    GL_ASSERT("copy_pointers_down", to_pp != NULL);
    GL_ASSERT("copy_pointers_down", from_pp != NULL);
    GL_ASSERT("copy_pointers_down", to_pp > from_pp);

    end_pp = from_pp - 1;
    from_pp = from_pp + nr_pointers - 1;
    to_pp = to_pp + nr_pointers - 1;
    while (from_pp != end_pp)
	*to_pp-- = *from_pp--;
} /* copy_pointers_down */

void copy_pointers(to_pp, from_pp, nr_pointers)
register any_pointer *to_pp, *from_pp;
register int nr_pointers;
{
    GL_ASSERT("copy_pointers_down", to_pp != NULL);
    GL_ASSERT("copy_pointers_down", from_pp != NULL);

    if (to_pp < from_pp)
	copy_pointers_up(to_pp, from_pp, nr_pointers);
    else
	copy_pointers_down(to_pp, from_pp, nr_pointers);
} /* copy_pointers */

/*---------------------------------------------------------------------------*/

static int find_index(the_list, object_to_search_for_p)
  register generic_list the_list;
  any_pointer object_to_search_for_p;
{
    register int i, n;
    int (*f)();
    any_pointer *b;

    GL_ASSERT("find_index", the_list != NULL);
    GL_ASSERT("find_index", object_to_search_for_p != NULL);
    GL_ASSERT("find_index", the_list->compare_function != NULL);

    n = the_list->current_number;
    b = the_list->buffer_of_pointers;
    f = the_list->compare_function;
    for (i = 0; i < n; ++i)
	if ((*f)(b[i], object_to_search_for_p) == 0)
	    return i;
    return -1;
} /* find_index */

/*---------------------------------------------------------------------------*/

/*  Allocate space in the list for one new element.
 *  If necessary, we will expand the buffer of pointers.
 */
static void expand_list(the_list)
  generic_list the_list;
{
    ++the_list->current_number;
    if (the_list->current_number > the_list->current_max_number) {
	the_list->current_max_number += the_list->increment;
	if (the_list->buffer_of_pointers)
	    the_list->buffer_of_pointers = (any_pointer *)
		safe_realloc(the_list->buffer_of_pointers,
			     the_list->current_max_number * sizeof(any_pointer));
	else
	    the_list->buffer_of_pointers = (any_pointer *)
		safe_malloc(the_list->current_max_number * sizeof(any_pointer));
    }
} /* expand_list */

/*---------------------------------------------------------------------------*/

/*  If this was the last object in the list, and the buffer of pointers
 *  previously has grown and is bigger than the minimum size,
 *  then free that buffer.
 */
static void check_empty_buffer(the_list)
generic_list the_list;
{

    if (   !the_list->current_number
	&& the_list->current_max_number > the_list->increment) {
	safe_free(the_list->buffer_of_pointers);
	the_list->current_max_number = 0;
	the_list->buffer_of_pointers = NULL;
    }
} /* check_empty_buffer */

/*---------------------------------------------------------------------------*/

/*  To create a generic list, call the function create_generic_list with
 *  a compare function (a pointer to a function returning integer) as
 *  argument. This function should take as arguments pointers to two
 *  somethings, compare them, and return negative, 0, or positive.
 *  The new generic list, of type generic_list, is returned.
 *  It is possible to give NULL as compare_function,
 *  but then you can not search the list.
 */
generic_list create_generic_list(compare_function)
int (*compare_function)();
{
    struct generic_list_struct *this_list;

    this_list = (struct generic_list_struct *)
	safe_malloc(sizeof(struct generic_list_struct));
    this_list->buffer_of_pointers = NULL;
    this_list->current_number = 0;
    this_list->current_max_number = 0;
    this_list->increment = 8;	/* Can be changed to anything reasonable. */
    this_list->compare_function = compare_function;
    return this_list;
} /* create_generic_list */

/*---------------------------------------------------------------------------*/

/*  Destroy a generic list. It frees the generic_list structure and the buffer
 *  of pointers used to point to the objects in the list, but the objects
 *  themselves, if there are any in the list, are not affected.
 */
void kill_generic_list(the_list)
  register generic_list the_list;
{

    GL_ASSERT("kill_generic_list", the_list != NULL);

    if (the_list->buffer_of_pointers)
	safe_free(the_list->buffer_of_pointers);
    safe_free(the_list);
} /* kill_generic_list */

/*---------------------------------------------------------------------------*/

/*  Remove all elements from a generic list.
 *  If function_to_call is not NULL, this function is called before
 *  removing each element, with that element as argument, and if it returns
 *  non-NULL, the emptying is aborted, and that non-NULL value is returned.
 */
any_pointer empty_generic_list(the_list, function_to_call)
  register generic_list the_list;
  any_pointer (*function_to_call)();
{
    any_pointer this_element, funret;

    GL_ASSERT("empty_generic_list", the_list != NULL);

    while ((this_element = find_first_in_generic_list(the_list)) != NULL) {
	if (   function_to_call
	    && (funret = (*function_to_call)(this_element)) != NULL)
	    return funret;
	remove_first_from_generic_list(the_list);
    }
    return NULL;
} /* empty_generic_list */

/*---------------------------------------------------------------------------*/

/*  This function searches in a generic list for an object matching
 *  the object pointed to by object_to_search_for.
 *  A pointer to the (first) found object is returned, or NULL is returned.
 */
any_pointer find_in_generic_list(the_list, object_to_search_for)
  register generic_list the_list;
  any_pointer object_to_search_for;
{
    int i;

    GL_ASSERT("find_in_generic_list", the_list != NULL);
    GL_ASSERT("find_in_generic_list", object_to_search_for != NULL);
    GL_ASSERT("find_in_generic_list", the_list->compare_function != NULL);

    i = find_index(the_list, object_to_search_for);
    if (i == -1)
	return NULL;
    else
	return the_list->buffer_of_pointers[i];
} /* find_in_generic_list */

/*---------------------------------------------------------------------------*/

/*  This function returns the first object in a generic list.
 *  A pointer to the object is returned.
 *  If the list is empty, NULL is returned.
 */
any_pointer find_first_in_generic_list(the_list)
  register generic_list the_list;
{

    GL_ASSERT("find_first_in_generic_list", the_list != NULL);

    if (the_list->current_number > 0)
	return the_list->buffer_of_pointers[0];
    else
	return NULL;
} /* find_first_in_generic_list */

/*---------------------------------------------------------------------------*/

/*  This function returns the last object in a generic list.
 *  A pointer to the object is returned.
 *  If the list is empty, NULL is returned.
 */
any_pointer find_last_in_generic_list(the_list)
  register generic_list the_list;
{

    GL_ASSERT("find_last_in_generic_list", the_list != NULL);

    if (the_list->current_number > 0)
	return the_list->buffer_of_pointers[the_list->current_number - 1];
    else
	return NULL;
} /* find_last_in_generic_list */

/*---------------------------------------------------------------------------*/

/*  This function searches in a generic list for the first object matching
 *  the object pointed to by object_to_search_for, and removes it.
 *  It returns a pointer to the object that was removed from the list.
 */
any_pointer remove_from_generic_list(the_list, object_to_remove)
  register generic_list the_list;
  any_pointer object_to_remove;
{
    int i;
    any_pointer pointer_to_it;

    GL_ASSERT("remove_from_generic_list", the_list != NULL);
    GL_ASSERT("remove_from_generic_list", object_to_remove != NULL);

    i = find_index(the_list, object_to_remove);
    if (i == -1)
	return NULL;
    pointer_to_it = the_list->buffer_of_pointers[i];
    copy_pointers_up(&the_list->buffer_of_pointers[i],
		     &the_list->buffer_of_pointers[i + 1],
		     the_list->current_number - i - 1);

    the_list->current_number--;
    check_empty_buffer(the_list);
    return pointer_to_it;
} /* remove_from_generic_list */

/*---------------------------------------------------------------------------*/

/*  Removes the first object from a generic list.
 *  It returns a pointer to the object that was removed.
 *  If the list was empty, NULL is returned.
 */
any_pointer remove_first_from_generic_list(the_list)
  register generic_list the_list;
{
    any_pointer pointer_to_it;

    GL_ASSERT("remove_first_from_generic_list", the_list != NULL);

    if (the_list->current_number == 0)
	return NULL;
    pointer_to_it = the_list->buffer_of_pointers[0];
    copy_pointers_up(&the_list->buffer_of_pointers[0],
		     &the_list->buffer_of_pointers[1],
		     the_list->current_number - 1);

    the_list->current_number--;
    check_empty_buffer(the_list);
    return pointer_to_it;
} /* remove_first_from_generic_list */

/*---------------------------------------------------------------------------*/

/*  Removes the last object from a generic list.
 *  It returns a pointer to the object that was removed.
 *  If the list was empty, NULL is returned.
 */
any_pointer remove_last_from_generic_list(the_list)
  register generic_list the_list;
{
    any_pointer pointer_to_it;

    GL_ASSERT("remove_last_from_generic_list", the_list != NULL);

    if (the_list->current_number == 0)
	return NULL;
    pointer_to_it = the_list->buffer_of_pointers[the_list->current_number - 1];
    the_list->current_number--;

    /* No need to copy anything here. */

    check_empty_buffer(the_list);
    return pointer_to_it;
} /* remove_last_from_generic_list */

/*---------------------------------------------------------------------------*/

/*  Add an object, pointed to by the_object_p, to a generic list,
 *  as the first element in that list.
 *  The new number of elements in the list is returned.
 */
int add_first_to_generic_list(the_list, the_object_p)
  register generic_list the_list;
  any_pointer the_object_p;
{

    GL_ASSERT("add_first_to_generic_list", the_list != NULL);
    GL_ASSERT("add_first_to_generic_list", the_object_p != NULL);

    expand_list(the_list);
    copy_pointers_down(&the_list->buffer_of_pointers[1],
		       &the_list->buffer_of_pointers[0],
		       the_list->current_number - 1);

    the_list->buffer_of_pointers[0] = the_object_p;
    return the_list->current_number;
} /* add_first_to_generic_list */

/*---------------------------------------------------------------------------*/

/*  Add an object, pointed to by the_object_p, to a generic list,
 *  as the last element in that list.
 *  The new number of elements in the list is returned.
 */
int add_last_to_generic_list(the_list, the_object_p)
  register generic_list the_list;
  any_pointer the_object_p;
{

    GL_ASSERT("add_last_to_generic_list", the_list != NULL);
    GL_ASSERT("add_last_to_generic_list", the_object_p != NULL);

    expand_list(the_list);
    the_list->buffer_of_pointers[the_list->current_number - 1] = the_object_p;
    return the_list->current_number;
} /* add_last_to_generic_list */

/*---------------------------------------------------------------------------*/

/*  This function returns the position in a generic list of the (first)
 *  object matching the object pointed to by object_to_search_for.
 *  The position is in the interval 0..N-1, where N is the current size
 *  of the list.
 *  If no matching element is found, or the list is empty, -1 is returned.
 */
int find_pos_of_element_in_generic_list(the_list, object_to_search_for)
  register generic_list the_list;
  any_pointer object_to_search_for;
{

    GL_ASSERT("find_pos_of_element_in_generic_list", the_list != NULL);
    GL_ASSERT("find_pos_of_element_in_generic_list", object_to_search_for != NULL);
    GL_ASSERT("find_pos_of_element_in_generic_list", the_list->compare_function != NULL);

    return find_index(the_list, object_to_search_for);
} /* find_pos_of_element_in_generic_list */

/*---------------------------------------------------------------------------*/

/*  This function returns the element at position the_pos in a generic list.
 *  The position should be in the interval 0..N-1, where N is the current
 *  size of the list.
 *  If the position is outside that interval, or the list is empty,
 *  NULL is returned.
 */
any_pointer find_element_at_pos_in_generic_list(the_list, the_pos)
  register generic_list the_list;
  int the_pos;
{

    GL_ASSERT("find_element_at_pos_in_generic_list", the_list != NULL);

    if (the_pos >= 0 && the_pos < the_list->current_number)
	return the_list->buffer_of_pointers[the_pos];
    else
	return NULL;
} /* find_element_at_pos_in_generic_list */

/*---------------------------------------------------------------------------*/

/*  This function calls the function function_to_call once for each object
 *  in the list, with a pointer to that object as first argument.
 *  This is done as long as the function_to_call returns NULL; if it returns
 *  another value, traverse_generic_list will stop, and return that value.
 *  If that never happens, traverse_generic_list returns NULL.
 *  WARNING: Do not change the list in the function function_to_call!
 *  The third argument to this function, optional_second_argument, will be
 *  passed to function_to_call as its second argument.
 */
any_pointer traverse_generic_list(the_list, function_to_call,
				  optional_second_argument)
  generic_list the_list;
  any_pointer (*function_to_call)();
  char *optional_second_argument;
{
    register int i, n;
    any_pointer *b;
    any_pointer ret_val;

    GL_ASSERT("traverse_generic_list", the_list != NULL);
    GL_ASSERT("traverse_generic_list", function_to_call != NULL);

    n = the_list->current_number;
    b = the_list->buffer_of_pointers;
    for (i = 0; i < n; ++i) {
	ret_val = (*function_to_call)(b[i], optional_second_argument);
	if (ret_val)
	    return ret_val;
    }
    return NULL;
} /* traverse_generic_list */

/*---------------------------------------------------------------------------*/

/*  As traverse_generic_list, but this function traverses the list backwards.
 */
any_pointer traverse_generic_list_backwards(the_list, function_to_call,
					    optional_second_argument)
  generic_list the_list;
  any_pointer (*function_to_call)();
  char *optional_second_argument;
{
    register int i, n;
    any_pointer *b;
    any_pointer ret_val;

    GL_ASSERT("traverse_generic_list_backwards", the_list != NULL);
    GL_ASSERT("traverse_generic_list_backwards", function_to_call != NULL);

    n = the_list->current_number;
    b = the_list->buffer_of_pointers;
    for (i = n - 1; i >= 0; --i) {
	ret_val = (*function_to_call)(b[i], optional_second_argument);
	if (ret_val)
	    return ret_val;
    }
    return NULL;
} /* traverse_generic_list_backwards */

/*---------------------------------------------------------------------------*/

int query_generic_list_size(the_list)
  generic_list the_list;
{

    GL_ASSERT("query_generic_list_size", the_list != NULL);

    return the_list->current_number;
} /* query_generic_list_size */

/*---------------------------------------------------------------------------*/

/*  Set the increment. The old increment is returned.
 */
int set_generic_list_increment(the_list, new_increment)
  generic_list the_list;
  int new_increment;
{
    int old_increment;

    GL_ASSERT("set_generic_list_increment", the_list != NULL);
    GL_ASSERT("set_generic_list_increment", new_increment > 0);

    old_increment = the_list->increment;
    the_list->increment = new_increment;
    return old_increment;
} /* set_generic_list_increment */

/*---------------------------------------------------------------------------*/

/*  Get the array in the list, that is, a pointer to the buffer of
 *  any_pointers that is part of the implementation of the generic_list.
 */
any_pointer *get_array_inside_generic_list(the_list)
  generic_list the_list;
{
    GL_ASSERT("get_array_inside_generic_list", the_list != NULL);

    return the_list->buffer_of_pointers;
} /* get_array_inside_generic_list */
