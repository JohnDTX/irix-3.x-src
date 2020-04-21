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
#include "grioctl.h"
#include "shmem.h"
#include "glerror.h"
#include "glipc.h"
#include "get.h"

/*
 * 	alloc and free
 *
 */
gl_gralloc()
{
    return grioctl(GR_GRALLOC, 0);
}

/*
 * gr_grinit:
 *	- the other have of gr_alloc - now that we have a grhandle
 *	  get some shared memory and initiliaze it
 */
gl_grinit()
{
    return grioctl(GR_GRINIT, 0);
}

void gl_grfree()
{
    gl_flushmap();
    grioctl(GR_GRFREE);
}

void gl_initinput()
{
    grioctl(GR_INITINPUT);
}

void gl_initallevents()
{
    grioctl(GR_INITBLINK);
}

/*
 * 	generic graphics commands
 *
 */
void gl_ErrorHandler (errno, severity, str, arg0, arg1, arg2, arg3)
long errno, severity;
char *str;
long arg0, arg1, arg2, arg3;
{
    struct errorrec *err = (struct errorrec *)gl_shmemptr->smallbuf;

    err->errno = errno;
    err->severity = severity;
    err->str = str;
    if(str)
	err->slen = strlen(str);
    else
	err->slen = 0;
    err->arg0 = arg0;
    err->arg1 = arg1;
    err->arg2 = arg2;
    err->arg3 = arg3;
    grioctl(GR_ERRORHANDLER,0);
}

long ismex()
{
    return grioctl(GR_ISMEX);
}

void qdevice(arg)
short arg;
{
    grioctl(GR_QDEVICE,arg);
}

void unqdevice(arg)
short arg;
{
    grioctl(GR_UNQDEVICE,arg);
}

long isqueued(arg)
short arg;
{
    if(grioctl(GR_ISQUEUED,arg))
   	return 1;
    else
   	return 0;
}

void signalerror(arg)
short arg;
{
    grioctl(GR_SIGNALERROR,arg);
}

void redirecterrors(arg)
short arg;
{
    grioctl(GR_REDIRECTERRORS,arg);
}

void setvaluator(arg0,arg1,arg2,arg3)
short arg0, arg1, arg2, arg3;
{
    register short *data = gl_shmemptr->smallbuf;

    data[0] = arg0;
    data[1] = arg1;
    data[2] = arg2;
    data[3] = arg3;
    grioctl(GR_SETVALUATOR,0);
}

/* there are NO buttons that have not clearly defined states! */
void setbutton(arg0,arg1)
short arg0, arg1;
{
    return;
}

long getvaluator( arg0 )
short arg0;
{
    return (short)grioctl(GR_GETVALUATOR,arg0);
}

long getbutton( arg0 )
short arg0;
{
    return (Boolean)grioctl(GR_GETBUTTON,arg0);
}

void qreset()
{
    grioctl(GR_QRESET);
}

long qread( arg )
short *arg;
{
    gl_flushmap();
    grioctl(GR_QREAD);
    if(arg)
       *arg = gl_shmemptr->qvalue;
    return gl_shmemptr->qdev;
}

long qtest() 	
{
    return gl_shmemptr->qtop;
}

void qenter( arg0, arg1)
short arg0, arg1;
{
    register short *data = gl_shmemptr->smallbuf;

    data[0] = arg0;
    data[1] = arg1;
    grioctl(GR_QENTER,0);
}

gl_reservebutton(bool, butnum)
short bool, butnum;
{
    register short *data = gl_shmemptr->smallbuf;

    data[0] = bool;
    data[1] = butnum;
    grioctl(GR_RESERVEBUTTON,0);
}

void noise( arg0, arg1)
short arg0, arg1;
{
    register short *data = gl_shmemptr->smallbuf;

    data[0] = arg0;
    data[1] = arg1;
    grioctl(GR_NOISE,0);
}

void tie( arg0, arg1, arg2)
short arg0, arg1, arg2;
{
    register short *data = gl_shmemptr->smallbuf;

    data[0] = arg0;
    data[1] = arg1;
    data[2] = arg2;
    grioctl(GR_TIE,0);
}

