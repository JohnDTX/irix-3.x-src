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
#include "TheMacro.h"
#include "imdraw.h"
#include "imattrib.h"
#include "glerror.h"

#define im_loccharstr(str) im_charstr(str)
#define im_charstr(str) {					\
	register unsigned char *_s = (unsigned char *)str;	\
	register long passcode = 0x0408001c;			\
	register int charcode;					\
	register long fc;					\
								\
	fc = (long)(WS->curatrdata.currentfont->chars);		\
	im_outfontbase(WS->fontbase);				\
	while ((charcode = *_s++) != 0) {			\
	    im_outlong(passcode);				\
	    WS = (windowstate *)(fc + (charcode<<3));		\
	    im_outlong(*(long *)WS);				\
	    im_outlong(*(((long *)WS)+1));			\
	}							\
	WS = gl_wstatep;					\
	im_outfontbase(WS->fontrambase);			\
	im_freepipe;						\
    }

extern int i_charstr();
extern char *gl_objalloc();
INTERP_NAME(charstr);
INTERP_NAME(loccharstr);

/* FORTRAN version of charstr	*/
charst(str,len)
    register char *str;
    register long len;
{
    register char *newstr;

    if (gl_openobjhdr == 0) {
	newstr = (char *)malloc(1+len);
	if (!newstr) {
	    gl_outmem(charstr_n);
	    return;
	}
	{
	    register char *to;
	    to = newstr;
	    while (len > 0) {
		*to++ = *str++;
		len--;
	    }
	    *to = 0;
	}
	{
	    im_setup;
	    im_charstr(newstr);
	    im_cleanup;
	}
	free (newstr);
    } else {
	if (gl_checkspace(4) == 0) return;
	newstr = gl_objalloc(gl_openobjhdr,1+len);
	if (!newstr) {
	    gl_outmem(charstr_n);
	    return;
	}
	{
	    register char *to;
	    to = newstr;
	    while (len > 0) {
		*to++ = *str++;
		len--;
	    }
	    *to = 0;
	}
	BEGINCOMPILE(4);
	ADDADDR(i_charstr);
	ADDADDR(newstr);
	ENDCOMPILE;
    }
}

/*
 * charstr can possibly lose a chunk of memory of size cons if the first
 * allocation succeeds and there is no space in the object.
 */
void charstr(str)
char *str;
{
    if (gl_openobjhdr == 0) {
	im_setup;
	im_charstr(str);
	im_cleanup;
    } else {
	register char 	*newstr;

	if (gl_checkspace(4) == 0) return;
	newstr = gl_objalloc(gl_openobjhdr,1+strlen(str));
	if (!newstr) {
	    gl_outmem(charstr_n);
	    return;
	}
	strcpy(newstr, str);
	BEGINCOMPILE(4);
	ADDADDR(i_charstr);
	ADDADDR(newstr);
	ENDCOMPILE;
    }
}

ROOT_1I(loccharstr)

#include "interp.h"

static bogus ()
{
    DECLARE_INTERP_REGS;

    INTERP_ROOT_1I(charstr);

INTERP_LABEL(loccharstr,4);
    bra(_i_charstr);
}

#ifdef 0

/* old interpreter code	*/
_i_charstr:
	movl	STATEREG@(CURRENTFONT), a0
	addql	#4, a0		| skip base font ram addr
	movl	a0@+, d1	| d1 = addr of char desc table
	movl	a0@+, d2	| d2 = number of chars in font
	movl	PC@+, a0	| a0 = ptr to chars
	movl	#0x0408001C, d0	| passthru + char code
	clrl	d3
1$:	clrw	d3
	movb	a0@+, d3	| d3 = next char code
	jle	2$		| test for end of string or illegal
	aslw	#3, d3
	movl	d3, a1
	addl	d1, a1
	outreg(d0)
	movl	a1@+, GE@
	movl	a1@, GE@
	jra	1$
2$:	thread
#endif
