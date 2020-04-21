#include "win.h"
#include "misc.h"
#include "stdio.h"
#include "gltypes.h"
#include "window.h"
#include "ctype.h"

#define DESKTOPNAME ".deskconfig"

#define LINSIZ 256
char dbline[LINSIZ];
char *deskfind();
char *index(), *rindex();

dodesk(name)
    char *name;
{
    char *desk;		/* line from data base file */
    int x1, y1, x2, y2;
    int nsame;
    int xoff, yoff;

    if(strlen(name) == 0)
	return;
    if((desk = deskfind(name)) == 0)
	return;
    xoff = 12;
    yoff = -9;
    nsame = samename(name) - 1;		/* minus our self!! */
    xoff *= nsame;
    yoff *= nsame;
    if(sscanf(desk,"%d %d %d %d", &x1, &y1, &x2, &y2) == 4) {
	qenter(RIGHTMOUSE, 1);
	qenter(CURSORX, x1+xoff);
	qenter(CURSORY, y1+yoff);
	qenter(RIGHTMOUSE, 0);
	qenter(CURSORX, x2+xoff);
	qenter(CURSORY, y2+yoff);
    }
}

char *deskfind(name)
	char name[];
{
    register FILE *f;
    register char *bptr, *eptr;
    int i;
    extern char homepath[];
    char *getenv(), *cp;

    /*
     * First look for DESKTOPNAME in current working directory,
     * then in home directory.
     */
    if((f = fopen(DESKTOPNAME,"r")) == (FILE *) 0) {
	if(cp = (char *) getenv("HOME")) {
		strcpy(homepath, cp);
		strcat(homepath, "/");
		strcat(homepath, DESKTOPNAME);
		if((f = fopen(homepath, "r")) == (FILE *) 0)
			return 0;
	} else
		return 0;
    }
    while(fgets(dbline, LINSIZ - 1, f)) {
	bptr = dbline;
	/*
	 * Skip any whitespace at beginning
	 */
	while(*bptr && isspace(*bptr))
	    bptr++;
	/*
	 * If we haven't exhausted the string, & if it contains a ":",
	 * then see if the string contains "name".
	 */
	if (*bptr && (eptr = rindex(bptr,':'))) {
		*eptr = '\0';		/* terminate string at ":" */
		if(strcmp(bptr,name) == 0) {
			fclose(f);
			return (eptr+1);
		}
	}
    }
    fclose(f);
    return 0;
}

buttodev(but)
    char *but;
{
    return RIGHTMOUSE;
}

samename(name)
char *name;
{
    register struct wm_window *w;
    register int count;

    w = win_first;
    count = 0;
    while (w) {
	if ((w->w_state & WS_GRAPHPORT) || (w->w_state & WS_GRAPHPORT)) {
	    if(strcmp(name,w->w_name) == 0)
		count++;
	}
	w = wn_inback(w);
    }
    return count;
}
