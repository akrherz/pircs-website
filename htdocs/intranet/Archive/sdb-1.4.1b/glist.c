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

#ifdef RCSID
static char RcsId[] = "@(#)glist.c,v 1.5 1996/04/15 17:57:33 georgev Exp";
#endif

#include <stdio.h>
#include <stdlib.h>
#include "glist.h"

/* macintosh */
#if defined SYMANTEC_C || defined __MWERKS__ 
#define malloc NewPtr
#define free   DisposePtr
#endif

#define NEW(x) ((x *) emalloc(sizeof(x)))

static char *module = "generic_list";

static void *emalloc(unsigned int n);

/****************************************************************************/

void
initialize_list(Generic_list *list)
{
    list->info = NEW(Generic_list_info);

    list->info->pre_element.pointer  = NULL;
    list->info->pre_element.previous = &list->info->pre_element;
    list->info->pre_element.next     = &list->info->post_element;
    list->info->post_element.pointer = NULL;
    list->info->post_element.previous = &list->info->pre_element;
    list->info->post_element.next     = &list->info->post_element;

    list->info->current = &list->info->pre_element;
    list->info->deleted_element.pointer = NULL;
    list->info->lt = NULL;
    list->info->num_of_elements = 0;
}

/****************************************************************************/

void
initialize_sorted_list(Generic_list *list, int (*lt)(void *a, void *b))
{
    initialize_list(list);
    list->info->lt = lt;
}

/****************************************************************************/

void
destroy_list(Generic_list *list)
{
    remove_all(*list);
    free((void *)list->info);
}

/****************************************************************************/

void
add_to_beginning(Generic_list list, void *pointer)
{
    Generic_list_element *element;

    if (!pointer) 
      {
          fprintf(stderr, "%s: NULL pointer passed\n", module);
          exit(1);
      }

    element = NEW(Generic_list_element);
    element->next     = list.info->pre_element.next;
    element->previous = &list.info->pre_element;
    element->pointer  = pointer;

    list.info->pre_element.next->previous = element;
    list.info->pre_element.next = element;

    list.info->num_of_elements++;
}

/****************************************************************************/

void
add_to_end(Generic_list list, void *pointer)
{
    Generic_list_element *element;

    if (!pointer) 
      {
        fprintf(stderr, "%s: NULL pointer passed\n", module);
        exit(1);
      }

    element = NEW(Generic_list_element);
    element->next     = &list.info->post_element;
    element->previous = list.info->post_element.previous;
    element->pointer  = pointer;

    list.info->post_element.previous->next = element;
    list.info->post_element.previous = element;

    list.info->num_of_elements++;
}

/****************************************************************************/

void
add_to_list(Generic_list list, void *pointer)
{
    Generic_list_element *element, *new_element;

    if (list.info->lt) 
      {
        if (!pointer) 
          {
            fprintf(stderr, "%s: NULL pointer passed\n", module);
            exit(1);
          }

        element = list.info->pre_element.next;
        while (element != &list.info->post_element &&
                (*list.info->lt)(element->pointer, pointer))
            element = element->next;
    
        new_element = NEW(Generic_list_element);
        new_element->next     = element;
        new_element->previous = element->previous;
        new_element->pointer  = pointer;

        element->previous->next = new_element;
        element->previous = new_element;

        list.info->num_of_elements++;
      }
    else 
        add_to_end(list, pointer);
}

/****************************************************************************/

void *
remove_from_list(Generic_list list, void *pointer)
{
    Generic_list_element *element;

    element = list.info->post_element.previous;

    while (element != &list.info->pre_element && element->pointer != pointer)
        element = element->previous;

    if (element == &list.info->pre_element)
        /* No such element was found. */
        return NULL;

    if (element == list.info->current) 
      {
        list.info->deleted_element.previous = element->previous;
        list.info->deleted_element.next     = element->next;
        list.info->current                  = &list.info->deleted_element;
      }

    element->previous->next = element->next;
    element->next->previous = element->previous;

    free(element);
    list.info->num_of_elements--;

    return pointer;
}

/****************************************************************************/

void *
remove_from_beginning(Generic_list list)
{
    Generic_list_element *element;
    void *pointer;

    if(list.info->num_of_elements == 0)
        return NULL;

    element = list.info->pre_element.next;
    if (element == list.info->current)
        list.info->current = &list.info->pre_element;

    pointer = element->pointer;
    list.info->pre_element.next = element->next;
    element->next->previous = &list.info->pre_element;

    free(element);
    list.info->num_of_elements--;

    return pointer;
}

/****************************************************************************/

void *
remove_from_end(Generic_list list)
{
    Generic_list_element *element;
    void *pointer;

    if(list.info->num_of_elements == 0)
        return NULL;

    element = list.info->post_element.previous;
    if (element == list.info->current)
        list.info->current = &list.info->post_element;

    pointer = element->pointer;
    list.info->post_element.previous = element->previous;
    element->previous->next = &list.info->post_element;

    free(element);
    list.info->num_of_elements--;

    return pointer;
}

/****************************************************************************/

void *
remove_current(Generic_list list)
{
    Generic_list_element *element;
    void *pointer;

    element = list.info->current;
    if (element->pointer == NULL)
        return NULL;

    list.info->deleted_element.previous = element->previous;
    list.info->deleted_element.next     = element->next;
    list.info->current                  = &list.info->deleted_element;

    pointer = element->pointer;
    element->next->previous = element->previous;
    element->previous->next = element->next;

    free(element);
    list.info->num_of_elements--;

    return pointer;
}

/****************************************************************************/

