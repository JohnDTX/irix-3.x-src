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
#include "splinegl.h"

extern int i_jump(), i_tag(), i_charstr(), i_slopnop2(),
	   i_slopnop3(), i_retsym(), i_callfunc(), 
	   gl_drawcurve(), gl_drawcurves(), gl_drawpatch();

gl_checkspacefunc(n)
long n;
{
    if(gl_replacemode) {
	if(gl_replacespace(n) == 0) return 0;
    } else {
	if(gl_insertspace(n) == 0) return 0;
    }
    return 1;
}

gl_replacespace(newlen)
long newlen;
{
    register long gl_temp,oldlen;

    if(gl_openobjhdr == 0)
	return 0;
    while (1) {
	if((gl_temp = *(long *)gl_currentpos) == (long)i_jump) {
	    gl_currentpos = (short *)(*(long *)(gl_currentpos + 2));
	    continue;
	}
	if(gl_temp == (long)i_tag) {
	    gl_currentpos += 4;
	    continue;
	}
	if(gl_temp == (long)i_slopnop2) {
	    gl_currentpos += 2;
	    continue;
	}
	if(gl_temp == (long)i_slopnop3) {
	    gl_currentpos += 3;
	    continue;
	}
	if(gl_temp == (long)i_retsym) {
	    gl_ErrorHandler (ERR_REPLACE, WARNING,
		"gl_replacespace -- can't replace past end of object");
	    return;
	}
	oldlen = gl_getcmdlength(*(long *)gl_currentpos,
				*(short *)(gl_currentpos+2));
	if(oldlen == newlen) {
	    if(gl_temp == (long)i_charstr) {
		gl_objfree(gl_openobjhdr, 
			(char *)(*(long *)(gl_currentpos + 2)));
	    } else if(gl_temp == (long)i_callfunc) { 
		register char	*str;
		register long	routine;
		str = (char *)(*(long *)(gl_currentpos + 5));
		routine = (long) (*(long *)(gl_currentpos + 3));
		if(routine == (long)gl_drawcurve)
		    gl_objfree(gl_openobjhdr, str);
		else if(routine == (long)gl_drawcurves) {
		    if(((Curves *)str)->geom)
			gl_objfree(gl_openobjhdr, ((Curves *)str)->geom);
		    if(((Curves *)str)->itermat)
			gl_objfree(gl_openobjhdr, ((Curves *)str)->itermat);
		    gl_objfree(gl_openobjhdr, str);
		} else if(routine == (long)gl_drawpatch) {
		    if(((Patch *)str)->itermats)
			gl_objfree(gl_openobjhdr, ((Patch *)str)->itermats);
		    gl_objfree(gl_openobjhdr, str);
		}
	    }
	    return 1;
	}
	else {
	    char buf[80];
	    sprintf (buf,"? length: %d, replacing %s length: %d",
		    newlen,gl_cmdname(*(long *)gl_currentpos),oldlen);
	    gl_ErrorHandler (ERR_REPLACE, WARNING, buf);
	    return 0;
	}
    }
}

gl_insertspace(n)
register long n;
{
    if(0 == gl_makeroom(n)) return 0;
    gl_openobjhdr->datasize += n;
    if(gl_currentpos == gl_openobjhdr->tailptr)
	gl_openobjhdr->tailptr += n;
    if (!gl_currentpos) return 0;
    *(long *)(gl_currentpos + n + 2) = *(long *)(gl_currentpos + 2);
    *(long *)(gl_currentpos + n) = *(long *)(gl_currentpos);
    return 1;
}

gl_endcompile_error (temp,curpos,gl_currentpos)
{
    printf("count mismatch at ENDCOMPILE: %d - actual: %d\n",
	    temp, (curpos-gl_currentpos)>>1);
}
