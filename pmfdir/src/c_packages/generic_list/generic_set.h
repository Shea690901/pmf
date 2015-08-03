/*  generic_list.h
 *      -- #include file for the generic set handling
 *  Thomas Padron-McCarthy (padrone@lysator.liu.se), 1991
 *  Share and enjoy, but be nice: don't steal my program! Hugo is watching!
 *  This file latest updated: Sept 21, 1991
 */

#ifndef GENERIC_SET_INCLUDE_INCLUDED

#define GENERIC_SET_INCLUDE_INCLUDED

#ifndef GENERIC_LIST_INCLUDE_INCLUDED
#include "generic_list.h"
#endif

typedef generic_list generic_set;

extern generic_set create_generic_set();
extern void kill_generic_set();
extern element_pointer_type empty_generic_set();
extern element_pointer_type find_in_generic_set();
extern element_pointer_type remove_from_generic_set();
extern element_pointer_type add_to_generic_set();
extern element_pointer_type traverse_generic_set();
extern int query_generic_set_size();
extern int set_generic_set_increment();
extern generic_list get_list_inside_generic_set();

#endif
