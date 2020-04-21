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

long strwidth(string)
register char *string;
{
    register Fontchar *fchar;
    register length  = 0;
    register long fontcount,c;

    fchar = gl_wstatep -> curatrdata.currentfont -> chars;
    fontcount = gl_wstatep -> curatrdata.currentfont -> maxchars;

    if (!string) 
	return 0;

    while (c = *string++) {
	if(c < fontcount) {
	    length += fchar[c].width;
	}
    }
    return length;
}

/* FORTRAN version	*/
long strwid(string,len)
register char *string;
register long len;
{
    register Fontchar *fchar;
    register length  = 0;
    register long fontcount,c;

    fchar = gl_wstatep -> curatrdata.currentfont -> chars;
    fontcount = gl_wstatep -> curatrdata.currentfont -> maxchars;

    if (!string) return 0;
    while (len > 0) {
	if ((c = *string++) < fontcount) 
	    length += fchar[c].width;
	len--;
    }
    return length;
}
