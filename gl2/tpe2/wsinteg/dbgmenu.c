/**************************************************************************
 *									  *
 * 		 Copyright (C) 1984,1985 Silicon Graphics, Inc.		  *
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
* Manual mode and diagnostic menu for debugging 3270 interface
*
*********************************************************************/

#include <sys/types.h>
#include <sys/stat.h>
#include <machine/cpureg.h>
#include <nlist.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include "term.h"
#include "hostio.h"
#include "gl.h"
#include "pxw.h"

#define BUFSIZE		1024


/*
**	Externals
*/
extern int		close();
extern 			conclose();
extern int		dflag[];
extern caddr_t		asc_status();
extern caddr_t		ebx_status();
extern int		fd;
extern long		force_asc_str();
extern long		force_ebx_str();
extern outft		frcv;
extern void		free();
extern u_short		f13_loop();
extern u_short		f13_diag();
extern long		get3270();
extern char		*getenv();
extern char		*gets();
extern char		*inputfile;
extern char		*malloc();
extern int		nlist();
extern int		open();
extern px_status	outb;
extern			printf();
extern long		pxdread();
extern px_bufs		pxl;
extern u_char		pxd_debug;
extern u_char		rbuf[];
extern rglft		rrcv;
extern long		send_asc_str();
extern long		send_ebx_str();
extern			send_big_file();
extern			sscanf();
extern u_char		ttytype;
extern long		update3270();
extern u_char		Asc2ebc[];
extern u_char		Buffer_is_mem;
extern long		Countr;
extern u_char		Display;
extern u_char		Display_xlat[];
extern dma_buffer	DMa;
extern u_char		Ebc2asc[];
extern u_char		F3174;
extern struct stat	Fstat;
extern long		Host_ack;
extern u_char		Keyboard;
extern u_char		Manuflag;
extern long		MaxRU;
extern long		Millibuzz;
extern u_char		Msg_proc;
extern u_char		Msgtype;
extern u_short		Prom_rel;
extern int		Pxd_dma_size;
extern u_char		Record_sep;
extern u_char		Rbuf[];
extern int		ttyd;
extern int		ttyp;

/*
**	Global variables
*/
FILE			*mlf = 0;
getput			Fst;
u_char			Blink_msgs = 0;


/*
**	Local variables
*/
int 			menu_display = 1;
int			menu0 = 1;
int			menu1 = 0;
int			menu2 = 0;
u_char			quitflag = 0;
u_char			buf[BUFSIZE];
int				i;
u_char			c; 
u_char			textbin[4] = { 0 };
struct stat		Fstat;
long 			HBUF = 0x40fc0000; /* Buffer as ram memory address */

u_char tclock[] = {
		 0x83,0x93,0x96,0x83,0x92,'\0'
	};
u_char tquit[] = {
		 0x98,0xa4,0x89,0xa3,'\0'
	};
struct nlist nl[3] = {
	{ "_Sbuf", },
	{ "_pxk", },
	{ NULL },
};

/*
**	Turn on/off the tracing of this module
**	Dummy to keep lint happy, no tracing (DT) here
*/
tr_dbgm(flag)
{
	trace = flag;
}

