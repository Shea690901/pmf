/*  generic_fifo.c
 *  	-- #include file for the generic FIFO queue handling
 *  Thomas Padron-McCarthy (padrone@lysator.liu.se), 1991
 *  Share and enjoy, but be nice: don't steal my program! Hugo is watching!
 *  This file latest updated: Sept 21, 1991
 */

#ifndef GENERIC_FIFO_INCLUDE_INCLUDED

#define GENERIC_FIFO_INCLUDE_INCLUDED

#ifndef GENERIC_LIST_INCLUDE_INCLUDED
#include "generic_list.h"
#endif

typedef generic_list generic_fifo;

extern generic_fifo create_generic_fifo();
extern void kill_generic_fifo();
extern element_pointer_type find_in_generic_fifo();
extern element_pointer_type remove_from_generic_fifo();
extern void queue_on_generic_fifo();
extern element_pointer_type get_from_generic_fifo();
extern element_pointer_type find_first_in_of_generic_fifo();
extern element_pointer_type find_last_in_generic_fifo();
extern element_pointer_type traverse_generic_fifo();
extern element_pointer_type traverse_generic_fifo_backwards();
extern int query_generic_fifo_size();
extern int set_generic_fifo_increment();
extern generic_list get_list_inside_generic_fifo();

#endif
