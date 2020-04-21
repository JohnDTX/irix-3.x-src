/*
 *	provide hints to the window manager 
 *
 *			Paul Haeberli - 1984
 *
 */
#include "gl.h"
#include "device.h"
#include "glipc.h"
#include "grioctl.h"
#include "globals.h"
#include "shmem.h"
#include "signal.h"
#include "stdio.h"

#define MIN( a, b )	(((a) < (b)) ? (a) : (b))
#define MAX( a, b )	(((a) > (b)) ? (a) : (b))
#define ABS( a )	(((a) > 0) ? (a) : -(a))

extern struct portreq gl_initportreq;
extern short gl_putinbackground;
extern int glpl_ginited;

/*
 *	minsize - hint at the minimum size the port may be. 
 *
 */
void minsize( x, y )
long x, y;
{
#ifdef UNIX
    gl_hintinit();
    gl_initportreq.minsizex = x;
    gl_initportreq.minsizey = y;
#endif
}

/*
 *	maxsize - hint at the maximum size the port may be. 
 *
 */
void maxsize( x, y )
long x, y;
{
#ifdef UNIX
    gl_hintinit();
    gl_initportreq.maxsizex = x;
    gl_initportreq.maxsizey = y;
#endif
}

/*
 *	keepaspect - hint that the aspect ratio should be preserved
 *
 */
void keepaspect( x, y )
long x, y;
{
#ifdef UNIX
    gl_hintinit();
    gl_initportreq.keepaspect = 1;
    gl_initportreq.aspectx = x;
    gl_initportreq.aspecty = y;
#endif
}

/*
 *	prefsize - hint at the preferred size
 *
 */
void prefsize( x, y )
long x, y;
{
#ifdef UNIX
    gl_hintinit();
    gl_initportreq.prefsize = 1;
    gl_initportreq.prefsizex = x;
    gl_initportreq.prefsizey = y;
#endif
}
/*
 *	stepunit - hint at the discrete step size
 *
 */
void stepunit( x, y )
long x, y;
{
#ifdef UNIX
    gl_hintinit();
    gl_initportreq.xunit = x;
    gl_initportreq.yunit = y;
#endif
}

/*
 *	fudge - hint at the port fudge factor
 *
 */
void fudge( x, y )
long x, y;
{
#ifdef UNIX
    gl_hintinit();
    gl_initportreq.xunitfudge = x;
    gl_initportreq.yunitfudge = y;
#endif
}

/*
 *	prefposition - hint at the preferred position
 *
 */
void prefposition( x1, x2, y1, y2 )
long x1, x2, y1, y2;
{
#ifdef UNIX
    gl_hintinit();
    gl_initportreq.preforigin = 1;
    gl_initportreq.preforiginx = MIN(x1,x2);
    gl_initportreq.preforiginy = MIN(y1,y2);
    gl_initportreq.prefsize = 1;
    gl_initportreq.prefsizex = ABS(x2-x1) + 1;
    gl_initportreq.prefsizey = ABS(y2-y1) + 1;
#endif
}

/*
 *	noport - hint that we don't need a port at all!
 *
 */
void noport()
{
#ifdef UNIX
    gl_hintinit();
    gl_initportreq.preforigin = 1;
    gl_initportreq.prefsize = 1;
    gl_initportreq.prefsizex = 0;
    gl_initportreq.prefsizey = 0;
#endif
}

/*
 *	foreground - hint that we dont want to end up in background
 *
 */
void foreground()
{
#ifdef UNIX
     gl_hintinit();
     gl_putinbackground = 0;
#endif
}

/*
 *	imakebackground - hint that we are a background
 *
 */
void imakebackground()
{
#ifdef UNIX
     gl_hintinit();
     gl_initportreq.imakebackground = 1;
#endif
}
/*
 *	getpor - FORTRAN version of getport()
 *
 */
void getpor(name, len)
char *name;
long len;
{
#ifdef UNIX
    long xsize, ysize;

    gl_hintinit();		/* CSK  2/26/86 */
    if (gl_putinbackground && grioctl(GR_ISMEX,0)) {
	/* only if mex is running */
	fflush(stdout);				/* peter 6/20/85 */
	switch (fork()) {
	    case -1:
		perror("getport");
		exit(1);
	    case 0:
		signal(SIGINT,SIG_IGN);
		signal(SIGQUIT,SIG_IGN);
		signal(SIGTERM,SIG_DFL);	/* peter 9/9/85 */
		break;
	    default:
		exit(0);
	}
    }
    gl_winname(name);
    gbegin();
    qdevice(INPUTCHANGE);
    qdevice(REDRAW);
    reshapeviewport();
    getsize(&xsize,&ysize);
    ortho2(-0.5,xsize-0.5,-0.5,ysize-0.5);
    glpl_ginited = 1;
#endif

#ifdef V
    gbegin();
#endif
}


/*
 *	getport - get the port for this application
 *
 */
void getport(name)
char *name;
{
#ifdef UNIX
    if (name == NULL)
	name = "";
    getpor(name, strlen(name));
#endif
}

/*
 *	getsize - get the size of the port for this application
 *		  This should only be called after getport.
 */
void getsize( x, y )
long *x, *y;
{
#ifdef UNIX
    *x = 1 + gl_shmemptr->ws.xmax - gl_shmemptr->ws.xmin;
    *y = 1 + gl_shmemptr->ws.ymax - gl_shmemptr->ws.ymin;
#endif
}

/*
 *	getorigin - get the origin of the port of this application
 */
void getorigin( x, y )
long *x, *y;
{
#ifdef UNIX
    *x = gl_shmemptr->ws.xmin;
    *y = gl_shmemptr->ws.ymin;
#endif
}

void screenspace()
{
#ifdef UNIX
    long xorg, yorg; 

    getorigin(&xorg,&yorg);
    viewport(0-xorg,XMAXSCREEN-xorg,0-yorg,YMAXSCREEN-yorg);
    ortho2(-0.5,XMAXSCREEN.5,-0.5,YMAXSCREEN.5);
#endif
}

/*
 * 	reshapeviewport - set viewport to current screen dimensions
 */
void reshapeviewport()
{
#ifdef UNIX
    viewport(0, gl_shmemptr->ws.xmax - gl_shmemptr->ws.xmin,
			     0, gl_shmemptr->ws.ymax - gl_shmemptr->ws.ymin);
#endif
}
