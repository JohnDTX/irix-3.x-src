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
 **************************************************************************

	********* INTERNAL USE ONLY ***********
	***** REQUIRES NON_DISCLOSURE AGREEMENT ********
	***** DO NOT SHOW TO ANYONE WITHOUT A NON_DISCLOSURE AGREEMENT ****


/****************** E M U L I N T . C *************************************
*
*   MODULE DESCRIPTION
*	This module contains emulation command processing code
*
*   ENTRY POINTS:
*	emulint() - processes commands from PCOX
*	getbyte() - fetch next byte from PCOX via kernel read
*	getby_nowait() - kernel read for next byte
*	getby_spl() - read second byte of Clear, Control Reg, Set Cur
*	print_dma() - print 'DMa' buffer in hex
*
*   GLOBAL VARIABLES
*	Ack_flag - host waits ack from outbound file xfer
*	Control_reg - shows keyboard lock state
*	Cursor_addr - Screen(3278) cursor value (80 is home)
*	Status_flags - for coordinating with emulator/update_3270
*
***********************************************************************/

#include <sys/types.h>
#include <errno.h>
#include "pxw.h"

#define ESPEC		(u_char)0xe0

/*
**	Externals
*/
extern			beep();
extern int		fd;		/* pxd device file descriptor */
extern u_char		getspot();	/* get a char from the display buffer */
extern u_short		inkindex();	/* increment display cursor */
extern u_short		inkspot();	/* disp char and increment cursor */
extern u_short		inkwbspot();	/* disp char and increment cursor */
extern px_status	outb;
extern long		pxdread();
extern int		replay[];
extern			setspot();	/* place a char on the display buffer */
extern u_char		Display_xlat[];
extern u_char		Msg_proc;
extern u_char		Noread;

/*
**	Globals
*/
u_char			Clear_flag, Control_reg, Status_flags;
u_char			Control_ctr;
u_char			Curhi_ctr;
u_char			Want_ack = 0;
u_short			Ack_flag,	/* Ack flag for direct buffer input */
			Cursor_addr = COLS;

u_short			Prom_rel;	/* PROM release and level */
u_char			Prom_type;	/* PROM type */

/*  PROM serial number.  Seven bytes of serial number received
from nano processor are packed into 4 bytes.  The high order
four bits of the packed serial number are a flag using the
following codes:
	00		No identification sequence received from nano
	80		No serial number programmed into PROM
	C0		Transmission of serial number incomplete
	F0		Complete serial number transmitted
*/
u_char			Prom_serial[4];	/* PROM serial number */

/*
**	Local declarations
*/

u_char
		getbyte(), getby_nowait(), getby_spl();

u_char		begin_row;
dma_opr		DMa = {0};
char		centwritten = 0, exit_time = 0;

/*
**	Group 1 commands (C1 thru CF)
*/
long	cmd_incr(),cmd_reset(),cmd_write(),cmd_clear(),
	cmd_insert(),cmd_loadctl(),cmd_alarm(),cmd_cursor();

fptr_t	group1_table[] = {
	cmd_incr,			/* CF increment cursor address */
	cmd_reset,			/* C2 reset */
	cmd_write,			/* C3 write data */
	cmd_clear,			/* C6 clear */
	cmd_insert,			/* C7 insert byte */
	cmd_loadctl,			/* CA load control register */
	cmd_alarm,			/* CB sound alarm */
	cmd_cursor			/* CE set cursor address */
};

/*
**	Group 2 commands (E1 thru EF)
*/
long	cmd_dummy(),cmd_sethigh(),cmd_setlow(),cmd_dum1(),
	cmd_wrbuffer(),cmd_identify(),cmd_ack(),cmd_extended();

fptr_t	group2_table[] = {
	cmd_dummy,			/* EF dma stop character */
	cmd_sethigh,			/* E2 set high cursor addr byte */
	cmd_setlow,			/* E3 set low cursor address byte */
	cmd_dum1,			/* E6 end write marker */
	cmd_wrbuffer,			/* E7 write entire buffer */
	cmd_identify,			/* EA release and serial info */
	cmd_ack,			/* EB ack for direct buffer input */
	cmd_extended			/* EE extended comands */
};

/*
**	Turn on/off the tracing of this module
**      Dummy to keep lint happy, no tracing (DT) present
*/
tr_emul(flag)
{
	trace = flag;
}