void attachcursor( arg0, arg1)
short arg0, arg1;
{
    register short *data = gl_shmemptr->smallbuf;

    data[0] = arg0;
    data[1] = arg1;
    grioctl(GR_ATTACHCURSOR,0);
}

void curson()
{
    grioctl(GR_CURSON);
}

void cursoff()
{
    grioctl(GR_CURSOFF);
}

void mapcolor(index, r, g, b)
short index;
short r, g, b;
{
    register short nextin;
    register struct shmem *sh = gl_shmemptr;

    if (index > gl_wstatep -> bitplanemask) {
	char buf[80];
	sprintf(buf, "mapcolor: %d, bitplanemask: %d",
			index, gl_wstatep -> bitplanemask);
	gl_ErrorHandler(ERR_BADINDEX, WARNING, buf);
	return;
    }
    nextin = (sh->inp+1) & (MAPTABSIZE-1);
    while(sh->outp == nextin)
	;
    sh->indices[nextin] = index;
    sh->rs[nextin] = r;
    sh->gs[nextin] = g;
    sh->bs[nextin] = b;
    sh->inp = nextin;
}

/* wait for mapcolors to be done */
gl_flushmap()
{
    while(gl_shmemptr->outp != gl_shmemptr->inp)
	;
}

void getmcolor(arg0, arg1, arg2, arg3)
short arg0;
short *arg1, *arg2, *arg3;
{
    register short *data = gl_shmemptr->smallbuf;

    if (arg0 > gl_wstatep -> bitplanemask) {
	char buf[80];
	sprintf(buf, "getmcolor: %d, bitplanemask: %d",
			arg0, gl_wstatep -> bitplanemask);
	gl_ErrorHandler(ERR_BADINDEX, WARNING, buf);
	return;
    }
    gl_flushmap();
    data[0] = arg0;
    grioctl(GR_GETMCOLOR,0);
    *arg1 = data[0];
    *arg2 = data[1];
    *arg3 = data[2];
}

long getcmmode () 
{
    return grioctl(GR_GETCMM);
}

void blink( arg0, arg1, arg2, arg3, arg4)
short arg0, arg1;
RGBvalue arg2, arg3, arg4;
{
    register short *data = gl_shmemptr->smallbuf;

    gl_flushmap();
    data[0] = arg0;
    data[1] = arg1;
    data[2] = arg2;
    data[3] = arg3;
    data[4] = arg4;
    grioctl(GR_BLINK,0);
}

void cyclemap( arg0, arg1, arg2)
short arg0, arg1, arg2;
{
    register short *data = gl_shmemptr->smallbuf;

    gl_flushmap();
    data[0] = arg0;
    data[1] = arg1;
    data[2] = arg2;
    grioctl(GR_CYCLEMAP,data);
}

void swapinterval(arg)
short arg;
{
    grioctl(GR_SWAPINTERVAL,arg);
}

void gl_singlebuffer()
{
    grioctl(GR_SINGLEBUFFER);
    gl_personal_buffer_mode = DMSINGLE;
}

void gl_doublebuffer()
{
    grioctl(GR_DOUBLEBUFFER);
    gl_personal_buffer_mode = DMDOUBLE;
}

void setmap (mapnumber)
    short mapnumber;
{
    gl_flushmap();
    grioctl(GR_SETMAP,mapnumber);
}

long getmap () 
{
    return(grioctl(GR_GETMAP));
}


void setmonitor(m)
    register short m;
{
    grioctl(GR_SETMONITOR,m);
}

long getmonitor()
{
    return(grioctl(GR_GETMONITOR));
}

long getothermonitor()
{
    return(grioctl(GR_GETOTHERMONITOR));
}

void setdblights( arg )
long arg;
{
    grioctl(GR_SETDBLIGHTS,arg);
}

void dbtext( arg )
char *arg;
{
    strncpy(gl_shmemptr->smallbuf,arg,8);
    grioctl(GR_DBTEXT,0);
}

void gl_rgbmode()
{
    gl_flushmap();
    grioctl(GR_RGBMODE);
}

long gl_numdbers()
{
    grioctl(GR_NUMDBERS);
}

long gl_numrgbers()
{
    grioctl(GR_NUMRGBERS);
}


void blankscreen(arg)
short arg;
{
    grioctl(GR_BLANKSCREEN,arg);
}

