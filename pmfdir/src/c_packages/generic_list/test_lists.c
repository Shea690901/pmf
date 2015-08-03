/*  test_lists.c
 *	-- a program to test the list, set and fifo package
 *  Thomas Padron-McCarthy (padrone@lysator.liu.se), 1991
 *  Share and enjoy, but be nice: don't steal my program! Hugo is watching!
 *  This file latest updated: Sept 21, 1991
 */

#include <stdio.h>
#include <strings.h>
#include "safe_malloc.h"
#include "generic_list.h"
#include "generic_set.h"
#include "generic_fifo.h"

#define ASSERT(what)       		\
        {                               \
                if (!(what))            \
                        fatal("test_lists: ASSERT failed in \"%s\" line %d: %s",	\
                              __FILE__, __LINE__, "what");                     	\
        }

#define NR_TESTS 6000

generic_list l1, l2, l3, l4, l5;
generic_set s1, s2, s3, s4, s5;
generic_fifo f1, f2, f3, f4, f5;

struct element_struct {
    char *key, *data;
    int intdata;
};

int toss_coin(sides)
int sides;
{
    return rand() % sides;
}

int compare_elements(struct1p, struct2p)
struct element_struct *struct1p, *struct2p;
{
    return strcmp(struct1p->key, struct2p->key);
} /* compare_elements */

int print_element(ep)
struct element_struct *ep;
{
    printf(" %d", ep->intdata);
    return NULL;
} /* print_element */

int print_list(the_list)
generic_list the_list;
{
    printf("%d elements: (", query_generic_list_size(the_list));
    traverse_generic_list(the_list, print_element);
    printf(" )\n");
} /* print_list */

struct element_struct *new_element(t)
char *t;
{
    static int nr = 0;
    static char buf[100];
    struct element_struct *ep;

    sprintf(buf, "Key string of element %d (%s)", nr, t);
    ep = (struct element_struct *)
	safe_malloc(sizeof(struct element_struct));
    ep->key = strcpy(safe_malloc(strlen(buf) + 1), buf);

    sprintf(buf, "Data string of element %d (%s)", nr, t);
    ep->data = strcpy(safe_malloc(strlen(buf) + 1), buf);

    ep->intdata = nr;

    ++nr;
    return ep;
} /* new_element */

void free_element(ep)
struct element_struct *ep;
{
    ASSERT(ep != NULL);
    ASSERT(ep->key != NULL);
    ASSERT(ep->data != NULL);

    safe_free(ep);
    safe_free(ep->key);
    safe_free(ep->data);
} /* free_element */

init() {

    printf("Creating lists...\n");
    l1 = create_generic_list(compare_elements);
    set_generic_list_increment(l1, 1);
    l2 = create_generic_list(compare_elements);
    set_generic_list_increment(l2, 2);
    l3 = create_generic_list(compare_elements);
    set_generic_list_increment(l3, 3);
/*
    l4 = create_generic_list((char *)NULL);
*/
    l4 = create_generic_list(compare_elements);
    set_generic_list_increment(l4, 4);
    l5 = create_generic_list(compare_elements);
    set_generic_list_increment(l5, 17000);

    printf("Creating sets...\n");
    s1 = create_generic_set(compare_elements);
    set_generic_set_increment(s1, 1);
    s2 = create_generic_set(compare_elements);
    set_generic_set_increment(s2, 2);
    s3 = create_generic_set(compare_elements);
    set_generic_set_increment(s3, 3);
/*
    s4 = create_generic_set((char *)NULL);
*/
    s4 = create_generic_set(compare_elements);
    set_generic_set_increment(s4, 4);

    s5 = create_generic_set(compare_elements);
    set_generic_set_increment(s5, 17000);

    printf("Creating fifos...\n");
    f1 = create_generic_fifo(compare_elements);
    set_generic_fifo_increment(f1, 1);
    f2 = create_generic_fifo(compare_elements);
    set_generic_fifo_increment(f2, 2);
    f3 = create_generic_fifo(compare_elements);
    set_generic_fifo_increment(f3, 3);
/*
    f4 = create_generic_fifo((char *)NULL);
*/
    f4 = create_generic_fifo(compare_elements);
    set_generic_fifo_increment(f4, 4);
    f5 = create_generic_fifo(compare_elements);
    set_generic_fifo_increment(f5, 17000);

} /* init */

fill()
{
    printf("Filling lists...\n");
    fill_list(l1, "l1");
    fill_list(l2, "l2");
    fill_list(l3, "l3");
    fill_list(l4, "l4");
    fill_list(l5, "l5");

    printf("Filling sets...\n");
    fill_set(s1, "s1");
    fill_set(s2, "s2");
    fill_set(s3, "s3");
    fill_set(s4, "s4");
    fill_set(s5, "s5");

    printf("Filling fifos...\n");
    fill_fifo(f1, "f1");
    fill_fifo(f2, "f2");
    fill_fifo(f3, "f3");
    fill_fifo(f4, "f4");
    fill_fifo(f5, "f5");
} /* fill */

fill_list(l, t)
generic_list l;
char *t;
{
    int i;
    for (i = 0; i < NR_TESTS; ++i) {
	ASSERT(query_generic_list_size(l) == i);
	if (toss_coin(2))
	    add_first_to_generic_list(l, new_element(t));
	else
	    add_last_to_generic_list(l, new_element(t));
	ASSERT(query_generic_list_size(l) == i + 1);
    }
    ASSERT(query_generic_list_size(l) == NR_TESTS);
} /* fill_list */

