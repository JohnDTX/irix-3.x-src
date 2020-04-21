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

/**************************** T 3 2 7 9 . C *******************************
*
*	3279 terminal emulator through the CXI coax interface card
*
*********************************************************************/
#include "gl.h"
#include "Venviron.h"
#include "Vioprotocl.h"
#include "Vserial.h"
#include "chars.h"
#include "Vxns.h"
#include "rpc.h"
#include "term.h"
#include "hostio.h"
#include "grioctl.h"
#include <sys/types.h>
#include <errno.h>
#ifdef GL2
#include <stdio.h>
#endif GL2
#undef ERROR
#include "pxw.h"


extern int	close();
extern 		db_emulator();
extern 		emulator();
extern int	errno;
extern void	free();
extern char	*getenv();
extern char	*gets();
extern 		printf();
extern int 	pxdclose();
extern int	pxdopen();
extern int	repainter();
extern int	start, end;
extern char	File_xfer;
extern u_short	Prom_rel;




/*
**	Globals
*/

u_char		UDload = 1;		/* 1 for RGL */
u_char 		have3279 = 1;
#ifdef GL2
static char ident[] = "@(#) Terminal 3279 Version 1.1, terminal GL2, PM2";
#else
static char ident[] = "@(#) Terminal 3279 Version 1.1, terminal GL1, PM1";
#endif GL2

Process_id	whid;
Process_id	rhid;
Process_id 	dadypid;
Process_id 	rgraphhid;
Process_id 	rttyhid;
u_char		towrite = 0;
extern int	readhost(), writehost();

main()
{

	long i;
	      
	InitExceptionServer();
	tr_conio(0);
	tr_dbgm(0);
	tr_disp(0);
	tr_emulator(0);
	tr_emul(0);
	tr_fnct(0);
	tr_pxdio(0);
	tr_main(0);
	graphinited = 0;
	irisinit();
	Ready(Create(3, repainter, 500), 0);	/*** for graphics ***/
	color(4);
	cursoff();
	clear();
	initcom(I3270_COM);
	File_xfer = 1;
	dbgmenu();
	if (pxdclose()) {
		(void)messagef("Could not close '/dev/pxd':  errno = %d\n", errno);
		exit(1);
	}
	exit(0);
}

/*
**	Trace this module
**	Dummy to keep lint happy, no tracing (DT) present
*/
tr_main(flag)
{
	trace = flag;
}

spclose()
{
}


/*
**	readhost - interpret the character stream from the host 
**
*/
readhost()
{	
	register int onechar;
	register int scrtouched;

	scrtouched = 0;
	while(1) {
		while( --rc >= 0 )
			if((*rp & 0x7f) == TESC) {
				if (scrtouched) {
					flushscreen();
					scrtouched = 0;
				}
				rp++;
				doprimitive();
			} else {
				putscreenchar(*rp++);
				scrtouched = 1;
			}
		if(scrtouched) {
			flushscreen();
			scrtouched = 0;
		}
		onechar = fillhostbuffer();
		if((onechar & 0x7f) == TESC) {
			doprimitive();
		} 
		else {
			if ( onechar == -1 ) {
				break;
			}
			putscreenchar(onechar);
			scrtouched = 1;
		}
	}
}

/*
**	writehost - shuffle characters from the keyboard to the host
**		    
**
*/
writehost()
{

	register unsigned char c, cnext;

	while (1) {
		c =  getc(stdin);
		towrite = c;
	}
}