void gl_onemap()
{
    gl_flushmap();
    grioctl(GR_ONEMAP);
}

void gl_multimap()
{
    gl_flushmap();
    grioctl(GR_MULTIMAP);
}


/*
 * 	utilities used internally
 *
 */
void gl_waitforswap()
{
    grioctl(GR_WAITFORSWAP);
}

void gl_setcursoroffset(arg0,arg1)
short arg0, arg1;
{
    register short *data = gl_shmemptr->smallbuf;

    data[0] = arg0;
    data[1] = arg1;
    grioctl(GR_SETCUROFFSET,0);
}

void gl_setcursor(arg0,arg1,arg2)
long arg0;
Colorindex arg1, arg2;
{
    register short *data = gl_shmemptr->smallbuf;

    *(long *)(&data[0]) = arg0;
    data[2] = arg1;
    data[3] = arg2;
    grioctl(GR_SETCURSOR,0);
}

gl_getcursor(addr, color, mask, on)
long	*addr;
Colorindex	*color, *mask;
Boolean	*on;
{
    register short *data = gl_shmemptr->smallbuf;

    grioctl(GR_GETCURSTATE,0);
    *addr = *(long *)(&data[0]);
    *color = data[2];
    *mask = data[3];
    *on = data[4];
}

void gl_RGBsetcursor(arg0,arg1,arg2,arg3,arg4,arg5,arg6)
long arg0;
RGBvalue arg1, arg2, arg3, arg4, arg5, arg6;
{
    register short *data = gl_shmemptr->smallbuf;

    *(long *)(&data[0]) = arg0;
    data[2] = arg1;
    data[3] = arg2;
    data[4] = arg3;
    data[5] = arg4;
    data[6] = arg5;
    data[7] = arg6;
    grioctl(GR_RGBSETCURSOR,0);
}

gl_RGBgetcursor(addr, cr, cg, cb, wtmr, wtmg, wtmb, on)
long	*addr;
short	*cr, *cg, *cb, *wtmr, *wtmg, *wtmb;
Boolean	*on;
{
    register short *data = gl_shmemptr->smallbuf;

    grioctl(GR_RGBGETCURSTATE,0);
    *addr = *(long *)(&data[0]);
    *cr = data[2];
    *cg = data[3];
    *cb = data[4];
    *wtmr = data[5];
    *wtmg = data[6];
    *wtmb = data[7];
    *on = data[8];
}

short gl_freepages()
{
    return grioctl(GR_FREEPAGES);
}


/*
 * 	textport stuff
 *
 */
void textinit()
{
    grioctl(GR_TEXTINIT, gl_shmemptr->inputchan);
}

void gl_textrefresh()
{
    grioctl(GR_TEXTREFRESH);
}

void tpon()
{
    grioctl(GR_TPON, gl_shmemptr->inputchan);
    gl_flushtext();
}

void tpoff()
{
    grioctl(GR_TPOFF, gl_shmemptr->inputchan);
}

void textport(arg0,arg1,arg2,arg3)
short arg0, arg1, arg2, arg3;
{
    struct sendrec *srec;
    struct textportreq *tpr;
    register short *data = gl_shmemptr->smallbuf;

    data[0] = gl_shmemptr->inputchan;
    data[1] = arg0;
    data[2] = arg1;
    data[3] = arg2;
    data[4] = arg3;
    grioctl(GR_TEXTPORT,0);
    gl_flushtext();
}

void gettp(arg0,arg1,arg2,arg3)
short *arg0, *arg1, *arg2, *arg3;
{
    register short *data = gl_shmemptr->smallbuf;

    data[0] = 0;
    grioctl(GR_GETTEXTPORT,0);
    *arg0 = data[0];
    *arg1 = data[1];
    *arg2 = data[2];
    *arg3 = data[3];
}

void textcolor(arg0)
short arg0;
{
    register short *data = gl_shmemptr->smallbuf;

    data[0] = gl_shmemptr->inputchan;
    data[1] = arg0;
    grioctl(GR_TEXTCOLOR,0);
    gl_flushtext();
}

void textwritemask(arg0)
short arg0;
{
    register short *data = gl_shmemptr->smallbuf;

    data[0] = gl_shmemptr->inputchan;
    data[1] = arg0;
    grioctl(GR_TEXTWRITEMASK,0);
    gl_flushtext();
}

