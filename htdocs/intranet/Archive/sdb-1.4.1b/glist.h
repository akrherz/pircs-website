/****************************************************************************
 * NCSA HDF                                                                 *
 * Software Development Group                                               *
 * National Center for Supercomputing Applications                          *
 * University of Illinois at Urbana-Champaign                               *
 * 605 E. Springfield, Champaign IL 61820                                   *
 *                                                                          *
 * For conditions of distribution and use, see the accompanying             *
 * COPYING file.                                                            *
 *                                                                          *
 ****************************************************************************/

/************************************************************************
  Credits:
         Original code is part of the public domain 'Generic List Library'
         by Keith Pomakis(kppomaki@jeeves.uwaterloo.ca)-Spring, 1994
         I have left it mostly intact.

  -George V.
 ************************************************************************/

#ifndef GENERIC_LIST_DEFINED
#define GENERIC_LIST_DEFINED

typedef struct GLE_struct {
    void                *pointer;
    struct GLE_struct   *previous;
    struct GLE_struct   *next;
} Generic_list_element;

typedef struct {
    Generic_list_element   *current;
    Generic_list_element   pre_element; 
    Generic_list_element   post_element;
    Generic_list_element   deleted_element;
    int                    (*lt)(void *a, void *b);
    unsigned int           num_of_elements;
} Generic_list_info;

typedef struct {
    Generic_list_info *info;
} Generic_list;

#define Generic_stack Generic_list
#define Generic_queue Generic_list

/* Function declarations */
void initialize_list(Generic_list *list);
void initialize_sorted_list(Generic_list *list, int (*lt)(void *a, void *b));
void destroy_list(Generic_list *list);
void add_to_beginning(Generic_list list, void *pointer);
void add_to_end(Generic_list list, void *pointer);
void add_to_list(Generic_list list, void *pointer);
void *remove_from_beginning(Generic_list list);
void *remove_from_end(Generic_list list);
void *remove_from_list(Generic_list list, void *pointer);
void remove_all(Generic_list list);
void *peek_at_beginning(Generic_list list);
void *peek_at_end(Generic_list list);

void *first_in_list(Generic_list list);
void *next_in_list(Generic_list list);
void *current_in_list(Generic_list list);
void *remove_current(Generic_list list);
void *previous_in_list(Generic_list list);
void *last_in_list(Generic_list list);
void reset_to_beginning(Generic_list list);
void reset_to_end(Generic_list list);

int num_of_objects(Generic_list list);
int is_empty(Generic_list list);
int is_in_list(Generic_list list, void *pointer);
Generic_list copy_list(Generic_list list);

void perform_on_list
     (Generic_list list, void (*fn)(void *pointer, void *args), void *args);
void *first_that
     (Generic_list list, int (*fn)(void *pointer, void *args), void *args);
void *next_that
     (Generic_list list, int (*fn)(void *pointer, void *args), void *args);
void *previous_that
     (Generic_list list, int (*fn)(void *pointer, void *args), void *args);
void *last_that
     (Generic_list list, int (*fn)(void *pointer, void *args), void *args);
Generic_list all_such_that
     (Generic_list list, int (*fn)(void *pointer, void *args), void *args);
void remove_all_such_that
     (Generic_list list, int (*fn)(void *pointer, void *args), void *args);


/****************************************************************************/

/* Stack operations */

#define initialize_stack initialize_list
#define destroy_stack    destroy_list
#define push             add_to_beginning
#define pop              remove_from_beginning
#define pop_all          remove_all
#define peek_at_top      peek_at_beginning
#define copy_stack       copy_list

/* Queue operations */

#define initialize_queue initialize_list
#define destroy_queue    destroy_list
#define enqueue          add_to_end
#define dequeue          remove_from_beginning
#define dequeue_all      remove_all
#define peek_at_head     peek_at_beginning
#define peek_at_tail     peek_at_end
#define copy_queue       copy_list

#endif /* GENERIC_LIST_DEFINED */

