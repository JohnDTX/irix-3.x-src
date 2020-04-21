/**************************************************************************
 *									  *
 * 		 Copyright (C) 1984, Silicon Graphics, Inc.		  *
 *									  *
 *  These coded instructions, statements, and computer programs  contain  *
 *  unpublished  proprietary  information of Silicon Graphics, Inc., and  *
 *  are protected by Federal copyright law.  They  may  not be disclosed  *
 *  to  third  parties  or copied or duplicated in any form, in whole or  *
 *  in part, without the prior written consent of Silicon Graphics, Inc.  *
 *									  *
 **************************************************************************/

#include "globals.h"
#define	NULL 0

/* this routine allocates n bytes linking it into the linked list of
the currently open object */

char *gl_objalloc(objecthdr, n)
objhdr	*objecthdr;
int 	n;
{
    register cons *cell;

    if(objecthdr != 0) {
        if((cell = (cons *)malloc(sizeof(cons))) != 0) {
            if ((cell->item = (short *)malloc(n)) != 0) {
	        cell->link = objecthdr->strings;
                objecthdr->strings = cell;
                return (char *) cell->item;
	    }
	    free(cell);
	}
	gl_outmem("gl_objalloc");
    }
    return (char *)0;
}

gl_objfree(objecthdr, ptr)
objhdr	*objecthdr;
register char 	*ptr;
{
    register cons *cell, *prev;

    /* grab the first cons cell in the object's list of memory */

    if(objecthdr == 0)
        return;
    cell = objecthdr->strings;
    if(cell == 0)
   	return;
    prev = NULL;
    while((cell->item != (short *)ptr) && (cell->link != NULL)) {
	prev = cell;
	cell = cell->link;
    }
    if(cell->item == (short *)ptr) {
	free(ptr);
	if(prev == NULL)
	    objecthdr->strings = cell->link;
	else
	    prev->link = cell->link;
	free(cell);
    } else
	printf("gl_objfree","can't find the block of memory\n");
}