/*
**	Dummy routine for invalid commands
*/
cmd_dummy()
{
#ifdef DEBUG
	DT("ZAP?-");
#endif /* DEBUG */
}
cmd_dum1()
{
#ifdef DEBUG
	DT("ew-");
#endif /* DEBUG */
}

/*
**	Reset command processor
*/
cmd_reset()
{
#ifdef DEBUG
	DT("-c_RESET-");
#endif /* DEBUG */
	Control_reg = 0;		/* set control register to zero */
	Cursor_addr = COLS;		/* initialize cursor address */
}

/*
**	Write Data command processor
*/
cmd_write()
{
	register u_char c;

#ifdef DEBUG
	DT("w-");
#endif /* DEBUG */
	c = getbyte();
	if (centwritten == 2 || c == CENT) {	/* cent,r split by 3274 */
		if (!centwritten || c == CENT)	/* c3,1b */
			c = getbyte();
		centwritten = 0;
		if (c == (u_char)0x90) {
#ifdef DEBUG
			DT("cQ-");
#endif /* DEBUG */
			Want_ack = 1;
		} else if (c == (u_char)0x91) {
#ifdef DEBUG
			DT("cR-");
#endif /* DEBUG */
			Status_flags |= R_FLAG;
		} else if (c == (u_char)0x92) {
#ifdef DEBUG
			DT("**CS-");
#endif /* DEBUG */
			Status_flags |= S_FLAG;
		} else if (c == KILLIT) {
#ifdef DEBUG
			DT("**CK-");
#endif /* DEBUG */
			got_killed();
		}
		if (c == (u_char)0x90 || c == (u_char)0x91 || c == (u_char)0x92
		    || (Msg_proc!=RGLXFER && (c == KILLIT))) { /*q,r,s,k*/
/*			while ((c = getbyte()) != END_WRITE) {
				if (iscommand(c))
					return;
			}*/
			for (;c != END_WRITE && DMa.d_cnt > 0;)
				c = getbyte();
			if (Status_flags & W_FLAG)
				if (Status_flags & (R_FLAG | S_FLAG))
					outb.ftdone = EACKNAK;
			exit_time = 0;
			return;
		} else {		/* cent ? or centwritten ? */
			Cursor_addr = inkspot(Cursor_addr, CENT); /* cent */
			if (iscommand(c)) {
				if (c == END_WRITE)	/* c3,1b,e6 */
					centwritten = 1;
				return;
			}
			Cursor_addr = inkspot(Cursor_addr, c);
		}
	} else {
		if (iscommand(c))
			return;
		else {
			centwritten = 0;
			Cursor_addr = inkspot(Cursor_addr, c);
		}
	}
	strt_row();
	while ((c = getbyte()) != END_WRITE) {
		if (iscommand(c))
			return;
		else
			Cursor_addr = inkspot(Cursor_addr, c);
	}
	end_row();
}

/*
**	Clear from Cursor to End of Screen command processor 
*/
cmd_clear()
{
	u_short new_cursor_addr;
	register short length;

#ifdef DEBUG
	DT("clr-");
#endif /* DEBUG */
	hibyte(new_cursor_addr) = getbyte();
	if (iscommand(hibyte(new_cursor_addr)))
		return;
	lobyte(new_cursor_addr) = getby_spl();
#ifdef DEBUG
	DT("%x-",new_cursor_addr);
#endif /* DEBUG */
	Clear_flag++;
	new_cursor_addr = new_cursor_addr == 0 ? PXBUFSIZ :
		min(new_cursor_addr, PXBUFSIZ);
	if (Cursor_addr >= PXBUFSIZ)
		Cursor_addr = PXBUFSIZ;
	length = new_cursor_addr > Cursor_addr ?
		new_cursor_addr - Cursor_addr : PXBUFSIZ - Cursor_addr;
	strt_row();
	while (length--)
		Cursor_addr = inkspot(Cursor_addr, 0);
	end_row();
	Cursor_addr = new_cursor_addr;
}

/*
**	Insert Byte command processing routine
*/
cmd_insert()
{
	register u_char c, old_c;

#ifdef DEBUG
	DT("ins-");
#endif /* DEBUG */
	c = getbyte();
	if (iscommand(c))
		return;
	strt_row();
	while ((old_c = getspot(Cursor_addr)) < ATTR_IND) {
		setspot(Cursor_addr, c);
		c = old_c;
		if (old_c == 0)
			break;
		if ((Cursor_addr = inkindex(Cursor_addr)) == COLS)
			break;
	}
	Cursor_addr++; end_row(); Cursor_addr--;
}

