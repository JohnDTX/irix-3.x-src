/*
**			 	Switch graphics contexts
**
*/
#include "gl.h"
#include "term.h"
#include "Vserial.h"
#include "grioctl.h"

switchtotext()
{
    if (context == TEXT)
	return;
    grioctl(GR_SAFE);
    lampoff(LAMP_KBDLOCKED);
    gl_textrefresh();
    context = TEXT;
}

switchtographics()
{
    if (context == GRAPHICS)
	return;
    grioctl(GR_SAFE);
    lampon(LAMP_KBDLOCKED);
    context = GRAPHICS;
}

extern dispatchEntry 	dispatch[];

irisinit()
{
    register dispatchEntry *de;

    turnaround = 0;
    if(!graphinited) {
	ginit();
	maxcom = 0;
	for(de = &dispatch[0]; de->func != NULL; de++)
	    maxcom++;
	color(0);
	clear();
    }
    tpon();
    ttyinit();
    context = TEXT;
    graphinited = 1;	/* this must be after ttyinit(); */
}