empty_list(l)
generic_list l;
{
    int i;
    struct element_struct *ep1, *ep2;

    for (i = 0; i < NR_TESTS; ++i) {
	ASSERT(query_generic_list_size(l) == NR_TESTS - i);
	switch (toss_coin(3)) {
	  case 0:
	    ep2 = (struct element_struct *)
		remove_first_from_generic_list(l);
	    free_element(ep2);
	    break;
	  case 1:
	    ep2 = (struct element_struct *)
		remove_last_from_generic_list(l);
	    free_element(ep2);
	    break;
	  case 2:
	    ep1 = (struct element_struct *)
		find_element_at_pos_in_generic_list(l, toss_coin(query_generic_list_size(l)));
	    ep2 = (struct element_struct *)
		remove_from_generic_list(l, ep1);
	    ASSERT (ep1 == ep2);
	    free_element(ep2);
	    break;
	}
	ASSERT(query_generic_list_size(l) == NR_TESTS - i - 1);
    }
    ASSERT(query_generic_list_size(l) == 0);
    ASSERT(find_first_in_generic_list(l) == NULL);
    ASSERT(find_last_in_generic_list(l) == NULL);
} /* empty_list */

fill_set(s, t)
generic_set s;
char *t;
{
    int i;
    for (i = 0; i < NR_TESTS; ++i) {
	ASSERT(query_generic_set_size(s) == i);
	add_to_generic_set(s, new_element(t));
	ASSERT(query_generic_set_size(s) == i + 1);
    }
    ASSERT(query_generic_set_size(s) == NR_TESTS);
} /* fill_set */

empty_set(s)
generic_set s;
{
    int i;
    struct element_struct *ep1, *ep2;

    for (i = 0; i < NR_TESTS; ++i) {
	ASSERT(query_generic_set_size(s) == NR_TESTS - i);

	ep1 = (struct element_struct *)
	    find_element_at_pos_in_generic_list(get_list_inside_generic_set(s),
						toss_coin(query_generic_set_size(s)));
	ep2 = (struct element_struct *)
	    remove_from_generic_set(s, ep1);
	ASSERT (ep1 == ep2);
	free_element(ep2);
	ASSERT(query_generic_set_size(s) == NR_TESTS - i - 1);
    }
    ASSERT(query_generic_set_size(s) == 0);
} /* empty_set */

fill_fifo(f, t)
generic_fifo f;
char *t;
{
    int i;
    for (i = 0; i < NR_TESTS; ++i) {
	ASSERT(query_generic_fifo_size(f) == i);
	queue_on_generic_fifo(f, new_element(t));
	ASSERT(query_generic_fifo_size(f) == i + 1);
    }
    ASSERT(query_generic_fifo_size(f) == NR_TESTS);
} /* fill_fifo */

empty_fifo(f)
generic_fifo f;
{
    int i;
    struct element_struct *ep1, *ep2;

    for (i = 0; i < NR_TESTS; ++i) {
	ASSERT(query_generic_fifo_size(f) == NR_TESTS - i);

	if (toss_coin(2)) {
	    ep1 = (struct element_struct *)
		find_element_at_pos_in_generic_list(get_list_inside_generic_fifo(f),
						    toss_coin(query_generic_fifo_size(f)));
	    ep2 = (struct element_struct *)
		remove_from_generic_fifo(f, ep1);
	    ASSERT (ep1 == ep2);
	    free_element(ep2);
	}
	else {
	    ep2 = (struct element_struct *)
		get_from_generic_fifo(f);
	    free_element(ep2);
	}

	ASSERT(query_generic_fifo_size(f) == NR_TESTS - i - 1);
    }
    ASSERT(query_generic_fifo_size(f) == 0);
    ASSERT(get_from_generic_fifo(f) == NULL);
} /* empty_fifo */


empty()
{
    printf("Emptying lists...\n");
    empty_list(l1);
    empty_list(l2);
    empty_list(l3);
    empty_list(l4);
    empty_list(l5);

    printf("Emptying sets...\n");
    empty_set(s1);
    empty_set(s2);
    empty_set(s3);
    empty_set(s4);
    empty_set(s5);

    printf("Emptying fifos...\n");
    empty_fifo(f1);
    empty_fifo(f2);
    empty_fifo(f3);
    empty_fifo(f4);
    empty_fifo(f5);
} /* empty */


cleanup()
{

    printf("Killing lists...\n");
    kill_generic_list(l1);
    kill_generic_list(l2);
    kill_generic_list(l3);
    kill_generic_list(l4);
    kill_generic_list(l5);

    printf("Killing sets...\n");
    kill_generic_set(s1);
    kill_generic_set(s2);
    kill_generic_set(s3);
    kill_generic_set(s4);
    kill_generic_set(s5);

    printf("Killing fifos...\n");
    kill_generic_fifo(f1);
    kill_generic_fifo(f2);
    kill_generic_fifo(f3);
    kill_generic_fifo(f4);
    kill_generic_fifo(f5);
} /* cleanup */

main()
{
    char *cp;

    srand(0);

    init();
    fill();
    empty();
    cleanup();

#ifdef DEBUG_MALLOC
    dump_malloc();
#endif

    exit(0);
} /* main */
