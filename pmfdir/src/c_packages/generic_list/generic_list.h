/*  generic_list.h
 *      -- #include file for the generic list handling
 *  Thomas Padron-McCarthy (padrone@lysator.liu.se), 1991
 *  Share and enjoy, but be nice: don't steal my program! Hugo is watching!
 *  This file latest updated: Sept 21, 1991
 */

#ifndef GENERIC_LIST_INCLUDE_INCLUDED

#define GENERIC_LIST_INCLUDE_INCLUDED

#ifndef ELEMENT_POINTER_TYPE
#  define ELEMENT_POINTER_TYPE char *
#endif

typedef char *any_pointer;
typedef ELEMENT_POINTER_TYPE element_pointer_type;

struct generic_list_struct {
    any_pointer *buffer_of_pointers;
    int current_number, current_max_number, increment;
    int (*compare_function)();
};

typedef struct generic_list_struct *generic_list;

extern generic_list create_generic_list();
extern void kill_generic_list();
extern element_pointer_type empty_generic_list();
extern element_pointer_type find_in_generic_list();
extern element_pointer_type find_first_in_generic_list();
extern element_pointer_type find_last_in_generic_list();
extern element_pointer_type remove_from_generic_list();
extern element_pointer_type remove_first_from_generic_list();
extern element_pointer_type remove_last_from_generic_list();
extern int add_first_to_generic_list();
extern int add_last_to_generic_list();
extern int get_pos_of_element_in_generic_list();
extern element_pointer_type find_element_at_pos_in_generic_list();
extern element_pointer_type traverse_generic_list();
extern element_pointer_type traverse_generic_list_backwards();
extern int query_generic_list_size();
extern int set_generic_list_increment();
extern element_pointer_type *get_array_inside_generic_list();

#endif
