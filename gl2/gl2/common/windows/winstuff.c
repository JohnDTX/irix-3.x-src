/*
 *	procedural interface to mex
 *
 *			Rocky Rhodes - 1985
 *
 */
#include "gl.h"
#include "device.h"
#include "glipc.h"
#include "grioctl.h"
#include "globals.h"
#include "shmem.h"
#include "stdio.h"
#include "uc4.h"
#include "imattrib.h"

extern struct portreq gl_initportreq;
void winfunc();

struct warray {
	windowstate	*ws;
	long		gfnum;	/* these are defined as longs to simplify
				 * returning values in the RGL -- NB.
				 * kernel still defines them as shorts */
};

#define NUMWSES 10
static struct warray wstuff[NUMWSES];

int glpl_ginited = 0;

/* FORTRAN version of winopen() */
long winope(name,len)
char *name;
long len;
{
#ifdef UNIX
    register short i;
    register windowstate *ws;
    register struct sendrec *srec;
    short v;
    long gfnum;
    long xsize, ysize;

    if(!glpl_ginited) {
        getpor(name,len); 	/* this will set glpl_ginited */
 	return winget();
    }
    for(i=0; i<NUMWSES; i++)
	if(!wstuff[i].ws)
	    break;
    if(i == NUMWSES) {
	fprintf(stderr, "no more gfports for this process!!\n");
	return -1;		/* no more gfports for this guy. */
    }
    if((gfnum = grioctl(GR_GETGFPORT, 0)) == -1) {
	fprintf(stderr, "no more gfports in system!!\n");
	return -1;		/* no more gfports in system */
    }
    if(!(ws = (windowstate *)malloc(sizeof(windowstate)))) {
	fprintf(stderr, "out of memory in getgfport\n");
	return -1;
    }
    wstuff[i].ws = ws;
    wstuff[i].gfnum = gfnum;

/*
 *	save 7400 bytes of text by using bcopy!!  to replace
 *
 * 		*ws = gl_shmemptr->ws; 			
 */
    bcopy(&gl_shmemptr->ws,ws,sizeof(windowstate));

/* make this windowstate look like one that hasn't been initialized: */
    ws->matrixstatep = 0;
    ws->softstacktop = ws->matrixlevel = 0;
    ws->hdwrstackbottom = 1;

    winset(gfnum);

    /* send port request to window manager (if one is running) */
    srec = (struct sendrec *)gl_shmemptr->smallbuf; 
    srec->msg = PORTREQ;
    srec->len = sizeof(struct portreq);
    gl_initportreq.gfnum = gfnum;
    gl_initportreq.pid = getpid();
    bcopy(&gl_initportreq, srec->data, sizeof(struct portreq));
    grioctl(GR_SEND, WINDOWMAN);
    gl_hintinited = 0;
    gl_initallstackstuff();
    reshapeviewport();
    getsize(&xsize,&ysize);
    ortho2(-0.5,xsize-0.5,-0.5,ysize-0.5);
    gl_hintinited = 0;
{
    im_setup;			/* need to do this so that front/back	*/
    im_outconfig();		/* buffers are enabled correctly	*/
}
    return gfnum;
#endif
#ifdef V
    gbegin();
    return -1;
#endif
}

long winopen(name)
char *name;
{
    if(name == NULL)
	name = "";
    return winope(name,strlen(name));
}

void winclose(gfnum)
long gfnum;
{
#ifdef UNIX
    register short i;

    for(i=0; i<NUMWSES; i++)
	if(wstuff[i].ws && (wstuff[i].gfnum == gfnum))
	    break;
    if(i == NUMWSES)
	return;		/* no gfport by that number belonging to me. */
    free(wstuff[i].ws);
    wstuff[i].ws = 0;
    grioctl(GR_PUTGFPORT, gfnum);
#endif
}

