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
#include <Vioprotocl.h>
#include "Vserial.h"
#include "chars.h"
#include "rpc.h"
#include "term.h"
#include "hostio.h"
#include "grioctl.h"
#include <sys/types.h>
#include <errno.h>
#include <Vio.h>
/*#include <Vdevtypes.h>*/
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
extern int start, end;
extern char	File_xfer;
extern u_short	Prom_rel;




/*
**	Globals
*/

u_char		UDload = 0;		/* 1 for upload */
#ifdef GL2TERM
static char ident[] = "@(#) Terminal 3279 Version 1.0, terminal GL2, PM2";
#else
static char ident[] = "@(#) Terminal 3279 Version 1.0, terminal GL1, PM1";
#endif GL2TERM



main()
{

	InitExceptionServer();
	tr_conio(0);
	tr_dbgm(0);
	tr_disp(0);
	tr_emulator(0);
	tr_emul(0);
	tr_fnct(0);
	tr_pxdio(0);
	tr_main(0);
	ginit();
	cursoff();
	color(7);
	ttyinit();
#ifdef GL2TERM
	pxdopen();
#else
	grunlock();
	if (pxdopen()<=0) {
		(void)printf("Cannot open '/dev/pxd': errno = %d\n", errno);
		exit(1);
	}
#endif GL2TERM
	File_xfer = 1;
	Ready(Create(3, repainter, 500), 0);	/*** for graphics ***/
	dbgmenu();
	if (pxdclose()) {
		(void)printf("Could not close '/dev/pxd':  errno = %d\n", errno);
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

dispatch()
{
}





