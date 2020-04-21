/**************************************************************************
 *									  *
 * 		 Copyright (C) 1984, Silicon Graphics, Inc.		  *
 *									  *
 *  These coded instructions, statements, and computer programs  contain  *
 *  unpublished  proprietary  information of Silicon Graphics, Inc., and  *
 *  are protected by Federal copyright law.  They  may  not be disclosed  *
 *  in part, without the prior written consent of Silicon Graphics, Inc.  *
 *									  *
 **************************************************************************/
#include "sys/types.h"
#include "shmem.h"
#include "window.h"
#include "errno.h"
#include "kb.h"
#include "grioctl.h"
#include "glipc.h"
#include "kfont.h"

#define TEMPBUFLEN 	100
static char tempbuf[TEMPBUFLEN];
extern short txneedsrepaint;
extern int defont_nc;

gr_ioctl( cmd, arg )
	register long cmd;
	register unsigned long arg;
{
	register short *data = gl_shmemptr->smallbuf;
	register unsigned short i;
	register struct inputchan *ic;

	gl_ioerror = 0;
	switch(cmd) {
	  case 0xbad:
		for(i=0; i<1; i++) {
		    saveeverything();
		    restoreeverything();
		}
		break;
	  case GR_ISMEX:
		return(gl_wmport ? 1 : 0);
	  case GR_GRALLOC:			/* alloc and free */
		return gr_alloc();
	  case GR_GRINIT:
		return;
	  case GR_GRFREE:
		gr_free(gr_getoshandle());
		break;
	  case GR_LOSEGR:
		gr_setgrhandle(gr_getoshandle(), 0);
		break;
	  case GR_INITINPUT:
		gl_initinput(getic());
		break;
	  case GR_NULL:
		break;
	  case GR_INITBLINK:
		gl_initblinkevents();
		gl_initcyclemap();
		break;

	  case GR_ERRORHANDLER:			/* general graphics */
		{
		    struct errorrec *er =
				(struct errorrec *)gl_shmemptr->smallbuf;

		    if(er->str) {
		        if(copyin(er->str,tempbuf,MIN(TEMPBUFLEN,er->slen+1))) {
			    gl_ioerror = EFAULT;
			    break;
		        }
		        tempbuf[TEMPBUFLEN-1] = 0;
		        gl_ErrorHandler(er->errno, er->severity, tempbuf, 
				    er->arg0, er->arg1, er->arg2, er->arg3);
		    } else
		        gl_ErrorHandler(er->errno, er->severity, 0);
		}
		break;
	  case GR_QDEVICE: 				/* queue */
		qdevice(arg);
		break;
	  case GR_UNQDEVICE: 				/* unqueue */
		unqdevice(arg);
		break;
	  case GR_SIGNALERROR: 
		signalerror();
		break;
	  case GR_REDIRECTERRORS: 
		redirecterrors(arg);
		break;
	  case GR_SETVALUATOR:			/* set and get */
		setvaluator(data[0],data[1],data[2],data[3]);
		break;
	  case GR_GETVALUATOR:
		return getvaluator(arg);
		break;
	  case GR_GETBUTTON: 
		return getbutton(arg);
	  case GR_QRESET:				/* queue control */ 
		qreset(getic());
		break;
	  case GR_QREAD:
		doqread();
		break;
	  case GR_QENTER:
		qenter(data[0],data[1]); 
		break;
	  case GR_NOISE:				/* device control */ 
		noise(data[0],data[1]); 
		break;
	  case GR_TIE: 
		tie(data[0],data[1],data[2]);
		break;
	  case GR_ATTACHCURSOR: 
		attachcursor(data[0],data[1]); 
		break;
	  case GR_CURSON:
		curson(1);
		break;
	  case GR_CURSOFF:
		cursoff();
		break;
	  case GR_MAPCOLOR:
		mapcolor(data[0], data[1], data[2], data[3]);
		break;
	  case GR_GETMCOLOR:
		getmcolor(data[0], &data[0], &data[1], &data[2]);
		break;
	  case GR_BLINK:
		blink(data[0],data[1],data[2],data[3],data[4]);
		break;
	  case GR_CYCLEMAP:
		cyclemap(data[0],data[1],data[2]);
		break;
	  case GR_GSYNC:
		dogsync();
		break;
	  case GR_BLANKSCREEN:
		blankscreen(arg);
		break;
	  case GR_SWAPINTERVAL:
		swapinterval(arg);
		break;
	  case GR_WAITFORSWAP:		/* internal utilities */
		dowaitforswap();
		break;
	  case GR_SINGLEBUFFER:
		gl_setsinglebuffer();
		break;
	  case GR_DOUBLEBUFFER:
		gl_setdoublebuffer();
		break;
	  case GR_RGBMODE:
		gl_setrgbmode();
		break;
	  case GR_NUMDBERS:
		return gl_getnumdbers();
		break;
	  case GR_NUMRGBERS:
		return gl_getnumrgbers();
		break;
	  case GR_ONEMAP:
		gl_setonemap();
		break;
	  case GR_MULTIMAP:
		gl_setmultimap();
		break;
	  case GR_SETMAP:
		gl_setmap(arg);
		break;
	  case GR_GETMAP:
		return gl_getmap();
		break;
	  case GR_GETCMM:
		return gl_getcmmode();
		break;
	  case GR_SETMONITOR:
		gl_setmonitor(arg);
		break;
	  case GR_GETMONITOR:
		return gl_getmonitor();
		break;
	  case GR_SETDBLIGHTS:
		setdblights(arg);
		break;
	  case GR_DBTEXT:
		dbtext(data);
		break;
	  case GR_ISQUEUED:
	    	if ( !(ic = getic()) )
			goto invalid;
		return isqueued(ic,arg);
		break;

	  case GR_SETCUROFFSET:
	    	if ( !(ic = getic()) )
			goto invalid;
		ic->ic_curoffsetx = data[0];
		ic->ic_curoffsety = data[1];
		if(!gl_wmport || ic == gl_curric) 
		    gl_setcuroffset(data[0],data[1]);
		break;
	  case GR_SETCURSOR:
	    	if ( !(ic = getic()) )
			goto invalid;
		if(!gl_wmport || (ic == gl_wmport) ) { /* wm setting */
		    if(ic == gl_curric)		/* wm is current input */
			gl_setcursor(*(long *)(&data[0]), data[2], data[3]);
		    else			/* changing masks only */
			gl_setcursor(gl_cursoraddr, data[2], data[3]);
		}
		else if(ic == gl_curric) {
		    gl_setcursor(*(long *)(&data[0]), 
					      gl_cursorcolor, gl_cursorwenable);
		}
		break;
	  case GR_RGBSETCURSOR:
		gl_RGBsetcursor(*(long *)(&data[0]),data[2],
			data[3], data[4], data[5], data[6], data[7]);
		break;
	  case GR_GETCURSTATE:
		gl_getcurstate(data,data+2,data+3,data+4);
		break;
	  case GR_RGBGETCURSTATE:
		gl_RGBgetcurstate(data,data+2,data+3,data+4,data+5,
						data+6,data+7,data+8);
		break;
	  case GR_FREEPAGES:
		return gr_freepages();
		break;

	  case GR_SYSTYPE:
		return gr_systype();
		break;

	  case GR_WRITEMICRO:
		{
		    int ldat[4];

		    if (copyin(arg,ldat,3*sizeof(int))) {
			gl_ioerror = EFAULT;
			break;
		    }
		    if (gl_microwrite(ldat[0],ldat[1],ldat[2]) >= 0)
			iprintf("GR_WRITEMICRO: micro write error\n");
		}
		break;

	  case GR_SCRTIMEOUT:
	        gl_scrtimeout(arg);
		kunblanklater();
		break;
	  case GR_WINSOFTINTR:
		if(gl_textportno >= 0)
			win_softintr(gl_textportno, arg&0xff, 0);
		break;
	  case GR_KBDSOFTINTR:
		kb_translate(arg&0xff);
		break;
	  case GR_CHANGEBUTTON:
		{
		    register short dev, val;

		    dev = (arg>>16)&0xffff;
		    val = arg&0xffff;
		    if(ISBUTTON(dev))
			ChangeButton(dev,val);
		}
		break;
	  case GR_CHANGEVALUATOR:
		{
		    register short dev, val;

		    dev = (arg>>16)&0xffff;
		    val = arg&0xffff;
		    if(ISVALUATOR(dev))
			ChangeValuator(dev,val);
		}
		break;
	  case GR_MEKBMAN:
	        if ( !(ic = getic()) )
		    goto invalid;
	        gl_kbport = ic;
		break;

	  case GR_TEXTINIT:		/* txport stuff */
		if (arg >= NTXPORTS)
			goto invalid;
		tx_initcolors(&txport[arg]);
		tx_textport(&txport[arg], 0,XMAXSCREEN,0,YMAXSCREEN);
		break;
	  case GR_TEXTREFRESH:
		txport[0].tx_state |= TX_REDISPLAY|TX_SCROLLED|TX_DRAWBORDER;
		break;
	  case GR_TPON:
		if (arg >= NTXPORTS)
			goto invalid;
		txport[arg].tx_state |= TX_ON;
		break;
	  case GR_TPOFF:
		if (arg >= NTXPORTS)
			goto invalid;
		txport[arg].tx_state &= ~TX_ON;
		break;
	  case GR_TEXTPORT:
		if((unsigned short)data[0] >= NTXPORTS) {
			gl_ioerror = EINVAL;
			break;
		}
		tx_textport(&txport[data[0]],
				    data[1],data[2],data[3],data[4]);
		break;
	  case GR_GETTEXTPORT:
		if((unsigned short)data[0] >= NTXPORTS)
			goto invalid;
		tx_gettextport(&txport[data[0]],
				&data[0],&data[1],&data[2],&data[3]);
		break;
	  case GR_TEXTWRITEMASK:
		if((unsigned short)data[0] >= NTXPORTS)
			goto invalid;
		tx_newtextwritemask(&txport[data[0]],data[1]);
		break;
	  case GR_TEXTCOLOR:
		if((unsigned short)data[0] >= NTXPORTS)
			goto invalid;
		tx_newtextcolor(&txport[data[0]], data[1]);
		break;
	  case GR_PAGECOLOR:
		if((unsigned short)data[0] >= NTXPORTS)
			goto invalid;
		tx_newpagecolor(&txport[data[0]], data[1]);
		break;
	  case GR_PAGEWRITEMASK:
		if((unsigned short)data[0] >= NTXPORTS)
			goto invalid;
		tx_newpagewritemask(&txport[data[0]],data[1]);
		break;

	  case GR_GETCHARINFO:
		data[0] = gl_charwidth;
		data[1] = gl_charheight;
		data[2] = gl_chardescender;
		break;
	  case GR_GETNUMCHARS:
		data[0] = defont_nc;
		break;

	  case GR_SETCHARINFO:
		if(data[0]) 
		    gl_charwidth = data[0];
		if(data[1]) 
		    gl_charheight = data[1];
	        gl_chardescender = data[2];
		tx_textport(&txport[0], 0, XMAXSCREEN, 0, YMAXSCREEN);
		break;
	  case GR_GETCHAROFFSETS:
		/* allow compatiblity with old libgl2.a's, in which
		 * initglob won't have data[3] set = MAXFONTNC
		 */
		i = (data[3] == MAXFONTNC) ? defont_nc : MINFONTNC;
		if(copyout(defont_font, arg, i*sizeof(struct fontchar))) {
		    gl_ioerror = EFAULT;
		}
		break;
	  case GR_SETCHAROFFSETS:
		if ((MINFONTNC <= data[0]) && (data[0] <= MAXFONTNC))
		    defont_nc = data[0];
		else	/* just in case an old loadfont is used */
		    defont_nc = MINFONTNC;
		if(copyin(arg,defont_font,defont_nc*sizeof(struct fontchar)))
		    gl_ioerror = EFAULT;
		else
		    shiftfontbase(defont_font,defont_nc);
		break;
	  case GR_SETCHARMASKS:
		{
		    long uaddr;
		    long nmasks, foaddr, thistime;

		    uaddr = *(long *)(&data[0]);
		    nmasks = data[2];
		    if(nmasks>FR_MAXKFONTSIZE)
			    nmasks = FR_MAXKFONTSIZE;
		    foaddr = FR_DEFFONT;
		    gl_lowfont(nmasks);
		    gl_shmemptr->ws.fontrambase = gl_fontslot();
		    gl_shmemptr->ws.fontramlength = FONTRAM_STARTSIZE;
		    while(nmasks) {
			    thistime = MIN(TEMPBUFLEN/sizeof(short),nmasks);
			    if(copyin(uaddr,tempbuf,thistime<<1)) {
			        gl_ioerror = EFAULT;	
			        break;
			    }
			    setfontbaseaddr(0);
			    gl_loadmasks(foaddr,tempbuf,thistime);
			    setfontbaseaddr(gl_shmemptr->ws.fontrambase);
			    foaddr+=thistime;
			    nmasks-=thistime;
			    uaddr+=thistime*sizeof(short);
		    }
		}
		break;

	  case GR_CLKON:			/* keyboard stuff */
		kb_setclick(1);
		break;
	  case GR_CLKOFF:
		kb_setclick(0);
		break;
	  case GR_LAMPON:
		kb_setlamp(1,arg);
		break;
	  case GR_LAMPOFF:
		kb_setlamp(0,arg);
		break;
	  case GR_SETBELL:
		kb_setbell(arg);
		break;
	  case GR_RINGBELL:
		kb_ringbell();
		break;

	  case GR_GFINPUTCHANNEL:			/* gfport stuff */
		if (arg >= NGFPORTS)
			gfinputchannel(-1);
		else
			gfinputchannel(arg);
		if(gl_curric) {
		    int pri;
	            long ctx;
		    long cursoraddr;

		    pri = spl6();
		    ic = gl_curric;
		    ctx = gr_setshmem(ic->ic_oshandle);
		    if(gl_shmemptr->ws.cursorbase >= 0)
			    cursoraddr = gl_shmemptr->ws.cursorbase+
					          gl_shmemptr->ws.fontrambase;
		    else
			    cursoraddr = FR_DEFCURSOR;
		    gr_restoreshmem(ctx);
		    splx(pri);
		    gl_setcursor(cursoraddr, gl_cursorcolor, gl_cursorwenable);
		    gl_setcuroffset(ic->ic_curoffsetx,ic->ic_curoffsety);
		}
		break;
	  case GR_TXINPUTCHANNEL:		
		if (arg >= NTXPORTS)
			txinputchannel(-1);
		else
			txinputchannel(arg);
		break;
	  case GR_RESERVEBUTTON:		
	        ic = getic();
		if((data[0] == 0) && (gl_buttons[data[1]].reserved == ic))
			gl_buttons[data[1]].reserved = 0;
		else if((data[0] == 1) && (!gl_buttons[data[1]].reserved))
			gl_buttons[data[1]].reserved = ic;
		break;
	  case GR_GETINCHAN:
		ic = &inchan[1];
		for(i=1; i<NINCHANS; i++, ic++) {
#ifdef NOTDEF
			if (!ic->ic_oshandle && !ic->ic_shmemptr)
#endif
			if (!ic->ic_shmemptr)
				return i;
		}
		gl_ioerror = EBUSY;
		return -1;
	  case GR_GETGFPORT:
		return getgfport();
	  case GR_GETTXPORT:
		{
		    register struct txport *tx;
		    tx = &txport[1];
		    for(i=1; i<NTXPORTS; i++, tx++) {
			if (!(tx->tx_state & TX_OPEN))
				return i;
		    }
		    gl_ioerror = EBUSY;
		}
		return -1;
	  case GR_PUTINCHAN:
		if(arg >= NINCHANS)
			goto invalid;
		ic = &inchan[(unsigned short) arg];
		if (ic->ic_oshandle) {
			gr_kill(ic->ic_oshandle);
			gr_free(ic->ic_oshandle);
		}
		/*
		 * Clear holding flag now that window manager has
		 * blessed this inchan for reusage
		 */
		ic->ic_holding = 0;
		break;
	  case GR_PUTGFPORT:
		{
		    register struct gfport *gf;
		    if(arg >= NGFPORTS)
			goto invalid;
		    gf = &gfport[(unsigned short) arg];
/* hack: need to check for double bufferedness here or something?? */
		    if (gf->gf_ic == (ic = getic())) {
			if(gl_wmport)
				gr_qenter(gl_wmport, WMGFCLOSE, arg); 
			if(ic->ic_gf == gf)
				ic->ic_gf = gf->gf_next;
			else {
				register struct gfport *gf2;

				for(gf2 = ic->ic_gf; gf2->gf_next != gf;
							gf2 = gf2->gf_next) ;
				gf2->gf_next = gf->gf_next;
			}
			gf->gf_ic = 0;
		    }
		}
		break;
	  case GR_PUTTXPORT:
		if (arg >= NTXPORTS)
			goto invalid;
		if(arg > 0)
		    tx_close(arg);
/* do something intelligent here like killing off everyone using this tty*/
		break;
	  case GR_SETGFPORT:
		return setgfport(*(long *)&gl_shmemptr->smallbuf[0],
		*(long *)&gl_shmemptr->smallbuf[2], gl_shmemptr->smallbuf[4]);
	  case GR_SETPIECE:
		return setpieces(arg);
	  case GR_SAFE:	
		tx_repaint(1);
		break;
	  case GR_RESERVEBITS:
		reservebits(arg);
		break;
	  case GR_STARTFEED:
		if(gr_startfeed())
			goto invalid;
		break;
	  case GR_ENDFEED:
		if(gr_endfeed())
			goto invalid;
		break;
	  case GR_SETFBC:
		gl_fbcstatus = data[0];
		gl_gestatus = data[1];
		break;
	  case GR_GETFBC:
		data[0] = gl_fbcstatus;
		data[1] = gl_gestatus;
		break;

	  case GR_LOCK:
		gl_lock = 1;
		break;
	  case GR_UNLOCK:
		gl_lock = 0;
		break;
	  case GR_SEND:
		if ( !(ic = getic()) )
			goto invalid;
		else {
			register struct inputchan *targetic;

			if (arg == -1) {
				if ((targetic = gl_wmport) == NULL)
					break;
			} else {
				if (arg >= NGFPORTS)
					goto invalid;
				targetic = &inchan[(unsigned short)arg];
			}
			if ((targetic != ic) && targetic->ic_shmemptr) {
			    gr_qenter(targetic, WMSEND, ic->ic_no);
			    ic->ic_sendwait = 1;
#ifdef UNIX
			    while(ic->ic_sendwait)
			        gr_sleep(ic->ic_oshandle, ic);
#else
			    gr_sleep(ic->ic_oshandle, ic);
#endif
			}
		}
		break;
	  case GR_REPLY:			/* answer a send */
		if ( !(ic = getic()) )
			goto invalid;
		if (arg >= NGFPORTS)
			goto invalid;
		ic = &inchan[arg];
		if (ic->ic_shmemptr && ic->ic_sendwait) {
		        ic->ic_sendwait = 0;
		        gr_wakeup(ic->ic_oshandle, ic);
		} else
			goto invalid;
		break;
	  case GR_SHREAD:		/* read from other guys shmem */
		{
		    struct sendrec dat;
		    int pri;
	            long ctx;

		    if ( !(ic = getic()) )
			goto invalid;
		    if (arg >= NGFPORTS)
			goto invalid;
		    ic = &inchan[arg];

		    pri = spl6();
		    ctx = gr_setshmem(ic->ic_oshandle);
		    bcopy(gl_shmemptr->smallbuf, &dat, sizeof(dat));
		    gr_restoreshmem(ctx);
		    splx(pri);

		    bcopy(&dat, gl_shmemptr->smallbuf, sizeof(dat));
		}
		break;
	  case GR_SHWRITE:		/* write to other guys shmem */
		{
		    struct sendrec dat;
		    int pri;
	            long ctx;

		    if ( !(ic = getic()) )
			goto invalid;
		    if (arg >= NGFPORTS)
			goto invalid;
		    ic = &inchan[arg];

		    bcopy(gl_shmemptr->smallbuf, &dat, sizeof(dat));

		    pri = spl6();
		    ctx = gr_setshmem(ic->ic_oshandle);
		    bcopy(&dat, gl_shmemptr->smallbuf, sizeof(dat));
		    gr_restoreshmem(ctx);
		    splx(pri);
		}
		break;
	  case GR_MEWMAN:		/* declare us as a port manager */
		if (gl_wmport)
			goto invalid;
		else {
			extern long gl_planes;

			if ( !(ic = getic()) )
				goto invalid;
			gl_wmport = ic;
			setbitplanemasks(gl_planes);
			ch_init();
		}
		break;
	  case GR_FONTMEM:		/* set size of users fontram piece */
		return gl_setfontmem(arg);
		break;
	  case GR_GETSHMEM:		/* get some chanels shmem */
		{
		    int copyresult;
		    int pri;
	            long ctx;
		    long dest;

		    if ( !(ic = getic()) )
			goto invalid;
		    if (arg >= NGFPORTS)
			goto invalid;
		    ic = &inchan[data[2]];
		    dest = ((long *)data)[0];

		    pri = spl6();
		    ctx = gr_setshmem(ic->ic_oshandle);
#ifdef NOTDEF
		    copyresult =
			!(!ic->ic_oshandle && !ic->ic_shmemptr) ?
			copyout(gl_shmemptr, dest, sizeof(struct shmem)) : -1;
#endif
		    copyresult =
			(ic->ic_shmemptr) ?
			copyout(gl_shmemptr, dest, sizeof(struct shmem)) : -1;
		    gr_restoreshmem(ctx);
		    splx(pri);
		    if(copyresult) {
			gl_ioerror = EFAULT;
			goto okay;
		    }
		    return sizeof(struct shmem);
		}
		break;

	  case GR_GETCONFSW:
#ifdef UNIX
#ifdef PM2
		return *(unsigned char *)CONFIG_REG;
			/* don't want sign extention */
#endif
#ifdef IP2
		printf("Cannot get configuration switch for this machine\n");
		return 1;
#endif
#endif UNIX
		break;
	  case GR_SWAPANYTIME:
	        if ( !(ic = getic()) )
		    goto invalid;
		gl_wmswapanytime = 1;
		break;
	  case GR_GETADDRS:		/* get interesting addresses */
		{
		    ic = &inchan[data[0]];	/* chanel of interest */
		    ((long *)data)[0] = (long)ic;
		    return (int)inchan;		/* base of all inputchanels */
		}
		break;
	  case GR_GETOTHERMONITOR:		/* the other monitor type */
		{
		    return gl_getothermonitor();
		    break;
		}
	  case GR_SIGCON:
		{
		    register short dev, val;

		    if ( !(ic = getic()) )
			    goto invalid;
		    dev = (arg>>16)&0xffff;
		    val = arg&0xffff;
		    if(ISOUTPUT(dev))
			ch_signal(ic,dev,val);
		}
		break;
	  case GR_MODCON:
	        return ch_modify(data[0],data[1],data[2],data[3],data[4]);
		break;
	  case GR_REPLYCON:
		if(arg >= NINCHANS)
			goto invalid;
		ic = &inchan[(unsigned short) arg];
		ch_reply(ic);
		break;
	  case GR_GETDEV:
		if ( !(ic = getic()) )
		    goto invalid;
		if(arg>128)
		    goto invalid;
		gl_getdev(arg,data);
		break;
	  case GR_DEVPORT:
		gl_devport(data[0],data[1]);
		break;
	  case GR_MOUSEWARP:
		gl_mousewarp(data[0],data[1]);
		break;
	  case GR_LPENSET:
		gl_lpenset(arg);
		break;
	  case GR_ANYQENTER:
		gl_anyqenter(data[0],data[1],data[2]);
		break;
#ifdef CLOVER
	  case GR_MESIMGUY:
	        if ( !(ic = getic()) )
		    goto invalid;
	        gl_simport = ic;
		break;
	  case GR_SENDGE:
		if ( !(ic = getic()) )
			goto invalid;
		else {
			register struct inputchan *targetic = gl_simport;

			if (targetic && (targetic != ic) &&
				     targetic->ic_shmemptr) {
				gr_qenter(targetic, WMSEND, ic->ic_no);
			    ic->ic_sendwait = 1;
			    while(ic->ic_sendwait)
			        gr_sleep(ic->ic_oshandle, ic);
			}
		}
		break;
#endif
	  default:
		printf("unknown grioctl cmd: %d\n",cmd);
		gl_ioerror = EINVAL;
	}

okay:
	return 0;

invalid:
	gl_ioerror = EINVAL;
	return -1;
}

