/**************************************************************************
 *									  *
 * 		 Copyright (C) 1985, Silicon Graphics, Inc.		  *
 *									  *
 *  These coded instructions, statements, and computer programs  contain  *
 *  unpublished  proprietary  information of Silicon Graphics, Inc., and  *
 *  are protected by Federal copyright law.  They  may  not be disclosed  *
 *  to  third  parties  or copied or duplicated in any form, in whole or  *
 *  in part, without the prior written consent of Silicon Graphics, Inc.  *
 *									  *
 **************************************************************************/

/******************* D B G M E N U . C *******************************
*
* Manual mode and diagnostic menu for debugging 3274 interface
*
*********************************************************************/

#include <sys/types.h>
#include <gl.h>
#include <device.h>
#ifdef GL2TERM
#include <stdio.h>
#else
#include "Vioprotocl.h"
#endif GL2TERM
#include "term.h"
#include "pxw.h"

#define BUFSIZE	80

#ifdef DEBUG
#define CMD
#endif DEBUG

/*
**	Externals
*/
extern int	close();
extern		cmd_flow();
extern 		conclose();
extern caddr_t	asc_status();
extern caddr_t	ebx_status();
extern int	errno;
extern u_short	fid;
extern int	force_asc_str();
extern int	force_ebx_str();
extern outft	frcv;
extern void	free();
extern u_short	f13_diag();
extern u_short	f13_loop();
extern char	*getenv();
extern char	*gets();
extern 		messagef();
extern int	open();
extern px_status outb;
extern		print_dma();
extern px_bufs	pxl;
extern rglft	rrcv;
extern int	send_asc_str();
extern int	send_binary_str();
extern int	send_ebx_str();
extern		sscanf();
extern u_char	Asc2ebc[];
extern u_char	Display_xlat[];
extern dma_buffer DMa;
extern u_char	Eprot;
extern char	File_xfer;
extern char	Ft_type;
extern u_short	Prom_rel;
extern u_char	Pxd_debug;
extern char	Rows;
extern u_char	vgetc();



/*
**	Globals
*/

/*
**	Local variables
*/
u_char		ebuf[2*BUFSIZE];
getput		fst;
char		s[80];
u_char		textbin = 1;		/* 1-text, 0-bin */
/*
**	Turn on/off the tracing of this module
**	Dummy to keep lint happy, no tracing (DT) here
*/
tr_dbgm(flag)
{
	trace = flag;
}


/********************** P X D U S R *************************************
*
*  FUNCTION:
*	This is the main test program for the PDX device driver.
*
*  ENTRY:
*	-t turns on tracing to file pxlog
*
*  RETURNS:
*
*  EXTERNALS USED:
*
************************************************************************/