/********************** D B G M E N U . C ******************************
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
	u_char ebuf[2*BUFSIZ];
	u_char type = 'a', sgn = '+', *p, *q;
	register int col;
	int argcount, retcnt;
	long m, n, arg1, arg2;

	setbuf (stdout, (char *)0);
	for (;;) {
 	 	if (menu_display && menu0) 
			printmenu0();
 	 	if (menu_display && menu1) 
			printmenu1();
 	 	if (menu_display && menu2) 
			printmenu2();
		(void)printf("\n\n Enter Option: ");
		argcount = 0;
		errno = 0;
		fill_buf();
		n = 99;
		argcount = sscanf(buf,"%ld %lx %lx",&n,&arg1,&arg2);
		if (!menu_display) {
 	 		if (menu0 && n < 5) {
				printmenu0();
				printf("\n\n Enter Option: %1d\n",n);
			} 	 		
 	 		if (menu1 && n) {
				printmenu1();
				printf("\n\n Enter Option: %1d\n",n);
			}
 	 		if (menu2 && n) {
	    			printmenu2();
				printf("\n\n Enter Option: %1d\n",n);
			}
		}
		menu_display = 0;
		if (menu1)
			n = n + 10;
		if (menu2)
			n = n + 100;
		if (n == 10 || n == 100) {
			menu2 = menu1 = 0;
			menu0 = 1;
			menu_display = 1; 
			n = 99;
		} 
		if (n == 0) {
			printf(" 3270 emulation ended\n\n");
			break;
		}
		errno = 0;
		switch (n)
		{
		case 1:			/* terminal emulation */
			Msg_proc = MXFER;
			Msgtype = 0;
			qoutb();
			if(retcnt = db_emulator())
				return(retcnt);
			if (Display == 0)
				lampoff(0x0f);
			Blink_msgs = 0;
			menu_display = 1; /* show menu after this call   wpc */
			break;
		case 2:			/* file transfer */
			if (Display == 0)    /* before db_emulator call  wpc */
				lampoff(0x0f);                        /* wpc */
			Blink_msgs = 0;      /* before db_emulator call  wpc */
			Msg_proc = 0;
			Msgtype = 0;
			qoutb();
			if(retcnt = db_emulator())
				return(retcnt);
			menu_display = 1; /* show menu after this call   wpc */
			break;
		case 3:			/* file transfer, blink the lights */
			if (Display == 0)   /* before db_emulator call   wpc */
			        Blink_msgs = 1;                      /*  wpc */
			Msg_proc = 0;
			Msgtype = 0;
			qoutb();
			if(retcnt = db_emulator())
				return(retcnt);
			menu_display = 1; /* show menu after this call   wpc */
			break;
		case 4:		/* prepare textfile for  upload */
			printf("\n Prepare textfile for upload\n");  
			printf("\n Enter filename: ");
			fill_buf();
			argcount = sscanf(buf,"%ls",ebuf);
			if (!argcount) {
				menu_display = 1;
				break;
			}
			printf("\n Enter f if file is fortran\n");  
			printf("            or RETRN if not: ");  
			fill_buf();
			argcount = sscanf(buf,"%lc",&type);
			p = ebuf;
			printf(" ");
			prep_text(ebuf,type);
			break;
		case 5:		/* diagnostic and setup menu */
			menu0 = menu2 = 0;
			menu1 = 1; 
			menu_display = 1;
			break;
		case 6: 	/* advanced diagnostics menu */
			menu0 = menu1 = 0;
			menu2 = 1; 
			menu_display = 1;
			break;
		case 11:		/* set controller type in .t3279rc */
			if (F3174)
				printf("\n Controller type is 3174\n");
			else 
				printf("\n Controller type is 3274\n");
			printf("\n Enter 3174 or 3274 for controller type: ");
			fill_buf();
			argcount = sscanf(buf,"%ld",&arg1);
			if (arg1 == 3274) {
				F3174 = FALSE;
				set_3174();
				printf("\n Controller type is now 3274");
				printf("\n To save change in .t3279rc, go to Advanced Diagnostics Menu");
			} else if (arg1 == 3174) {
				F3174 = TRUE;
				set_3174();
				printf("\n Controller type is now 3174");
				printf("\n To save change in .t3279rc, go to Advanced Diagnostics Menu");
			} 
			break;
		case 12:	/* set host timeout */ 
			printf("\n Host timeout is %d seconds\n",Host_ack);
			printf("\n Enter Host timeout in seconds: ");
			arg1 = 0;
			fill_buf();
			argcount = sscanf(buf,"%ld",&arg1);
			if (arg1 < 5)
				break;
			Host_ack = arg1 > 900 ? 900 : arg1; /* 15 minute cap */
			printf("\n Host timeout is %d seconds",Host_ack);
			printf("\n To save change in .t3279rc, go to Advanced Diagnostics Menu");
			break;
		case 13:	/* set host RU size */
			printf("\n Host write RU is %d bytes\n",MaxRU);
			printf("\n Enter Host write RU in bytes: ");
 			arg1 = 0;
			fill_buf();
			argcount = sscanf(buf,"%ld",&arg1);
			if (arg1 < 256)
				break;
			if (F3174)
				MaxRU = arg1 & 0xffffff80;
			else
				MaxRU = arg1 & 0xfffffff0;
			if (F3174 && (MaxRU > PXINDSIZ-COLS))
				MaxRU = PXINDSIZ-COLS;
			arg1 = Buffer_is_mem ? 4*SMALL_DMA : SMALL_DMA;
			if (MaxRU > arg1)
				MaxRU = arg1;
			set_3174();
			printf("\n Host write RU is %d bytes ",MaxRU);
			printf("\n To save change in .t3279rc, go to Advanced Diagnostics Menu");
			break;
		case 14:		/* set Host_ack */
			printf("\n Record separator is %02x\n", Record_sep);
			printf("\n Enter new record separator as hex value: ");
			fill_buf();
			argcount = sscanf(buf,"%lx",&m);
			if (argcount == 1)
				Record_sep = (u_char)m;
			set_3174();
			printf("\n Record separator is now %02x",Record_sep);
			printf("\n To save change in .t3279rc, go to Advanced Diagnostics Menu");
			break;
		case 15:    /*   Reserved    wpc */
			break;
		case 16:			/* self test */
			printf("\n 3270 self-test diagnostics\n ");  
			printf("\n Enter number of times to repeat test: ");  
			fill_buf();
			argcount = sscanf(buf,"%ld",&arg1);
			if (update3270() >= 1)
				(void)printf("#");
			if (arg1 == 1) {
				if ((i = f13_diag()) < 38)
					printf(" 3270 self test failed rc = %d ",i);
				i = f14_diag();
			} else {
				f13_loop(arg1,1);
				i = f14_loop(arg1,1);
			}
			if (i < 38) {
				pdelay(50*MS_100);
				show_hex_screen();
			}
			if (update3270() >= 1)
				(void)printf("#");
			break;
		case 17:			/*  static memory test */
			printf("\n 3270 memory diagnostics\n ");  
			printf("\n Enter number of times to repeat test: ");  
			fill_buf();
			argcount = sscanf(buf,"%ld",&arg1);
			if (update3270() >= 1)
			if (!arg1)
				arg1 = 1;
			while (arg1--) {
				(void)test_dma_memory(&Fst);
				printf(" .");
			}
			if (Fst.mbchar == 8)
				printf("\n 3270 memory test passed ");
			else
				printf("\n 3270 memory test failed rc = %02x",
				(u_char)Fst.mbchar);
			break;
		case 18:			/* dma operation */
			printf("\n 3270 DMA diagnostics\n ");  
			printf("\n Enter number of times to repeat test: ");  
			fill_buf();
			argcount = sscanf(buf,"%ld",&arg1);
			while(arg1--)
				dma_test();
			printf("\n DMA testing complete");
			printf("\n Resetting coax interface");
			(void)force_open_init();
			(void)pxdclose();
			retcnt = pxdopen();
			if (retcnt < 38)
				printf(" pxd open failed rc = %d",retcnt);
			else
				printf("\n Coax interface reset");
			break;
		case  19:		/* log select flag */
			printf("\n Start Log\n ");  
			printf("\n Enter o for ASCII RGL log\n");  
			printf("       d for 3278 text log\n");  
			printf("       g for RGL command log\n");  
			printf("       r for RAW log\n");
			printf("\n Enter log type: ");
			fill_buf();
			argcount = sscanf(buf,"%lc",&type);
			printf(" ");
			if (type == 'o') {
				dflag[2] = 1;	/* ascii RGL logs */
				hostloginit();
				printf("\n opened WRDATA.1");
			} else if (type == 'd') {
				tr_disp(1);	/* log 3278 text */
				tr_init();
				printf("opened pxlog");
			} else if (type == 'g') {
				dflag[1] = 1;	/* RGL command log */
				tadelay(-1);
			} else if (type == 'q' || type == 'r') {
				logenable(2);
				printf("opened RAWINP");
			} else {
				printf("no log opened");
			}
			break;
		case  20:		/* log close */
			printf("\n End Log\n ");  
			printf("\n Enter o for ASCII RGL log\n");  
			printf("       d for 3278 text log\n");  
			printf("       g for RGL command log\n");  
			printf("       r for RAW log\n");
			printf("\n Enter log type: ");
			fill_buf();
			argcount = sscanf(buf,"%lc",&type);
			printf(" ");
			if (type == 'o') {
				dflag[2] = 0;	/* ascii RGL logs */
				hostloginit();
				printf("\n closed WRDATA.1");
			} else if (type == 'd') {
			} else if (type == 'd') {
				tr_disp(0);	/* log 3278 text */
				tr_close();
			} else if (type == 'g') {
				dflag[1] = 0;	/* RGL command log */
				tadelay(1);
			} else if (type == 'q' || type == 'r') {
				logenable(1);	/* RAWINP log */
				printf("closed RAWINP");
			} else {
				printf("no log closed");
			}
			break;
		case 21:		/* hex file dump  */
			printf("\n Display File in Hex\n");  
			printf("\n Enter filename to be displayed: ");
			fill_buf();
			argcount = sscanf(buf,"%ls",ebuf);
			printf("\n Enter o for ASCII RGL log\n");  
			printf("       d for 3278 text log\n");  
			printf("       g for RGL command log\n");  
			printf("       e for EBCDIC\n");
			printf("       r for RAW log\n");
			printf("       a for ASCII\n");
			printf("\n Enter file type: ");
			fill_buf();
			argcount = sscanf(buf,"%lc",&type);
			p = ebuf;
			if ((i = (stat(p, &Fstat))) < 0) {
				printf("\n File error rc = %d errno %d\n",
				retcnt,errno);
				break;
			}
			if ((retcnt = (open(p, O_RDONLY,"r"))) <= 0) {
				printf("\n File error rc =  %d errno %d\n",
				retcnt,errno);
				break;
			}
			m = 0;
			n = Fstat.st_size;
			printf("%s size hex %x dec %d\n",ebuf,n,n);
			type |= ' ';		/* to lower case */
			if (type != 'd' && type != 'g')
				printf("%06x  ",m);
			quitflag = 0;
			if (ttyp)
				rawmode(ttyp);
		while (quitflag == 0 && n > 0) {
			n = read(retcnt,ebuf,BUFSIZE);
			q = ebuf;
			if (type == 'd' || type == 'g') {
				for (col = 1, i=1; i <= n && !quitflag ; col++, i++) {
					c = *q++;
					printf("%c",c);
					if (c == '\n' && *q != '\r')
						printf("\r");
					if (ttyp) {
						if ((argcount = read(ttyp,textbin,1)) != 0) {
							if (textbin[0] != 0x11 
							&& textbin[0] != 0x13)
								quitflag = 1;
							else if (textbin[0] == 0x13) {
								while(!(argcount = read(ttyp,textbin,1)))
									;
							}
						}
					}
				}
			} else {
				for (col = 1, i=1; i <= n && !quitflag ; col++, i++) {
					c = *q;
					buf[col] = *q++;
					if (type == 'r') {
						if (c == CENT || c == (u_char)0xef) { /* cent was 0x1b now 0x45 lessen impact wpc */
							printf("\0339P%02x\0330@",c);
						} else
							printf("%02x",c);
						buf[col] = Display_xlat[c];
						if (c < 7 || buf[col] == 0x0a)
							buf[col] = ' ';
					} else if (type == 'o') { /* RGL */
						if (c == 0x10 || c == 0x7e) { /* RESC */
							printf("\0339P%02x\0330@",c);
						} else
							printf("%02x",c);
						buf[col] = c;
						if (c < ' ' || 0x7f <= (char)c)
							buf[col] = '.';
					} else if (type == 'e') { /* EBCDIC */
							printf("%02x",c);
							buf[col] = Ebc2asc[c];
							if (buf[col] <= ' ')
								buf[col] = ' ';
					} else {
						printf("%02x",c); /* ASCII */
						if (c < ' ' || 0x7f <= (char)c)
							buf[col] = '.';
					}
					if (col == 16) {
						printf("  ");
						for (col = 1; col < 17; col++)
							printf("%c",buf[col]);
						m += 16;
						printf("\r\n%06x  ",m);
						col = 0;
					} else if (col % 2 == 0) {
						printf(" ");
					}
					if (ttyp) {
						if ((argcount = read(ttyp,textbin,1)) != 0) {
							if (textbin[0] != 0x11 
							&& textbin[0] != 0x13)
								quitflag = 1;
							else if (textbin[0] == 0x13) {
								while(!(argcount = read(ttyp,textbin,1)))
									;
							}
						}
					}
				}
			}
			if (quitflag)
				break;
		}
			if (ttyp)
				restoremode(ttyp);
			menu_display = 1;
			break;
		case 101:		/* reset coax interface */
			printf("\n Resetting coax interface");
			(void)force_open_init();
			(void)pxdclose();
			retcnt = pxdopen();
			if (retcnt < 38)
				printf(" pxd open failed rc = %d",retcnt);
			else
				printf("\n Coax interface reset");
			break;
		case 102:		/* force kb_nano */
			printf("\n Send nano code to coax interface\n");
			printf("\n Enter nano code: ");
			fill_buf();
			argcount = sscanf(buf,"%lx",&arg1);
			if (argcount = 1) {
				arg1 &= 0xff;
				(void)send_pxd(arg1);
				printf("\n Nano code sent");
			} else 
				(void)printf("\007");
			break;
		case 103:		/* force signal */
			printf("\n Send signal code to coax interface\n");
			printf("\n Enter signal code: ");
			fill_buf();
			argcount = sscanf(buf,"%lx",&arg1);
			if (argcount = 1) {
				arg1 &= 0xff;
				(void)force_signal(arg1);
				printf("\n Signal code sent");
			} else 
				(void)printf("\007");
			break;
		case 104:		/* change key codes  */
			printf("\n This option allows: ");
			printf("\r\n\tChanging the type of Display and Keyboard ");
			printf("\r\n\tRedefining the keys on the keyboard ");
			printf("\r\n\tAltering the scan codes generated by the keyboard ");
			printf("\r\n\tSaving setup information in .t3279rc ");
			printf("\r\n\n Want to continue with this option ? (y/n): ");
		 	fill_buf();
			argcount = sscanf(buf,"%lc",&type);
			if (type == 'Y' || type == 'y') { 
				system("clear");   
				rdcon();
				menu_display = 1;
			}
			break;
		case 105:			/* send clock string */
			printf("\n Sending clock\n");
			arg1 = MaxRU;
			if(retcnt=send_ebx_str(tclock,sizeof(tclock)-1)) {
				printf("\n Send clock failed rc = %d\n",
				retcnt);
			} else
				printf("\n Clock sent");
			break;
		case 106:		/* send message string */
			printf("\n Send message string\n");
			arg1 = MaxRU;
			printf("\n Enter message to send: ");
			i = 0;
			while ((c = getc(stdin)) != 0x0a) {
				buf[i++] = c;
				if (c == 0x08)
					i--;
			}
			buf[i] = 0;
			Blink_msgs = 1;
			if(retcnt=send_asc_str(buf,i)) {
				printf("\n Send message failed rc = %d\n",
				retcnt);
			} else
				printf("\n Message sent");
			Blink_msgs = 0;
			break;
		case 107:		/* update 3270  */
			qoutb();
			arg1 = 1000;
			i = 1000;
			while (i-- && arg1) {
				Countr = 0;
				if (update3270() >= 1) {
					(void)printf("#");
					arg1--;
					qoutb();
				} else if (Countr > 0) {
					p = Rbuf;
					while (Countr--)
						printf("%02x-",*p++);
				}
			}
			break;
		case 108:		/* show screen in hex */
			system("clear");
			printf("\n Screen buffer Dump in hex\n");
			show_hex_screen();
			printf("\n\n Press RETRN to return to menu ");
			fill_buf(); 
			menu_display = 1;
			break;
		case 109:		/* show user message */
			printf("\n\routb %x buf %x length %x hex\n\r%x  ",
			&outb.wrp,outb.bufp,outb.length,outb.bufp);
			argcount = sscanf(buf,"%lx %lc",&n,&type);
			if (outb.length < 216)
				n = 216;
			else
				n = (outb.max_len > outb.length) ?
					outb.length : outb.max_len;
			p = outb.bufp;
			for (col = 1, i = 1; i < n; col++, i++, p++) {
				if (type == 'e') { /* EBCDIC */
					buf[col] = Ebc2asc[*p];
					if (buf[col] <= ' ')
						buf[col] = ' ';
				} else if (' '<= (char)*p && 0x7f > (char)*p) {
					buf[col] = *p;
				} else
					buf[col] = '.';
				printf("%02x",*p);
				if (col == 16) {
					printf("  ");
					for (col = 1; col < 17; col++)
						printf("%c",buf[col]);
					printf("\r\n%x  ",p+1);
					col = 0;
				} else
					printf("-");
				if (col == 8) {
					printf(" ");
				}
			}
			break;
		case 110:		/* show user memory */
			printf("\n Show user memory\n");
			printf("\n Rbuf begins at %x\n",Rbuf);
			printf("\n Enter beginning address to display: ");
			fill_buf();
			argcount = sscanf(buf,"%lx",&arg1);
			printf("\n Enter length to display (in hex): ");
			fill_buf();
			argcount = sscanf(buf,"%lx",&arg2);
			printf("\n Enter q for message highlighting\n");  
			printf("       e for EBCDIC\n");  
			printf("       or RETRN for default\n");  
			printf("\n Enter display type: ");
			fill_buf();
			argcount = sscanf(buf,"%lc",&type);
			system("clear");
			printf(" Show user memory\n");
			n = ++arg2;
			arg2--;
			printf("\r\n%x  ",arg1);
			for (col = 1, i=1; i < n; col++, i++) {
				c = *(u_char *)arg1;
				buf[col] = *(u_char *)arg1++;
				if (type == 'q') {
					if (c == CENT) { /* cent was 0x1b wpc */
						printf("\0339P%02x\0330@",c);
					} else
						printf("%02x",c);
					buf[col] = Display_xlat[c];
					if (c < 7)
						buf[col] = ' ';
				 } else if (type == 'e') {
						printf("%02x",c);
						buf[col] = Ebc2asc[c];
					} else {
						printf("%02x",c);
						if (c < ' ' || 0x7f <= (char)c)
							buf[col] = '.';
					}
				if (col == 16) {
					printf("  ");
					for (col = 1; col < 17; col++)
						printf("%c",buf[col]);
					printf("\r\n%x  ",arg1);
					col = 0;
				} else
					printf("-");
				if (col == 8) {
					printf(" ");
				}
			}
			printf("\n\n Press RETRN to return to menu ");
			fill_buf(); 
			menu_display = 1;
			break;
		case 111:		/* show kernel memory */
			printf("\n Show kernel memory\n");
			printf("\n kernel begins at %x",KERN_VBASE); 
                        printf("\n multibus I/O begins at %x",MBIO_VBASE);
			printf("\n 8237 dma begins at %x",MBIO_VBASE+0x7e00);
			printf("\n 3270 gate array begins at %x",MBIO_VBASE+0x7eee);
			printf("\n static RAM begins at %x\n",MBIO_VBASE+0xc000);
			nlist("/vmunix", nl);
			if (nl[0].n_type == 0) {
				fprintf(stderr, "%s: No IBM namelist\n",
				"/vmunix");
			} else {
				printf(" pxk rdp %x wrp %x Sbuf %x\n",
				nl[1].n_value, nl[1].n_value+4,nl[0].n_value);
				Fst.mbaddr = nl[1].n_value+8; /* pxk.bufp */
				HBUF = 0;
				for (col = 4; col ; ) {
					if (fetch_byte(&Fst)) {
						printf(" %x Bad", Fst.mbaddr);
						break;
					} else {
						HBUF |= (u_char)Fst.mbchar;
						Fst.mbaddr++;
						if (--col)
							HBUF <<= 8;
					}
				}
			}
			printf("\n Enter beginning address to display: ");
			fill_buf();
			argcount = sscanf(buf,"%lx",&arg1);
			printf("\n Enter length to display (in hex): ");
			fill_buf();
			argcount = sscanf(buf,"%lx",&arg2);
			system("clear");
			printf(" Show kernel memory\n");
			n = ++arg2;
			if (arg1 > 0) {
				if (arg2 < 1)
					arg2 = 10;
				Fst.mbaddr = arg1;
				if (arg1 < 0xffff)
					if (Buffer_is_mem == TRUE)
						Fst.mbaddr += HBUF;
					else
						Fst.mbaddr += MBIO_VBASE;
				printf("\r\n%x  ",Fst.mbaddr);
				arg2++;
				for (col = 1, i=1; i<arg2; col++, i++) {
					if (fetch_byte(&Fst)) {
						printf(" %x Bad", Fst.mbaddr);
						break;
					}
					buf[col] = (u_char)Fst.mbchar;
					printf("%02x",buf[col]);
					if (buf[col] < ' ' || 0x7f <= (char)buf[col])
						buf[col] = '.';
					if (col == 16) {
						printf("  ");
						for (col = 1; col < 17; col++)
							printf("%c",buf[col]);
						printf("\r\n%x  ",(Fst.mbaddr + 1));
						col = 0;
					} else
						printf("-");
					if (col == 8) {
						printf(" ");
					}
					Fst.mbaddr++;
				}
			}
			printf("\n\n Press RETRN to return to menu ");
			fill_buf(); 
			menu_display = 1;
			break;
		default:
			argcount = sscanf(buf,"%lc%lc",
						&sgn,&type);
			if (sgn == '~' && type == '!') {
				menu_display = 1;
				shell(stdin);
		 	} 
		}
	}
	return 0;
}
fill_buf()
{
	i = 0;
	while ((c = getc(stdin)) != 0x0a) {
		buf[i++] = c;
		}
	buf[i] = 0;
}
/* print menu 0, main menu */

