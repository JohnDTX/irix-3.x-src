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

extern objhdr	*gl_findobjhdr(), *gl_getnewobjhdr();
extern int i_retsym(),i_jump(),i_tag(),i_slopnop2(),i_slopnop3(), i_charstr(),
	   i_callfunc(), gl_drawcurve(), gl_drawcurves(), gl_drawpatch();

static char objdelete_n[] = "objdelete";

void objdelete(tag1, tag2)
Tag tag1, tag2;
{
    cons		*deletedtags = 0, *save, *strcons;
    register short	*begin, *end;
    register long	gl_temp;

    if(gl_openobjhdr == 0) {
	gl_ErrorHandler(ERR_NOOPENOBJ, WARNING, objdelete_n);
	return;
    }
    if(tag1 == STARTTAG)
	begin = gl_openobjhdr->head - 4; /* -4 offsets next +4 */
    else if(!(begin = (short *)gl_findhash(gl_openobj, tag1))) {
	gl_ErrorHandler(ERR_NOSUCHTAG, WARNING, objdelete_n);
	return;
    }
    begin += 4;
    end = begin;
    while(1) {
	if((gl_temp = *(long *)end) == (long)i_retsym) {
	    if(tag2 == ENDTAG) {
		while(deletedtags) {
		    save = deletedtags;
		    deletedtags = deletedtags->link;
		    deltag(save->item);
		    free((char *)save);
		}
		*(long *)begin = (long)i_retsym;
		gl_openobjhdr -> tailptr = begin;	/* peter per scr 1446 */
		goto good_exit;
	    }
	    while(deletedtags) {
		save = deletedtags;
		deletedtags = deletedtags->link;
		free((char *)save);
	    }
	    gl_ErrorHandler(ERR_NOSUCHTAG, WARNING, objdelete_n);
	    return;
	} else if(gl_temp == (long)i_jump) {
	    end = (short *)(*(long *)(end + 2));
	} else if(gl_temp == (long)i_tag) {
	    if(*(long *)(end + 2) == tag2) {
		if(end == begin)
		    goto good_exit;
		if(end == begin + 2) {
		    *(long *)begin = (long)i_slopnop2;
		    goto good_exit;
		}
		if(end == begin + 3) {
		    *(long *)begin = (long)i_slopnop3;
		    goto good_exit;
		}
		while(deletedtags) {
		    save = deletedtags;
		    deletedtags = deletedtags->link;
		    deltag(save->item);
		    free((char *)save);
		}
		*(long *)begin = (long)i_jump;
		*(long *)(begin + 2) = (long)end;
		goto good_exit;;
	    }
	    if(!(save = (cons *)malloc(sizeof(cons)))) {
		while(deletedtags) {
		    save = deletedtags;
		    deletedtags = deletedtags->link;
		    free((char *)save);
		}
		gl_outmem(objdelete_n);
		return;
	    }
	    save->item = (short *)(*(long *)(end + 2));
	    save->link = deletedtags;
	    deletedtags = save;
	    end += 4;
	} else {
	    register	char	*str;
	    if(gl_temp == (long)i_charstr) {
		str = (char *)(*(long *)(end + 2));
		gl_objfree(gl_openobjhdr, str);
	    } else if(gl_temp == (long)i_callfunc) { 
		register long	routine;
		str = (char *)(*(long *)(end + 5));
		routine = (long)(*(long *)(end + 3));
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
	    if (gl_temp == (long)i_charstr)
		end += 4;
	    else
		end += gl_getcmdlength(*(long *)end, *(short *)(end + 2));
	}
    }
good_exit:    
    gl_replacemode = 0;
    gl_currentpos = gl_openobjhdr -> tailptr;
    gl_currentend = gl_openobjhdr -> tailend;
}