dbgmenu()
{
	u_char buf[BUFSIZE];	/* keyboard input buffer */
	u_char *p, *q;
	register u_char c, argcount;
	register short col, i;
	long n, arg1, arg2, retcnt;
	short data;
	Device button;
	static u_char clock[] = {
		 0x83,0x93,0x96,0x83,0x92,'\0'
	};
	static u_char clock6[] = {
	 0x5b,0x4a,0x83,0x93,0x96,0x83,0x92,0x40,0x40,0x40,0x40,0x40,
	 0x5b,0x4a,0x83,0x93,0x96,0x83,0x92,0x40,0x40,0x40,0x40,0x40,
	 0x5b,0x4a,0x83,0x93,0x96,0x83,0x92,0x40,0x40,0x40,0x40,0x40,
	 0x5b,0x4a,0x83,0x93,0x96,0x83,0x92,0x40,0x40,0x40,0x40,0x40,
	 0x5b,0x4a,0x83,0x93,0x96,0x83,0x92,0x40,0x40,0x40,0x40,0x40,
	 0x5b,0x4a,0x83,0x93,0x96,0x83,0x92,0x40,0x40,0x40,0x40,0x40,
	 0x5b,0x4a,0x83,0x93,0x96,0x83,0x92,0x40,0x40,0x40,0x40,0x40,
	 0x5b,0x4a,0x83,0x93,0x96,0x83,0x92,0x40,0x40,0x40,0x40,0x40,
	 0x5b,0x4a,0x83,0x93,0x96,0x83,0x92,0x40,0x40,0x40,0x40,0x40,
	 0x5b,0x4a,0x83,0x93,0x96,0x83,0x92,0x40,0x40,0x40,0x40,0x40,'\0'
	};
	static u_char tquit[] = {
		 0x98,0xa4,0x89,0xa3,'\0'
	};

	File_xfer = 0;
	Ft_type = FT_RGL;
	usropen();
	printmenu();
	for (;;) {
		messagef("\r\n->");
		argcount = 0;
		for (i=0;i++ < 10000; )
			;
		i = 0;
		while ((c = getc(stdin)) && c != 0x0d ) {
			buf[i++] = c;
			if (c == 8) {		/* backspace */
				buf[--i] = 0;	/* erase backspace */
				buf[--i] = 0;	/* erase prev char */
				messagef("%c %c",0x08,0x08);
			} else
				messagef("%c",c);
		}
		buf[i] = 0;
		messagef(" ");
		if (!i)
			break;
		n = 0;
		argcount = sscanf(buf,"%ld %lx %lx",&n,&arg1,&arg2);
		switch (n)
		{
		case 1:
			if (argcount != 1)
				break;
			printmenu();
			break;
		case 2:			/* plain 3279 terminal */
			if (argcount != 1)
				break;
#ifdef RGL
			switchtographics();
			lampoff(LAMP_KBDLOCKED);
#endif RGL
			context = 0;
			qoutb();
			if(retcnt = db_emulator());
				(void)messagef("\r\nreturned %d\r\n",retcnt);
#ifdef RGL
			context = GRAPHICS;
			tpon();
			switchtotext();
#endif RGL
			break;
		case 3:			/* RGL 3279 terminal */
			if (argcount != 1)
				break;
#ifdef RGL
			switchtographics();
			lampoff(LAMP_KBDLOCKED);
			context = TEXT;
#endif RGL
			qoutb();
			if(retcnt = db_emulator());
				(void)messagef("\r\nreturned %d\r\n",retcnt);
#ifdef RGL
			ginit();
			tpoff();
			cursoff();
			context = GRAPHICS;
			tpon();
			switchtotext();
#endif RGL
			context = 0;
			break;
		case 4:			/* download 3279 terminal */
			if (argcount != 1)
				break;
			switchtographics();
			lampoff(LAMP_KBDLOCKED);
			context = TEXT;
			qoutb();
			kill_outb();
			if(retcnt = db_emulator());
				(void)messagef("\r\nreturned %d\r\n",retcnt);
#ifdef RGL
			ginit();
			tpoff();
			cursoff();
			context = GRAPHICS;
			tpon();
			switchtotext();
#endif RGL
			context = 0;
			break;
		case 5:			/* PCOX diag test */
			if (update3270() >= 1)
				(void)message("#");
			argcount = sscanf(buf,"%ld %ld %lx",&n,&arg1,&arg2);
			if (arg1 == 1 || argcount != 2)
				i = f13_diag();
			else
				i = f13_loop(arg1);
			if (update3270() >= 1)
				(void)message("#");
			messagef("diag returned %d of 38",i);
		case 6:			/* PCOX static memory test */
			(void)test_dma_memory(&fst);
			if (fst.mbchar == 8)
				(void)message(" \r\n3274 i/f PCOX statics passed test\n");
			else
				(void)messagef("\r\n3274 i/f PCOX memory test returned %02x\n",
				(u_char)fst.mbchar);
			break;
		case 7:			/* toggle text/binary send */
			if (textbin) {
				textbin = 0;
				messagef(" to binary files");
			} else {
				textbin = 1;
				messagef(" to text files");
			}
			break;
		case 8:			/* send quit string */
			if (textbin) {
				if(retcnt = send_ebx_str(tquit,sizeof(tquit)-1))
					messagef("\r\nsend_ebx_string returned %d\n",retcnt);
			} else {
				if(retcnt = send_binary_str(tquit,sizeof(tquit)-1))
					messagef("\r\nsend_ebx_string returned %d\n",retcnt);
			}
			break;
		case 9:			/* send clock string */
			if (textbin) {
				if(retcnt = send_ebx_str(clock,sizeof(clock)-1))
					messagef("\r\nsend_ebx_string returned %d\n",retcnt);
			} else {
				if(retcnt = send_binary_str(clock,sizeof(clock)-1))
					messagef("\r\nsend_ebx_string returned %d\n",retcnt);
			}
			break;
		case 10:		/* send 6 text clocks */
			if (textbin) {
				if(retcnt = send_ebx_str(clock6,sizeof(clock6)-1))
					messagef("\r\nsend_ebx_string returned %d\n",retcnt);
			} else {
				if(retcnt = send_binary_str(clock6,sizeof(clock6)-1))
					messagef("\r\nsend_ebx_string returned %d\n",retcnt);
			}
			break;
		case 11:		/* toggle emulint cmd display */
#ifdef CMD
			if (Pxd_debug) {
#endif CMD
				message("no cmd display");
#ifdef CMD
				Pxd_debug = 0;
			} else {
				message("cmd display");
				Pxd_debug = 1;
			}
#endif CMD
			break;
		case 12:		/* force screen update */
			if (argcount != 1)
				break;
			get_screen();
			messagef(" Proms are rev %x",Prom_rel);
			break;
		case 13:		/* get 3270 cmds */
			argcount = sscanf(buf,"%ld %ld %lx",&n,&arg1,&arg2);
			if (!arg1)
				arg1 = 1;
			if (arg1 > 1000)
				arg1 = 1000;
			while (arg1--) {
				while (get3270() >= 1) {
					(void)message("#");
				}
			}
			break;
		case 14:		/* update 3270 for user */
			qoutb();
			argcount = sscanf(buf,"%ld %ld %lx",&n,&arg1,&arg2);
			if (!arg1)
				arg1 = 100;
			if (arg1 > 1000)
				arg1 = 1000;
			i = 1000;
			while (i-- && arg1) {
				if ((arg2 = update3270()) >= 1) {
					(void)message("#");
					arg1--;
					qoutb();
				} else if (arg2)	/*** DEBUG ***/
					messagef("r%x+",arg2);
			}
			break;
		case 15:		/* show screen */
			if (argcount != 1)
				break;
			delay(MS_100);
			repaint(1,24);
			show_screen();
			delay(10*MS_100);
			break;
		case 16:		/* show screen in hex */
			if (argcount != 1)
				break;
			show_hex_screen();
			break;
		case 17:		/* show DMa.d_buf */
			if (argcount != 1)
				break;
			print_dma();
			break;
		case 18:		/* show user message */
			messagef("\n\r");
			messagef("outb %x buf %x\n\r",&outb.wrp,pxl.dma_buf);
			for (p = pxl.dma_buf, col = 1; col < 216; p++, col++) {
				messagef("%02x-",(u_char) *p);
				if (col%8 == 0) {
					messagef(" ");
				}
				if (col%24 == 0) {
					messagef("\r\n");
				}
			}
			break;
		case 19:		/* pxd close/open */
			pxdclose();
			if (arg1)
				force_open_init();
			retcnt = pxdopen();
			if (retcnt != 38)
				messagef("\r\npxd open returned %d of 38\r\n",retcnt);
			File_xfer = 0;
			break;
		case 20:		/* show user memory */
			if (argcount = 3) {
				for (i=0; i<arg2; i++, arg1++) {
					if (i%8 == 0) {
						(void)messagef(" ");
					}
					if (i%24 == 0) {
						(void)messagef("\r\n ");
					}
					(void)messagef("%02x-",*(u_char *)arg1);
				}
			} else 
				(void)messagef("\007");
			break;
		case 21:		/* force kb_nano */
			if (argcount >= 2) {
				arg1 &= 0xff;
				(void)kb_nano(arg1);
			} else 
				(void)message("\007");
			break;
		case 22:		/* force signel */
			if (argcount >= 2) {
				arg1 &= 0xff;
				(void)force_signal(arg1);
			} else 
				(void)message("\007");
			break;
		case 23:		/* show kernel memory */
			if (argcount = 3) {
				fst.mbaddr = arg1;
				for (i=0; i<arg2; i++) {
					if (i%8 == 0) {
						messagef(" ");
					}
					if (i%24 == 0) {
						messagef("\r\n");
					}
					(void)fetch_byte(&fst);
					messagef("%02x-",(u_char)fst.mbchar);
					fst.mbaddr++;
				}
			} else 
				(void)messagef("\007");
			break;
		case 24:		/* store kernel byte */
			if (argcount = 3) {
				fst.mbaddr = arg1;
				fst.mbchar = arg2;
				(void)store_byte(&fst);
			} else 
				(void)messagef("\007");
			break;
		case 27:
			for (i = 1; i++ < 1000; ) {
				lampon(1);
				lampoff(1);
			}
			break;
		case 28:
			while(!(c = vgetc()))
				;
			messagef("c %x ",c);
			break;
		case 29:
			/* first accept typed ascii string */
			i = 0;
			while ((c = getc(stdin)) && c != 0x0d ) {
				buf[i++] = c;
				if (c == 8) {		/* backspace */
					buf[--i] = 0;	/* erase backspace */
					buf[--i] = 0;	/* erase prev char */
					messagef("%c %c",0x08,0x08);
				} else
					messagef("%c",c);
			}
			buf[i] = 0;
			buf[i+1] = 0;
			buf[i+2] = 0;
			printf(" ");
			if (!i)
				break;
			/* convert to ebcdic, then encode */
			for (n = 0; n < i; n++)
				buf[n] = Asc2ebc[buf[n]];
			/* 4 for 3 and print result */
			ebuf[0] = DATA1RXFER;
			ebuf[1] = DATA2RXFER;
			/*** LEN + 3 ENCODE HERE ***/
			n = i + 3;
			ebuf[9] = 0;
			ebuf[10] = 0;
			ebuf[11] = n;
			p = &ebuf[9];
			q = &ebuf[2];
			bin_encode(p,q);
			p = &buf[0];
			q += 4;
			for (n = 0; n < i; ) {
				bin_encode(p,q);
				p += 3;
				q += 4;
				n += 3;
			}
			*q++ = 0;
			i = q - &ebuf[0];
			for (n = 0; n < i; n++)
				ebuf[n] = Display_xlat[ebuf[n]];
			messagef(ebuf);
			break;
		case 99:	/* just to please lint */
			if(retcnt = send_asc_str(tquit,sizeof(tquit)-1));
				(void)messagef(" returned %d\n",retcnt);
			(void)cmd_flow();
			(void)asc_status();
			(void)ebx_status();
			(void)force_asc_str(tquit,sizeof(tquit)-1);
			(void)force_ebx_str(tquit,sizeof(tquit)-1);
			break;
		default:
			(void)messagef("\007");
		}
	}
	messagef("we are leaving????");
	usrclose();
}