printmenu0()
{
	system("clear"); 
	(void)printf("\n 3270 Emulation Menu");
	(void)printf("\n\n ~! - Escape to shell (key exit to return from shell)");
	(void)printf("\n  1 - Terminal Emulation");
	(void)printf("\n  2 - File Transfer");
	(void)printf("\n  3 - File Transfer (Blink the lights)");
	(void)printf("\n  4 - Prepare Textfile for Upload ");
	(void)printf("\n  5 - Setup and User Diagnostics Menu");
	(void)printf("\n  6 - Advanced Diagnostics Menu");
	(void)printf("\n\n  0 - Exit\n");
	print_keymap();
}

printmenu1()
{
	system("clear"); 
	(void)printf("\n 3270 Setup and Diagnostics Menu");
	(void)printf("\n\n  1 - Set controller model");
	(void)printf("\n  2 - Set host timeout value");
	(void)printf("\n  3 - Set RU size");
	(void)printf("\n  4 - Set record separator");
	(void)printf("\n  5 - Reserved for future use");
	(void)printf("\n\n  6 - Perform 3270 self test diagnostics");
	(void)printf("\n  7 - Perform 3270 memory diagnostics");
	(void)printf("\n  8 - Perform 3270 DMA diagnostics");
	(void)printf("\n\n  9 - Start Log");
	(void)printf("\n 10 - End Log");
	(void)printf("\n 11 - Display file in hex ");
	(void)printf("\n\n  0 - Return to main menu");
}

