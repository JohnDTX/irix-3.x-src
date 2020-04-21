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
#include "grioctl.h"

/* 
 * getmem: returns the number of bytes on the malloc free list plus the number
 * 	of free pages owned by the kernel.
 *
 */
long getmem()
{
    register char *memptr, *newmemptr;
    register i;
    register totalfree;
    long allocated, freed, acells, fcells, maxfree;

    gl_mallocstats(&allocated,&freed,&acells,&fcells,&maxfree);
    return (freed + 4096*grioctl(GR_FREEPAGES));
}

#define TEXT_BUFFER_SIZE 80
#undef BUSY
#define BUSY 1
#define INT int
#define testbusy(p) ((INT)(p)&BUSY)
#define clearbusy(p) ((INT)(p)&~BUSY)

/* 
 * gl_mallocstats:
 * 	returns current teamsize, number of bytes malloc'ed, number of bytes
 *  	on the free list, the number of memory cells in the arena, the
 *	number of free cells in the arena, and the max free area.
 */
gl_mallocstats(allocated,freed,acells,fcells,maxfree)
register long *allocated, *freed, *acells, *fcells, *maxfree;
{
    register int *p, pend, size;
    extern int allocs[2];

    *allocated = 0;
    *freed = 0;
    *acells = 0;
    *fcells = 0;
    *maxfree = 0;
    pend = (int) &allocs[0];
    for(p = (int *) clearbusy(allocs[1]);
		 clearbusy(*p) != pend;
			     p = (int *)clearbusy(*p)) {
	size = clearbusy(*p) - (int)p;
	if(testbusy(*p)) {
	    *allocated += size; 
	    *acells += 1;
	} else {
	    if(size > *maxfree) 
		*maxfree = size;
	    *freed += size; 
	    *fcells += 1;
	}
    }
}
