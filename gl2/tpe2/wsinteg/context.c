/*
**			 	Switch graphics contexts
**
*/

#include <stdio.h>
#include <sys/types.h>
#include <signal.h>
#include <gl.h>
#include "term.h"

extern int writehostpid;
extern int dispatchLen;

switchtotext()
{
    if (context == TEXT)
	return;

 /* turn KBD LOCKED light off */
    kblamp(LAMP_KBDLOCKED, 0);
	kblamp(LAMP_LOCAL,0);
    context = TEXT;
    if (lf)
	fprintf(lf, "<TEXT>\n");
}

switchtographics()
{
    if (context == GRAPHICS)
	return;

 /* turn KBD LOCKED light on */
    kblamp(LAMP_KBDLOCKED, 1);
    context = GRAPHICS;
    if (lf)
	fprintf(lf, "<GRAPHICS>\n");
}

irisinit()
{
    register dispatchEntry *de;
    register int i;
    char *cp;
    long err = 0;


    if (!maxcom) {
	maxcom = dispatchLen;
#ifndef GL1
	maxcom--;	/* don't count the the null entry on the end */
#endif	
/*	for (i = 0; i < maxcom; i++) {
	    if (strlen(dispatch[i].format) > MAXARGS) {
		fprintf(stderr, "wsiris: remote routine ");
		if (cp = getcmdname(i))
		    fprintf(stderr,cp);
		else
		    fprintf(stderr,"%d",i);
		fprintf(stderr, " has more than %d arguments\n\r", MAXARGS);
		err++;
	    }
	}
	if (err)
	    oops("wsiris: fix these routines\n\r");
	*/
    }
    context = TEXT;
}