printmenu2()
{
	system("clear"); 
	(void)printf("\n 3270 Advanced Diagnostics Menu");
	(void)printf("\n\n  1 - Reset Coax interface");
	(void)printf("\n  2 - Send nano code to Coax interface");
	(void)printf("\n  3 - Send signal code to Coax interface");
	(void)printf("\n\n  4 - Change Display, keyboard, or save setup data in .t3279rc"); 
	(void)printf("\n\n  5 - Send clock");
	(void)printf("\n  6 - Send message string to host process");
	(void)printf("\n\n  7 - Update 3270 buffer");
	(void)printf("\n  8 - Show screen buffer in hex");
	(void)printf("\n  9 - Show outbound message in hex [e]");
	(void)printf("\n 10 - Show User memory");
	(void)printf("\n 11 - Show Kernel memory");
	(void)printf("\n\n  0 - Return to main menu");
}
/* dma_test */
dma_test()
{
#define SETMASK1	0x05
#define CLRMASK1	0x01
#define SETREQ1		0x05	/* force a dma request */
#define CLRREQ1		0x01	/* clear a dma request */
#define MODE1		0x55	/* single,incr,autoinit,write,1 */
#define MODE1B		0x81	/* block,incr,noautoinit,verify,1 */
struct pxdma {			/* 8237 DMA controller on PX */
	u_char	BADDR0;		/* 0 */
	u_char	BWRDCT0;	/* 1 PX card has correct addresses on mb */
	u_char	BADDR1;		/* 2 */
	u_char	BWRDCT1;	/* 3 */
	u_char	BADDR2;		/* 4 */
	u_char	BWRDCT2;	/* 5 */
	u_char	BADDR3;		/* 6 */
	u_char	BWRDCT3;	/* 7 */

	u_char	COMMAND; 	/* 8 read for status */
	u_char	REQUEST;	/* 9 */
	u_char	SINGLMASK;	/* a */
	u_char	MODE;		/* b */
	u_char	CLEARBYTE;	/* c */
	u_char	DMACLEAR;	/* d read for temporary register */
	u_char	CLEARMASK;	/* e */
	u_char	MASKWRITE;	/* f */
};
	struct pxdma *pxd_ioaddr;
	u_char c2;
	pxd_ioaddr = (struct pxdma *)(MBIO_VBASE + 0x7e00);

/* disable dma and read 8237 chan 1 addr reg */
	Fst.mbaddr = (long)&pxd_ioaddr->SINGLMASK;
	Fst.mbchar = SETMASK1;	/* disable DMA1 */
	(void)store_byte(&Fst);
/* put chan 1 in block write mode, read addr, count */
	Fst.mbaddr = (long)&pxd_ioaddr->MODE;
	Fst.mbchar = MODE1B;
	(void)store_byte(&Fst);
/* cause a request */
	Fst.mbaddr = (long)&pxd_ioaddr->SINGLMASK;
	Fst.mbchar = CLRMASK1;	/* enable DMA1 */
	(void)store_byte(&Fst);
	Fst.mbaddr = (long)&pxd_ioaddr->REQUEST;
	Fst.mbchar = SETREQ1;
	(void)store_byte(&Fst);
	Fst.mbaddr = (long)&pxd_ioaddr->REQUEST;
	Fst.mbchar = CLRREQ1;
	(void)store_byte(&Fst);
/* read chan 1 regs to verify programmability */
	Fst.mbaddr = (long)&pxd_ioaddr->SINGLMASK;
	Fst.mbchar = SETMASK1;	/* disable DMA1 */
	(void)store_byte(&Fst);
	Fst.mbaddr = (long)&pxd_ioaddr->MODE;
	Fst.mbchar = MODE1;
	(void)store_byte(&Fst);
	Fst.mbaddr = (long)&pxd_ioaddr->COMMAND;
	(void)fetch_byte(&Fst);
	c2 = Fst.mbchar;
	if (!(c2 & 2))
		printf("forced DREQ1 failed to change count reg1\n");
/* reprogram chan 1 */
	Fst.mbaddr = (long)&pxd_ioaddr->SINGLMASK;
	Fst.mbchar = CLRMASK1;	/* enable DMA1 */
	(void)store_byte(&Fst);
}

/*
**	enable selected outb file receive
*/
qoutb()
{
	rrcv.bodyaddr = (u_char *)pxl.dma_buf;
	rrcv.bodylen = (long)PXDMASIZ-3;
	(void)set_rglout_ptr(&rrcv);
}

/* delay 5 usec for px direct write */
 
dwdelay()
{
	register unsigned t;

#ifdef juniper
	for(t=13; t--;)
#else
	for(t=3; t--;)
#endif juniper
		;
}