/*
**	print menu
*/
printmenu()
{
	(void)message("\r\n	 1 - menu			 2 - terminal emulation");
	(void)message("\r\n	 3 - reserved			 4 - reserved ");
	(void)message("\r\n	 5 - PCOX diagnostic loop n#	 6 - test PCOX receive memory");
	(void)message("\r\n	 7 - toggle text/binary send	 8 - send quit");
	(void)message("\r\n	 9 - send clock 		10 - send 6 clocks ");
	(void)message("\r\n	11 - toggle 3270 cmd display	12 - force PCOX reset");
	(void)message("\r\n	13 - get 3270 cmds n#		14 - update 3270 buffer n#");
	(void)message("\r\n	15 - show screen 		16 - show screen hex");
	(void)message("\r\n	17 - show DMa.d_buf buffer	18 - show outb message hex");
#ifdef GL2    
	(void)message("\r\n	19 - PCOX close/open (f)	20 - show user (40000) nnnn bb");
#else
	(void)message("\r\n	19 - PCOX close/open (f)	20 - show user (28000) nnnn bb");
#endif GL2
	(void)message("\r\n	21 - force kb_nano (7eef) bb	22 - force signal (7eee) bb\r\n");
	message("\r\n PF 1-12 top 12 on island and CTRL 1234567890-= on keyboard");
	message("\r\n EXIT EMULATION is SET UP");
	message("\r\n ATTN CTRL-a          DEV CNCL CTRL-c      DUP CTRL-u       FM CTRL-f ");
	message("\r\n SYS REQ CTRL-r      CURSR SEL CTRL-s      TEST CTRL-t     EXIT is SET UP");
	message("\r\n BACK TAB is shift TAB    CENT is '['      INSERT is shift NO SCRL");
	message("\r\n CLEAR island 'ENTER'   ERASE EOF island '1'   ERASE INPUT island '2'");
	message("\r\n HOME island '.'  PA1 island CTRL-PF1      PA2 island CTRL-PF2");
}

/*
**	enable selected outb file receive
*/
qoutb()
{
	pxl.pxt_buf.bufp = pxl.dma_buf;
	rrcv.bodyaddr = (u_char *)pxl.dma_buf;
	rrcv.bodylen = (long)PXDMASIZ-3;
	(void)set_rglout_ptr(&rrcv);
}