/*
**	Load Control Register command processor
**	1 = blink cursor, 2 = reverse cursor, 4 = inhibit cursor
**	8 = inhibit display, 10 = inhibit step
*/
cmd_loadctl()
{
#ifdef DEBUG
	DT("ctl-");
#endif /* DEBUG */
	Control_reg = getby_spl();
	Control_ctr++;
#ifdef DEBUG
	DT("%x-",Control_reg);
#endif /* DEBUG */
}

/*
**	Alarm command processor
*/
cmd_alarm()
{
#ifdef DEBUG
	DT("bel-");
#endif /* DEBUG */
	Status_flags |= A_FLAG;
	got_killed();
}

/*
**	Set Cursor Address command processor
*/
cmd_cursor()
{
	u_short new_cursor;

#ifdef DEBUG
	DT("cur-");
#endif /* DEBUG */
	hibyte(new_cursor) = getbyte();
	if (iscommand(hibyte(new_cursor)))
		return;
	lobyte(new_cursor) = getby_spl();
#ifdef DEBUG
	DT("%x-",new_cursor);
#endif /* DEBUG */
	new_cursor &= (PXBUFSIZ - 1);	/* 11 bit mask */
	Cursor_addr = min(new_cursor, S_CHARS - 1);
}

/*
**	Increment Cursor Address command processor
*/
cmd_incr()
{
#ifdef DEBUG
	DT("inc-");
#endif /* DEBUG */
	Cursor_addr = inkindex(Cursor_addr);
}

/*
**	Set Cursor Address High command processor
*/
cmd_sethigh()
{
	register u_char c;
	
	c = getby_spl();
	hibyte(Cursor_addr) = c;
	if (c)
		Curhi_ctr++;
	Cursor_addr &= (PXBUFSIZ - 1);	/* 11 bit mask */
	Cursor_addr = min(Cursor_addr, S_CHARS - 1);
}

/*
**	Set Cursor Address Low command processor
*/
cmd_setlow()
{
#ifdef DEBUG
	DT("csr-");
#endif /* DEBUG */
	lobyte(Cursor_addr) = getby_spl();
	Cursor_addr &= (PXBUFSIZ - 1);	/* 11 bit mask */
#ifdef DEBUG
	DT("%x-",Cursor_addr);
#endif /* DEBUG */
	if (centwritten == 1)
		centwritten = 2;
}

/*
**	Write entire buffer command processor
*/
cmd_wrbuffer()
{

	register u_char c;

#ifdef DEBUG
	DT("wrb-");
#endif /* DEBUG */
	Cursor_addr = 0;
	strt_row();
	while ((c = getbyte()) != END_WRITE) {
		if (iscommand(c)) {
#ifdef DEBUG
			DT("cmd-");
#endif /* DEBUG */
			return;
		}
		Cursor_addr = inkwbspot(Cursor_addr, c);
	}
	end_row();
}

/*
** Subroutine to pack PROM serial number bits.
**  Packing algorithm depends on PROM type.
**  PROM which is initially in zero state is indicated by "01"
**  in top 2 bits.  Bits 1,3,5, and 7 are packed.  PROM which
**  is initially in ones state is indicated by "00" in top
**  2 bits.  Bits 0,2,4, and 6 are packed.
*/
u_char
pack_sn_bits(c)
u_short c;
{
	register char bit_counter;

	c &= 0xff;			/* clear upper nibble */
	if (Prom_type & (u_char)0xff) {
		/* pack bits for zeroes PROM (*1*1*1*1) */
		for (bit_counter=0; bit_counter<4; bit_counter++) {
			c <<= 1;	/* shift a bit into upper nibble */
			c = (c & 0xff00) | (c << 1 & 0xff); /* drop a one */
		}
	} else {
		/* Pack bits for ones PROM (0*0*0*0*) */
		for (bit_counter=0; bit_counter<4; bit_counter++) {
			c = (c & 0xff00) | (c << 1 & 0xff); /* drop a zero */
			c <<= 1;	/* shift a bit into upper nibble */
		}
	}
	return (c >> 4);
}

