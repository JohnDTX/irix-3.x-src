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

/*************************** E M U L I N T . C ****************************
*
*   MODULE DESCRIPTION
*	This module contains emulation command processing code
*
*   ENTRY POINTS:
*	emulint() - processes commands from PCOX
*	getbyte() - fetch next byte from PCOX via kernal read
*	getby_nowait() - kernal read for next byte
*	getby_spl() - read second byte of Clear, Control Reg, Set Cur
*	kb() - send null to PCOX if time is up
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


/*
**	Externals
*/
extern		beep();
extern int	errno;
extern u_short	fid;
extern u_char	getspot();	/* get a char from the display buffer */
extern u_short	inkindex();	/* increment display cursor */
extern u_short	inkspot();	/* disp char and increment cursor */
extern void	longjmp();
extern px_status outb;
extern		setspot();	/* place a char on the display buffer */
extern u_char	Eprot;
extern char	File_xfer;
extern long	Old_time;
extern u_char	Pxd_debug;
extern u_char	*Rbuf[];

/*
**	Globals
*/

dma_buffer DMa = {0};

u_char
	Control_reg,
	Status_flags;
u_short
	Ack_flag,		/* Ack flag for direct buffer input */
	Cursor_addr = 80;

u_short	Prom_rel;		/* PROM release and level */
u_char	Prom_type;		/* PROM type */


/*  PROM serial number.  Seven bytes of serial number received
from nano processor are packed into 4 bytes.  The high order
four bits of the packed serial number are a flag using the
following codes:
	00		No identification sequence received from nano
	80		No serial number programmed into PROM
	C0		Transmission of serial number incomplete
	F0		Complete serial number transmitted
*/
u_char	Prom_serial[4];		/* PROM serial number */

/*
**	Local declarations
*/
u_char
	getbyte(), getby_nowait(), getby_spl();

char	begin1_row, exit_time = 0;

/*
**	Group 1 commands (C1 thru CF)
*/
int	cmd_incr(),cmd_reset(),cmd_write(),cmd_clear(),
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
int	cmd_dummy(),cmd_sethigh(),cmd_setlow(),cmd_dum1(),
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
	DP0("ZAP???-");
}
cmd_dum1()
{
	DP0("End_write-");
}

/*
**	Reset command processor
*/
cmd_reset()
{
	DP0("-c_RESET-");
	Control_reg = 0;		/* set control register to zero */
	Cursor_addr = 0x50;		/* initialize cursor address */
}

