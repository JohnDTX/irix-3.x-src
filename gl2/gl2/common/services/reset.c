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
#include "shmem.h"
#include "imsetup.h"

/* configuration tables for ge's */

short	gl_normalconfigtab[] = {
	0x00,	0x9,	0xa,	0xb,	0xc,
		0x15,	0x10,	0x11,	0x12,
		0x13,	0x14,	0x20,	0x21,
	0x08
};

short	gl_3dconfigtab[] = {
	0x00,	0x9,	0xa,	0xb,	0xc,
		0x15,	0x10,	0x11,	0x12,
		0x13,	0x14,	0x22,	0x23,
	0x0a
};

/* configuration table for getting the graphics position */
short	gl_getgposconfigtab[] = {
	0x00,	0x09,	0x0a,	0x0b,	0x0c,
		0x09,	0x0a,	0x0b,	0x0c,
		0x13,	0x14,	0x20,	0x21,
	0x00
};

/* configuration table for getting the graphics position */
short	gl_getgpos10configtab[] = {
	0x00,	0x09,	0x0a,	0x0b,	0x0c,
		0x15,	0x09,	0x0a,	0x0b,
		0x0c,	0x14,	0x20,	0x21,
	0x00
};

/*
 * gl_justconfigure:
 *	- configure the ge's according to the ones we found in gefind()
 *	- also configures CFP's if found
 */
gl_justconfigure(table)
register short *table;
{
    register struct shmem *sh = gl_shmemptr;
    register short mask, found, temp;
    im_setup;

    mask = sh->ge_mask;
    found = sh->ge_found;

    im_outshort(GEreconfigure);
    for (temp = 0; temp < 14; temp++) {
	    if ((mask <<= 1) & 0x4000)
		    im_outshort((--found<<8) | *(table+temp));
    }
    im_passthru(0);
    if (gl_pickselect)		/* restore hitmode if necessary */
	im_outshort(GEsethitmode);	
    im_cleanup;		/* no last out here ok? */
}

gl_normalconfig()
{
    gl_justconfigure(gl_normalconfigtab);
}

gl_3dconfig()
{
    gl_justconfigure(gl_3dconfigtab);
}

gl_getgposconfig()
{
    if (gl_shmemptr -> ge_found == 14 /* counting GEPAs */)
	gl_justconfigure(gl_getgposconfigtab);
    else gl_justconfigure(gl_getgpos10configtab);
}

void gewrite(buf,len)
    register short buf[];
    register long len;
{
    im_setup;
    
    while (--len > 0) {
	im_outshort(*buf++);
    }
    im_last_outshort(*buf++);
    im_cleanup;
}