void
remove_all(Generic_list list)
{
    Generic_list_element *element;

    element = list.info->pre_element.next;
    while (element != &list.info->post_element) 
      {
        element = element->next;
        free(element->previous);
      }

    list.info->pre_element.next = &list.info->post_element;
    list.info->post_element.previous = &list.info->pre_element;
    list.info->num_of_elements = 0;
}

/****************************************************************************/

void *
peek_at_beginning(Generic_list list)
{
    return list.info->pre_element.next->pointer;
}

/****************************************************************************/

void *
peek_at_end(Generic_list list)
{
    return list.info->post_element.previous->pointer;
}

/****************************************************************************/

void *
first_in_list(Generic_list list)
{
    list.info->current = list.info->pre_element.next->next->previous;
    return list.info->current->pointer;
}

/****************************************************************************/

void *
current_in_list(Generic_list list)
{
    return list.info->current->pointer;
}

/****************************************************************************/

void *
last_in_list(Generic_list list)
{
    list.info->current = list.info->post_element.previous->previous->next;
    return list.info->current->pointer;
}

/****************************************************************************/

void *
next_in_list(Generic_list list)
{
    list.info->current = list.info->current->next;
    return list.info->current->pointer;
}

/****************************************************************************/

void *
previous_in_list(Generic_list list)
{
    list.info->current = list.info->current->previous;
    return list.info->current->pointer;
}

/****************************************************************************/

void
reset_to_beginning(Generic_list list)
{
    list.info->current = &list.info->pre_element;
}

/****************************************************************************/

void
reset_to_end(Generic_list list)
{
    list.info->current = &list.info->post_element;
}

/****************************************************************************/

int
num_of_objects(Generic_list list)
{
    return list.info->num_of_elements;
}

/****************************************************************************/

int
is_empty(Generic_list list)
{
    return (list.info->num_of_elements == 0);
}

/****************************************************************************/

int
is_in_list(Generic_list list, void *pointer)
{
    Generic_list_element *element;

    element = list.info->pre_element.next;

    while (element != &list.info->post_element && element->pointer != pointer)
        element = element->next;

    return (element != &list.info->post_element);
}

/****************************************************************************/

Generic_list
copy_list(Generic_list list)
{
    Generic_list list_copy;
    Generic_list_element *element;

    initialize_sorted_list(&list_copy, list.info->lt);
    element = list.info->pre_element.next;
    while (element != &list.info->post_element) 
      {
        add_to_end(list_copy, element->pointer);
        element = element->next;
      }
    
    return list_copy;
}

/****************************************************************************/

void
perform_on_list(Generic_list list, void (*fn)(void *pointer, void *args),
        void *args)
{
    Generic_list_element *element;

    element = list.info->pre_element.next;
    while (element != &list.info->post_element) 
      {
        (*fn)(element->pointer, args);
        element = element->next;
      }
}

/****************************************************************************/

void *
first_that(Generic_list list, int (*fn)(void *pointer, void *args), void *args)
{
    Generic_list_element *element;

    element = list.info->pre_element.next;
    while (element != &list.info->post_element &&
                            !(*fn)(element->pointer, args)) 
      {
        element = element->next;
      }

    if (element->pointer)
        list.info->current = element;

    return element->pointer;
}

/****************************************************************************/

void *
next_that(Generic_list list, int (*fn)(void *pointer, void *args), void *args)
{
    Generic_list_element *element;

    element = list.info->current->next;
    while (element != &list.info->post_element &&
                            !(*fn)(element->pointer, args)) 
      {
        element = element->next;
      }

    if (element->pointer)
        list.info->current = element;

    return element->pointer;
}

/****************************************************************************/

void *
previous_that(Generic_list list, int (*fn)(void *pointer, void *args),
          void *args)
{
    Generic_list_element *element;

    element = list.info->current->previous;
    while (element != &list.info->pre_element &&
                            !(*fn)(element->pointer, args)) 
      {
        element = element->previous;
      }

    if (element->pointer)
        list.info->current = element;

    return element->pointer;
}

/****************************************************************************/

void *
last_that(Generic_list list, int (*fn)(void *pointer, void *args), void *args)
{
    Generic_list_element *element;

    element = list.info->post_element.previous;
    while (element != &list.info->pre_element &&
                            !(*fn)(element->pointer, args)) 
      {
        element = element->previous;
      }

    if (element->pointer)
        list.info->current = element;

    return element->pointer;
}

/****************************************************************************/

Generic_list
all_such_that(Generic_list list, int (*fn)(void *pointer, void *args),
          void *args)
{
    Generic_list list_copy;
    Generic_list_element *element;

    initialize_sorted_list(&list_copy, list.info->lt);
    element = list.info->pre_element.next;
    while (element != &list.info->post_element) 
      {
        if ((*fn)(element->pointer, args))
            add_to_end(list_copy, element->pointer);
        element = element->next;
      }
    
    return list_copy;
}

/****************************************************************************/

void
remove_all_such_that(Generic_list list, int (*fn)(void *pointer, void *args),
             void *args)
{
    void *obj;

    reset_to_beginning(list);
    while ((obj = next_in_list(list)))
        if ((*fn)(obj, args))
            remove_current(list);
}



/****************************************************************************/
/****************************************************************************/
/**                                                                        **/
/**                         Internal functions                             **/
/**                                                                        **/
/****************************************************************************/
/****************************************************************************/

static void *
emalloc(unsigned int n)
{
    void *ptr;

    ptr = (void *) malloc(n);
    if ( ptr == NULL ) 
      {
        fprintf(stderr,"%s: error allocating memory\n", module);
        exit(1);
      }
    return ptr;
}

