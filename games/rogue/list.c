static	char	*List_c	= "@(#)list.c	1.5";
/*
 * Functions for dealing with linked lists of goodies
 *
 * @(#)list.c	2.3 (Berkeley) 2/1/81
 */

#include "rogue.h"

/*
 * detach:
 *	Takes an item out of whatever linked list it might be in
 */

_detach(list, item)
reg1 rega1	struct	linked_list **list;
reg2 rega2	struct	linked_list *item;
{
    if (*list == item)
	*list = next(item);
    if (prev(item) != NULL) item->l_prev->l_next = next(item);
    if (next(item) != NULL) item->l_next->l_prev = prev(item);
    item->l_next = NULL;
    item->l_prev = NULL;
}

/*
 * _attach:
 *	add an item to the head of a list
 */

_attach(list, item)
reg1 rega1	struct	linked_list **list;
reg2 rega2	struct	linked_list *item;
{
    if (*list != NULL) {
	item->l_next = *list;
	(*list)->l_prev = item;
    } else {
	item->l_next = NULL;
	item->l_prev = NULL;
    }

    *list = item;
}

/*
 * _free_list:
 *	Throw the whole blamed thing away
 */

_free_list(ptr)
reg1 rega1	struct	linked_list **ptr;
{
	reg2 rega2	struct	linked_list *item;

	while (*ptr != NULL) {
		item = *ptr;
		*ptr = next(item);
		discard(item);
	}
}

/*
 * discard:
 *	free up an item
 */

discard(item)
reg1 rega1	struct	linked_list *item;
{
    total -= 2;
    FREE(item->l_data);
    FREE(item);
}

/*
 * new_item
 *	get a new item with a specified size
 */

struct	linked_list *
new_item(size)
reg1 regd1	int	size;
{
    reg2 rega1	struct	linked_list *item;

    if ((item = (struct linked_list *) new(sizeof *item)) == NULL)
	msg("Ran out of memory for header after %d items", total);
    if ((item->l_data = new(size)) == NULL)
	msg("Ran out of memory for data after %d items", total);
    item->l_next = item->l_prev = NULL;
    return item;
}

/*
 * Next and prev, for walking along the linked lists of things
 */

#ifndef FAST
struct	linked_list *
next(ptr)
reg1 rega1	struct	linked_list *ptr;
{
    if (ptr == NULL)
	debug("Next through a nil pointer");
    return (ptr == NULL) ? NULL : ptr->l_next;
}

struct	linked_list *
prev(ptr)
reg1 rega1	struct	linked_list *ptr;
{
    if (ptr == NULL)
	debug("Prev through a nil pointer");
    return (ptr == NULL) ? NULL : ptr->l_prev;
}

/*
 * ldata:
 *	Return the data pointer for a linked list item
 */

char	*
ldata(item)
reg1 rega1	struct	linked_list *item;
{
    if (item == NULL) {
	debug("Tried to get data through nil pointer.");
	return NULL;
    }
    return item->l_data;
}
#endif

char	*
new(size)
{
    char	*space = ALLOC(size);

    if (space == NULL) {
	sprintf(prbuf, "Rogue ran out of memory sbrk=%u pc=%u. Wanted %u. \
Fatal error!",
	  sbrk(0),(unsigned int) &space,size);
	fatal(prbuf);
    }
    total++;
    return space;
}