/*
 * setpieces:
 *	- read a piece list from the user
 *	- certain simple checks are made for data integrity; however, the
 *	  pieces themselves are unchecked
 *	- pieces are assumed to be contiguous in piece array
 */
setpieces(uaddr)
    register char *uaddr;
{
    register struct gfport *gf;
    register struct txport *tx;
    register struct piece *pp;
    register short i;
    register unsigned short npieces, type;
    register short error = 0;
    register int pri;
    struct grpiecehdr piecehdr;
    struct grpiece piece;

    if (copyin(uaddr, (caddr_t)&piecehdr, sizeof(piecehdr)))
	return EFAULT;
    uaddr += sizeof(piecehdr);

/* make sure # of pieces is ok */
    npieces = piecehdr.gr_pieces;
    if (npieces > NPIECES)
	return EINVAL;

/* make sure gfport/txport # is ok, and in use */
    if ((type = piecehdr.gr_type) == TXPORT) {
	if (piecehdr.gr_no >= NTXPORTS)
	    return EINVAL;
	tx = &txport[piecehdr.gr_no];
	tx->tx_state |= TX_BUSY;
	tx_textport(tx, piecehdr.gr_llx, piecehdr.gr_urx,
			piecehdr.gr_lly, piecehdr.gr_ury);
	pri = spl6();
	gr_freepieces(tx->tx_piecelist);
	tx->tx_piecelist = pp = gr_allocpieces(npieces);
	spl5();			/* block repaint but allow serial */
    } else if (type == GFPORT) {
	if ((piecehdr.gr_no >= NGFPORTS) || (!gfport[piecehdr.gr_no].gf_ic))
	    return EINVAL;
	gf = &gfport[piecehdr.gr_no];
	gf->gf_llx = piecehdr.gr_llx;
	gf->gf_lly = piecehdr.gr_lly;
	gf->gf_urx = piecehdr.gr_urx;
	gf->gf_ury = piecehdr.gr_ury;
	pri = spl6();
	gr_freepieces(gf->gf_piecelist);
	gf->gf_piecelist = pp = gr_allocpieces(npieces);
	spl5();			/* block repaint but allow serial */
    } else
	return EINVAL;

/* read in each piece, verifying it as we go */
    while (pp) {
	if (copyin(uaddr, (caddr_t)&piece, sizeof(piece))) {
	    error = EFAULT;
	    if (type == GFPORT) {
		gr_freepieces(gf->gf_piecelist);
		gf->gf_piecelist = 0;
	    } else {
		gr_freepieces(tx->tx_piecelist);
		tx->tx_piecelist = 0;
	    }
	    break;
	}
	uaddr += sizeof(piece);
	pp->p_xmin = piece.gr_xmin;
	pp->p_ymin = piece.gr_ymin;
	pp->p_xmax = piece.gr_xmax;
	pp->p_ymax = piece.gr_ymax;
	pp = pp->p_next;
    }

    if (type == GFPORT) {
	gf_copypieces(gf);		/* copy to shmem */
	if(gf->gf_ic->ic_doqueue & DQ_PIECECHANGE) 
	    gr_qenter(gf->gf_ic, PIECECHANGE, gf->gf_no);
	if (piecehdr.gr_doredraw) {	/* send redraw cookie */
	    if(gf->gf_ic->ic_doqueue & DQ_REDRAW)
		gr_qenter(gf->gf_ic, REDRAW, gf->gf_no);
	}
    } else {
	if(piecehdr.gr_doredraw)
	    tx->tx_state |= TX_SCROLLED | TX_REDISPLAY | TX_DRAWBORDER;
	tx->tx_state &= ~TX_BUSY;
    }
    splx(pri);
    return error;
}