/*
**	"Identify" command processor
**	  Stores PROM release/level and serial number information
*/
cmd_identify()
{
	register u_char c, *promp;
	register char byte_counter;

#ifdef DEBUG
	DT("id-");
#endif /* DEBUG */
	hibyte(Prom_rel) = getbyte();		/* release number */
	lobyte(Prom_rel) = getbyte();		/* level number */
	if (hibyte(Prom_rel) > 1 || lobyte(Prom_rel) > 3)
		return;
	promp = Prom_serial;
	*promp = (u_char)0xc0;			/* signal incomplete serial */
	for (byte_counter = 1; byte_counter <= 7; byte_counter++) {
		c = getbyte();
		if (byte_counter == 1) {
			if ((c & (u_char)0xc0) == 0x40) {	/* are top 2 bits "01"?
							 */
				Prom_serial[0]= (u_char)0x80;	/* mark as unprogrammed
							   serial */
			}
			Prom_type = c & (u_char)0xc0;	/* save as prom type
							   indicator */
		}
		if (byte_counter & 1)
			*promp |= pack_sn_bits((u_short)c);
		else
			*++promp = pack_sn_bits((u_short)c) << 4;
	}
	Prom_serial[0] |= (u_char)0xf0;		/* mark as complete serial */
}

/*
**	ACK command processor
*/
cmd_ack()
{
#ifdef DEBUG
	DT("A");
#endif /* DEBUG */
	Ack_flag++;
}

/*
**	Extended command processor
*/
cmd_extended()
{
#ifdef DEBUG
	DT("NAK-");
#endif /* DEBUG */
	Ack_flag = 0x8000 | getbyte();	/* set high bit for NAK */
}


/********************** E M U L I N T ***********************************
*
*  FUNCTION:
*	Main reader routine from the PCOX card
*
*  ENTRY:
*
*  RETURNS:
*	 0 - terminal panel(s) received
*	 ERROR - file panel received
*
*  EXTERNALS USED:
*
************************************************************************/

emulint()
{
	register u_char cmd_processed;
	register u_char c;
	
	for (cmd_processed = 0, exit_time = 2; --exit_time > 0; ) {
		c = getby_nowait();
		if (outb.ftdone || errno) {
			return(ERROR);
		}
		if (c != DMA_ZAP)
			if ((c & CMD_MASK) == CMD_PATTERN) {
				if (c <= ESPEC)
					(*group1_table[((c + 1) & 0xe)>>1])();
				else
					(*group2_table[((c + 1) & 0xe)>>1])();
				cmd_processed = 1;
				if (exit_time) {
#ifdef DEBUG
					if (replay[1])
						exit_time = 1;
					else
#endif /* DEBUG */
						exit_time = 2;
				}
			}
		else			/* no PCOX char available */
			if (cmd_processed != 0)
				break;
		if (Status_flags & A_FLAG) {
			beep();
			Status_flags &= ~A_FLAG;
		}
		if (outb.ftdone) { 
			return(ERROR);
		}
	}
	if (Want_ack)
		return(-2);
	else
		return 0;
}


end_row()
{
	if (Msg_proc <= MXFER)
		repaint ((char)begin_row, (char)((Cursor_addr - 1) / COLS));
}


/*
** getbyte gets next cmd byte from DMa buffer
*/
u_char
getbyte()
{
	register u_char c;

	c = DMA_ZAP;
 	while (!errno && !outb.ftdone && (c = getby_nowait()) == DMA_ZAP)
		;
	return (c);
}


/*
** getby_nowait gets next cmd byte from DMa buffer
*/
u_char
getby_nowait()
{
	register u_char c;

	if (!DMa.d_cnt) {
		if (Noread)
			return (DMA_ZAP);
		DMa.d_cnt = pxdread();
		if (DMa.d_cnt <= 0) {
			DMa.d_cnt = 0;
			return(DMA_ZAP);
		} 
#ifdef DEBUG
		DT("*%x ",DMa.d_cnt);
#endif /* DEBUG */
	}
	c = *DMa.d_rdp++;
	DMa.d_cnt--;
	return(c);
}


/*
** Subroutine to get next command byte in special cases
** where px may have substituted an escape sequence.
** This applies to Set Cursor, Clear,Set High and Set Low
** Address Counter, and Load Control Register commands.
** 0xef is sent as 0xe0, 0xff; and the byte 0xe0 is sent as
** 0xe0, 0x00.
*/
u_char
getby_spl()
{
	register u_char c;

	if ((c = getbyte())!= ESPEC)
		return(c);
	return (getbyte() & (u_char)0xf | ESPEC);
}


strt_row()
{
	if (Msg_proc <= MXFER)
		begin_row = Cursor_addr / COLS;
}