long winset(gfnum)
long gfnum;
{
#ifdef UNIX
    register short i;
    register long oldwin;
    register windowstate *oldws, *newws;

    oldwin = winget();
    if(oldwin == gfnum)
	return oldwin;		/* already current gfport */
    for(i=0; i<NUMWSES; i++)
	if(wstuff[i].ws && (wstuff[i].gfnum == gl_shmemptr->gfnum))
	    break;
    if(i == NUMWSES) {
	for(i=0; i<NUMWSES; i++)
	    if(!wstuff[i].ws)
		break;
	if(i == NUMWSES) {
	    fprintf(stderr, "no more gfports, winset\n");
	    return oldwin;	/* no more space to hold saved windowstate */
	}
	if(!(wstuff[i].ws = (windowstate *)malloc(sizeof(windowstate)))) {
	    fprintf(stderr, "out of memory, winset\n");
	    return oldwin;	/* no more space to hold saved windowstate */
	}
	wstuff[i].gfnum = gl_shmemptr->gfnum;
    }
    oldws = wstuff[i].ws;
    for(i=0; i<NUMWSES; i++)
	if(wstuff[i].ws && (wstuff[i].gfnum == gfnum))
	    break;
    if(i == NUMWSES) {
	fprintf(stderr, "trying to set to unowned gfport: %d\n", gfnum);
	return oldwin;		/* no gfport by that number here. */
    } else
	newws = wstuff[i].ws;

    *(long *)&gl_shmemptr->smallbuf[0] = (long)newws;
    *(long *)&gl_shmemptr->smallbuf[2] = (long)oldws;
    gl_shmemptr->smallbuf[4] = gfnum;
/*
fprintf(stderr, "winset %d, before setgfport\n\t", gfnum);
fprintf(stderr, "nc:%x, oc:%x, glc:%x\n", newws->curatrdata.myconfig,
	oldws->curatrdata.myconfig, gl_wstatep->curatrdata.myconfig);
*/
    grioctl(GR_SETGFPORT, 0);
/*
fprintf(stderr, "winset %d\n\t", gfnum);
fprintf(stderr, "nc:%x, oc:%x, glc:%x\n", newws->curatrdata.myconfig,
	oldws->curatrdata.myconfig, gl_wstatep->curatrdata.myconfig);
*/
    return oldwin;
#endif
#ifdef V
    return -1;
#endif
}

long winget()
{
#ifdef UNIX
    return gl_shmemptr->gfnum;
#endif
#ifdef V
    return -1;
#endif
}

long inchanget()
{
#ifdef UNIX
    return gl_shmemptr->inputchan;
#endif
#ifdef V
    return -1;
#endif
}

void winpush()
{
#ifdef UNIX
    winfunc(PUSHREQ);
#endif
}

void winpop()
{
#ifdef UNIX
    winfunc(POPREQ);
#endif
}

long winattach()
{
#ifdef UNIX
    register struct sendrec *srec;

    srec = (struct sendrec *)gl_shmemptr->smallbuf; 
    srec->msg = ATTACHREQ;
    srec->len = 2;
    srec->data[0] = gl_shmemptr->gfnum;
    grioctl(GR_SEND, WINDOWMAN);
    return srec->data[0];
#endif
#ifdef V
    return -1;
#endif
}

void winmove(orgx, orgy)
long orgx, orgy;
{
#ifdef UNIX
    register struct sendrec *srec;

    srec = (struct sendrec *)gl_shmemptr->smallbuf; 
    srec->msg = MOVEREQ;
    srec->len = 6;
    srec->data[0] = gl_shmemptr->gfnum;
    srec->data[1] = orgx;
    srec->data[2] = orgy;
    grioctl(GR_SEND, WINDOWMAN);
#endif
}

void winposition(x1, x2, y1, y2)
long x1, x2, y1, y2;
{
#ifdef UNIX
    register struct sendrec *srec;

    srec = (struct sendrec *)gl_shmemptr->smallbuf; 
    srec->msg = RESHAPEREQ;
    srec->len = 10;
    srec->data[0] = gl_shmemptr->gfnum;
    srec->data[1] = x1;
    srec->data[2] = x2;
    srec->data[3] = y1;
    srec->data[4] = y2;
    grioctl(GR_SEND, WINDOWMAN);
#endif
}