/*
**	Write Data command processor
*/
cmd_write()
{
	register u_char c;

	DP0("write-");
	if ((c = getbyte()) == 0x1b) { /* cent sign */
		c = getbyte();
		if (c == (u_char)0x90) {
			DP0("cQ-");
			send_ack();
		}
		if (c == (u_char)0x91) {
			DP0("cR-");
			Status_flags |= R_FLAG;
			if (!(Status_flags & W_FLAG)) {
				DP0("emu-");
				send_ack();
			}
		}
		if (c == (u_char)0x92) {
			printf("**CS-");
			Status_flags |= S_FLAG;
			if (!(Status_flags & W_FLAG)) {
				DP0("emu-");
				send_ack();
			}
		}
		if (c == (u_char)0x90 || c == (u_char)0x91 || c == (u_char)0x92) { /*q,r,s*/
#ifdef SQIRAL
			printf("!");
#endif SQIRAL
			while ((c = getbyte()) != (u_char)END_WRITE) {
#ifdef SQIRAL
				printf("!");
#endif SQIRAL
				if (iscommand(c))
					return;
			}
			return;
		} else {
			Cursor_addr = inkspot(Cursor_addr, 0x1b); /* cent */
#ifdef SQIRAL
			if(c == (u_char)DATA2RXFER)
				printf("cent-buck");
#endif SQIRAL
			if (iscommand(c))
				return;
			Cursor_addr = inkspot(Cursor_addr, c);
		}
	} else {
		if (iscommand(c))
			return;
		else
			Cursor_addr = inkspot(Cursor_addr, c);
	}
	strt_row();
	while ((c = getbyte()) != (u_char)END_WRITE) {
		if (iscommand(c))
			return;
		else
#ifdef SQIRAL
			if(c == (u_char)DATA2RXFER)
				printf("late-buck");
#endif SQIRAL
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

	DP0("clear to -");
	hibyte(new_cursor_addr) = getbyte();
	if (iscommand(hibyte(new_cursor_addr)))
		return;
	lobyte(new_cursor_addr) = getby_spl();
	DP1("%x-",new_cursor_addr);
	new_cursor_addr = min(new_cursor_addr, S_CHARS - 1);
	length = ((new_cursor_addr==0) ? S_CHARS : new_cursor_addr)
				- Cursor_addr;
	if (length < 0)
		length += S_CHARS - 80;
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

	DP0("insert-");
	c = getbyte();
	if (iscommand(c))
		return;
	strt_row();
	while ((old_c = getspot(Cursor_addr)) < (u_char)ATTR_IND) {
		setspot(Cursor_addr, c);
		c = old_c;
		if (old_c == 0)
			break;
		if ((Cursor_addr = inkindex(Cursor_addr)) == 0x50)
			break;
	}
	Cursor_addr++; end_row(); Cursor_addr--;
}

/*
**	Load Control Register command processor
*/
cmd_loadctl()
{
	DP0("ctl-");
	Control_reg = getby_spl();
	DP1("%x-",Control_reg);
}

/*
**	Alarm command processor
*/
cmd_alarm()
{
	DP0("alarm-");
	Status_flags |= A_FLAG;
}

/*
**	Set Cursor Address command processor
*/
cmd_cursor()
{
	u_short new_cursor;

	DP0("cursor-");
	hibyte(new_cursor) = getbyte();
	if (iscommand(hibyte(new_cursor)))
		return;
	lobyte(new_cursor) = getby_spl();
	DP1("%x-",new_cursor);
	Cursor_addr = min(new_cursor, S_CHARS - 1);
}

/*
**	Increment Cursor Address command processor
*/
cmd_incr()
{
	DP0("incr-");
	Cursor_addr = inkindex(Cursor_addr);
}

/*
**	Set Cursor Address High command processor
*/
cmd_sethigh()
{
	hibyte(Cursor_addr) = getby_spl();
	Cursor_addr = min(Cursor_addr, S_CHARS - 1);
}

/*
**	Set Cursor Address Low command processor
*/
cmd_setlow()
{
	DP0("csr-");
	lobyte(Cursor_addr) = getby_spl();
	DP1("%x-",Cursor_addr);
}

/*
**	Write entire buffer command processor
*/
cmd_wrbuffer()
{

	register u_char c;

	DP0("wrb-");
	Cursor_addr = 0;
	strt_row();
	while ((c=getbyte()) != (u_char)END_WRITE) {
		if (iscommand(c)) {
			DP0("cmd-");
			return;
		}
/*		Cursor_addr = inkspot(Cursor_addr, c);*/
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
	if (Prom_type & 0xff) {
		/* pack bits for zeroes PROM (*1*1*1*1) */
		for (bit_counter = 0; bit_counter < 4; bit_counter++) {
			c <<= 1;	/* shift a bit into upper nibble */
			c = (c & 0xff00) | (c << 1 & 0xff); /* drop a one */
		}
	} else {
		/* Pack bits for ones PROM (0*0*0*0*) */
		for (bit_counter = 0; bit_counter < 4; bit_counter++) {
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

	DP0("c_id-");
	hibyte(Prom_rel) = getbyte();		/* release number */
	lobyte(Prom_rel) = getbyte();		/* level number */
	promp = Prom_serial;
	*promp = (u_char)0xc0;			/* signal incomplete serial */
	for (byte_counter = 1; byte_counter <= 7; byte_counter++) {
		c = getbyte();
		if (byte_counter == 1) {
			if ((c & (u_char)0xc0) == (u_char)0x40) {/* are top 2 bits "01"?
							 */
				Prom_serial[0]= (u_char)0x80;	/* unprogrammed
							   serial */
			}
			Prom_type = c & (u_char)0xc0;		/* prom type
							   indicator */
		}
		if (byte_counter & 1)
			*promp |= pack_sn_bits((u_short)c);
		else
			*++promp = pack_sn_bits((u_short)c) << 4;
	}
	Prom_serial[0] |= (u_char)0xf0;			/* complete serial */
}

/*
**	ACK command processor
*/
cmd_ack()
{
	DP0("ack-");
	Ack_flag++;
}

/*
**	Extended command processor
*/
cmd_extended()
{
	DP0("NAK-");
	Ack_flag = 0x8000 | getbyte();	/* set high bit for NAK */
}
/*
 *   makes flow show all routines used
 */
cmd_flow()
{
	cmd_ack();
	cmd_alarm();
	cmd_clear();
	cmd_cursor();
	cmd_dummy();
	cmd_dum1();
	cmd_extended();
	cmd_identify();
	cmd_incr();
	cmd_insert();
	cmd_loadctl();
	cmd_reset();
	cmd_sethigh();
	cmd_setlow();
	cmd_wrbuffer();
	cmd_write();
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
	register char cmd_processed;
	register u_char c;
	
	for (cmd_processed = 0, exit_time = 2; --exit_time; ) {
		c = getby_nowait();
/*		kb();			/* output a null every second */
		if (outb.ftdone || errno)
			return(ERROR);
		if (c != (u_char)DMA_ZAP)
			if ((c & (u_char)CMD_MASK) == (u_char)CMD_PATTERN) {
				if (c <= (u_char)0xe0)
					(*group1_table[((c + 1) & 0xe)>>1])();
				else
					(*group2_table[((c + 1) & 0xe)>>1])();
				cmd_processed = 1;
				exit_time = 2;
			}
		else 			/* no PCOX char available */
			if (cmd_processed == 1)
				break;
		if (Status_flags & A_FLAG) {
			beep();
			Status_flags &= ~A_FLAG;
		}
		if (outb.ftdone)
			return(ERROR);
	}
	return (0);
}


/*
** getbyte gets next cmd byte from DMa buffer
*/
u_char
getbyte()
{
	register u_char c;

 	while (!errno && !outb.ftdone && (c =getby_nowait()) == (u_char)DMA_ZAP)
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
		DMa.d_cnt = pxdread((char *)DMa.d_buf,
		    sizeof(DMa.d_buf));
		if (!DMa.d_cnt)
			return (DMA_ZAP);
		DMa.d_rdp = DMa.d_buf;
		if (DMa.d_cnt == ERROR) {
			DMa.d_cnt = 0;
			return(DMA_ZAP);
		}
	}
	c = *DMa.d_rdp++;
	DMa.d_cnt--;
	return(c);
}



/*
**  Print emulint DMa buffer
*/
print_dma()
{
	register short col = 24;
	register u_char *p;

/*	for (p = DMa.d_buf; p < DMa.d_rdp; p++, col++) {
		(void)printf(" %02x", (u_char) *p);
		if (col%8 == 0)
			(void)printf(" ");
		if (col%16 == 0)
			(void)printf("\n   ");
	}*/
	messagef("Rbuf %x   DMa.d_buf %x outb %x\n\r",Rbuf,DMa.d_buf,&outb);
	for (p = DMa.d_buf; p < DMa.d_rdp; p++, col++) {
		if (col%8 == 0) {
			(void)messagef(" ");
		}
		if (col%24 == 0) {
			(void)messagef("\r\n ");
		}
		(void)messagef("%02x ",(u_char) *p);
	}
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

	if ((c = getbyte())!= (u_char)0xe0)
		return(c);
	return (getbyte() & (u_char)0xf | (u_char)0xe0);
}


strt_row()
{
	if (File_xfer)
		return;
	begin1_row = Cursor_addr / COLS;
}

end_row()
{
	if (File_xfer)
		return;
	repaint (begin1_row, (int)(Cursor_addr - 1) / COLS);
}
/*
 *   send ack to host
 */
send_ack()
{
	DP0("SEND_ack-");
	send_x_key(X_ENTER);
}

