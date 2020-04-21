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
#include "glerror.h"

extern objhdr *gl_findobjhdr(), *gl_getnewobjhdr();
extern short  *gl_addchunk();
extern int i_jump();

/*
 * gl_makeroom: makes certain that there is enough space in the display list
 * to take an entry of length n (being sure to leave room for a jump
 * instruction (4 shorts) after it.  0 is returned if gl_makeroom is
 * unsuccessful.
 */
gl_makeroom(n)
long n;
{
    register short	*new;

    if(gl_openobjhdr == 0)
	return 0;
    if(gl_replacemode) printf ("You called makeroom in replace mode\n");
    if(gl_currentpos == 0)
  	return 0;
    if((n<<1) <= (long)gl_currentend - (long)gl_currentpos - 8)
	return 1;
    if(!(new = gl_addchunk()))
	goto nomemerror;
    if(gl_currentpos == gl_openobjhdr->tailptr) {
	gl_openobjhdr->tailptr = new;
	gl_openobjhdr->tailend = new + (gl_objchunksize>>1);
    }
    *(long *)new = *(long *)gl_currentpos;
    *(long *)(new+2) = *(long *)(gl_currentpos+2);
    *(long *)gl_currentpos = (long)i_jump;
    gl_currentpos += 2;
    *(long *)gl_currentpos = (long)new;
    gl_currentpos = new;
    gl_currentend = new + (gl_objchunksize>>1);
    return 1;
nomemerror:
    gl_outmem("gl_makeroom");
    return 0;
}