void pagecolor(arg0)
short arg0;
{
    register short *data = gl_shmemptr->smallbuf;

    data[0] = gl_shmemptr->inputchan;
    data[1] = arg0;
    grioctl(GR_PAGECOLOR,0);
    gl_flushtext();
}

void pagewritemask(arg0)
short arg0;
{
    register short *data = gl_shmemptr->smallbuf;

    data[0] = gl_shmemptr->inputchan;
    data[1] = arg0;
    grioctl(GR_PAGEWRITEMASK,0);
    gl_flushtext();
}

gl_getcharinfo(width,height,descender)
short *width, *height, *descender;
{
    register short *data = gl_shmemptr->smallbuf;

    grioctl(GR_GETCHARINFO,0);
    *width = data[0];
    *height = data[1];
    *descender = data[2];
}

gl_getnumchars(nc)
short *nc;
{
    register short *data = gl_shmemptr->smallbuf;

    data[0] = 0;
    *nc = MINFONTNC;
    if (grioctl(GR_GETNUMCHARS, 0) >= 0) {
	if ((MINFONTNC <= data[0]) && (data[0] <= MAXFONTNC))
	    *nc = data[0];
    }
}

gl_getcharoffsets(offsets)
  Fontchar offsets[];
{
    register short *data = gl_shmemptr->smallbuf;

    data[0] = MAXFONTNC;	/* Indicate we can take max # chars.
				 * (This will be ignored by old kernels.)
				 */
    grioctl(GR_GETCHAROFFSETS, offsets);
}

gl_flushtext()
{
    grioctl(GR_SAFE);
}

/*
 * 	keyboard stuff
 *
 */
void clkon()
{
    grioctl(GR_CLKON);
}

void clkoff()
{
    grioctl(GR_CLKOFF);
}

void lampon(arg)
short arg;
{
    grioctl(GR_LAMPON,arg);
}

void lampoff(arg)
short arg;
{
    grioctl(GR_LAMPOFF,arg);
}

void setbell(arg)
short arg;
{
    grioctl(GR_SETBELL,arg);
}

void ringbell()
{
    grioctl(GR_RINGBELL);
}

void gl_reservebitplanes(num)
short	num;
{
    grioctl(GR_RESERVEBITS, num);
}

gl_startfeed()
{
    if(grioctl(GR_STARTFEED) == -1) {
	gl_ErrorHandler(ERR_STARTFEED, FATAL, "gl_startfeed");
	exit(1);
    }
}

gl_endfeed()
{
    grioctl(GR_ENDFEED);
}

gl_getshmem(buf,chan)
    long *buf;
    short chan;
{
    register short *data = gl_shmemptr->smallbuf;

    ((long *)data)[0] = (long)buf;
    data[2] = chan;
    return grioctl(GR_GETSHMEM);
}

gl_getaddrs(buf,chan)
    long *buf;
    short chan;
{
    register short *data = gl_shmemptr->smallbuf;
    long ics;

    data[0] = chan;
    ics = grioctl(GR_GETADDRS);
    *buf = ((long *)data)[0];
    return ics;
}

void getdev(n,devs,vals)
int n;
short *devs;
short *vals;
{
    register short *data = gl_shmemptr->smallbuf;

    if(n>128) 		/* this 128 should be SMALLBUFSIZE */
	n = 128;	/* this should be an error */
    bcopy(devs,data,n*sizeof(short));
    grioctl(GR_GETDEV,n);
    bcopy(data,vals,n*sizeof(short));
}

void devport(dev,port)
int dev, port;
{
    register short *data = gl_shmemptr->smallbuf;

    data[0] = dev;
    data[1] = port;
    grioctl(GR_DEVPORT);
}

void blanktime(n)
int n;
{
    grioctl(GR_SCRTIMEOUT,n);
}

void gl_lpenset(val)
int val;
{
    grioctl(GR_LPENSET,val);
}

void gl_anyqenter(port,dev,val)
int port, dev, val;
{
    register short *data = gl_shmemptr->smallbuf;

    data[0] = port;
    data[1] = dev;
    data[2] = val;
    grioctl(GR_ANYQENTER);
}

short *gl_smallbufaddr()
{
    return gl_shmemptr->smallbuf;
}
