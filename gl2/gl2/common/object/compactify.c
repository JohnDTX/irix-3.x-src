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

extern objhdr	*gl_findobjhdr(), *gl_getnewobjhdr();
extern int	i_retsym(), i_jump(), i_tag(), i_slopnop2(), i_slopnop3();

void compactify(obj)
Object obj;
{
    objhdr		*hdr;
    short		*head, *tmpptr;
    long		datasize, physicalsize, gl_temp;
    register short	*oldptr, *newptr;
    register long	spaceleft, i;
    cons		*chunks;
    register cons	*consptr, *saveptr;

    if(!isobj(obj)) return;
    hdr = gl_findobjhdr(obj);
    if(hdr == gl_openobjhdr) {
	gl_openobjhdr = 0;
	gl_openobj = -1;
    }
    if(!(newptr = head = (short *)malloc(gl_objchunksize))) 
	return;
    if(!(chunks = (cons *)malloc(sizeof(cons)))) {
	free((char *)head);
	return;
    }
    chunks->link = 0;
    chunks->item = newptr;
    oldptr = hdr->head;
    physicalsize = gl_objchunksize>>1;
    datasize = 0;
    spaceleft = (gl_objchunksize>>1) - 4;
    while (1) {
	if ((gl_temp = *(long *)oldptr) == (long)i_retsym) {
	    *(long *)newptr = (long)i_retsym;
	    hdr->physicalsize = physicalsize;
	    hdr->datasize = datasize;
	    consptr = hdr->chunks;
	    while (consptr) {
		free((char *)(consptr->item));
		saveptr = consptr;
		consptr = consptr->link;
		free((char *)saveptr);
	    }
	    hdr->chunks = chunks;
	    hdr->head = head;
	    hdr->tailptr = newptr;
	    hdr->tailend = newptr + spaceleft + 4;
	    return;
	} else if(gl_temp == (long)i_jump) {
	    oldptr = (short *)(*(long *)(oldptr + 2));
	} else if(gl_temp == (long)i_slopnop2) {
	    oldptr += 2;
	} else if(gl_temp == (long)i_slopnop3) {
	    oldptr += 3;
	} else /* default */ {
	    i = gl_getcmdlength(*(long *)oldptr, *(short *)(oldptr + 2));
	    if(i > spaceleft) {
		*(long *)newptr = (long)i_jump;
		if(!(tmpptr = (short *)malloc(gl_objchunksize))) {
		    while (chunks) {
			free((char *)(chunks->item));
			consptr = chunks;
			chunks = chunks->link;
			free((char *)consptr);
			return;
		    }
		}
		if(!(consptr = (cons *)malloc(sizeof(cons)))) {
		    free((char *)tmpptr);
		    while (chunks) {
			free((char *)(chunks->item));
			consptr = chunks;
			chunks = chunks->link;
			free((char *)consptr);
			return;
		    }
		}
		consptr->item = tmpptr;
		consptr->link = chunks;
		chunks = consptr;
		newptr += 2;
		*(long *)newptr = (long)tmpptr;
		spaceleft = (gl_objchunksize>>1) - 4;
		physicalsize += (gl_objchunksize>>1);
		newptr = tmpptr;
	    }
	    if(*(long *)oldptr == (long)i_tag)
		gl_replacehash(newptr, obj, *(long *)(oldptr+2));
	    datasize += i;
	    spaceleft -= i;
	    while(i--)
		*newptr++ = *oldptr++;
	}
    }
}