/* FORTRAN version of wintitle() */
void wintit(name,len)
char *name;
long len;
{
#ifdef UNIX
    register struct sendrec *srec;
    register char *cp;

    if (len > (sizeof (short))*(SENDSIZE-1)-1)
	len = (sizeof (short))*(SENDSIZE-1)-1;
    srec = (struct sendrec *)gl_shmemptr->smallbuf; 
    srec->msg = SETTITLEREQ;
    srec->len = (sizeof (short)) + len + 1;
    srec->data[0] = gl_shmemptr->gfnum;
    cp = (char *)(&srec->data[1]);
    strncpy(cp, name, len);
    *(cp+len) = '\0';
    grioctl(GR_SEND, WINDOWMAN);
#endif
}

void wintitle(name)
char *name;
{
#ifdef UNIX
    if (name == NULL)
	name = "";
    wintit(name,strlen(name));
#endif
}

void gl_winname(name)
char *name;
{
#ifdef UNIX
    int len;

    if(name == NULL)
	name = "";
    len = strlen(name);
    if (len > (sizeof (short))*(SENDSIZE-1)-1)
	len = (sizeof (short))*(SENDSIZE-1)-1;
    strncpy(gl_initportreq.name, name, len);
    *(gl_initportreq.name+len) = '\0';
#endif
}

void winfunc(func)
int func;
{
#ifdef UNIX
    register struct sendrec *srec;

    srec = (struct sendrec *)gl_shmemptr->smallbuf; 
    srec->msg = func;
    srec->len = 2;
    srec->data[0] = gl_shmemptr->gfnum;
    grioctl(GR_SEND, WINDOWMAN);
#endif
}

int winat(x,y)
int x, y;
{
#ifdef UNIX
    register struct sendrec *srec;

    srec = (struct sendrec *)gl_shmemptr->smallbuf; 
    srec->msg = WINDOWAT;
    srec->len = 4;
    srec->data[0] = x;
    srec->data[1] = y;
    grioctl(GR_SEND, WINDOWMAN);
    return srec->data[0];
#else 
    return -1;
#endif
}

int inchanat(x,y)
int x, y;
{
#ifdef UNIX
    register struct sendrec *srec;

    srec = (struct sendrec *)gl_shmemptr->smallbuf; 
    srec->msg = INCHANAT;
    srec->len = 4;
    srec->data[0] = x;
    srec->data[1] = y;
    grioctl(GR_SEND, WINDOWMAN);
    return srec->data[0];
#else 
    return -1;
#endif
}

int txopen()
{
#ifdef UNIX
    register struct sendrec *srec;

    srec = (struct sendrec *)gl_shmemptr->smallbuf; 
    srec->msg = TXOPEN;
    srec->len = 0;
    grioctl(GR_SEND, WINDOWMAN);
    return srec->data[0];
#else 
    return -1;
#endif
}

void txclose(tx)
int tx;
{
#ifdef UNIX
    register struct sendrec *srec;

    srec = (struct sendrec *)gl_shmemptr->smallbuf; 
    srec->msg = TXCLOSE;
    srec->len = 2;
    srec->data[0] = tx;
    grioctl(GR_SEND, WINDOWMAN);
#endif
}

void winconstraints()
{
#ifdef UNIX
    struct sendrec *srec;

    gl_hintinit();
    srec = (struct sendrec *)gl_shmemptr->smallbuf; 
    srec->msg = NEWHINTS;
    srec->len = sizeof(struct portreq);
    gl_initportreq.gfnum = gl_shmemptr->gfnum;
    gl_initportreq.pid = getpid();
    bcopy(&gl_initportreq, srec->data, sizeof(struct portreq));
    grioctl(GR_SEND, -1);
    gl_hintinited = 0;
#endif
}
