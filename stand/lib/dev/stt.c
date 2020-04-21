/*
 * $Source: /d2/3.7/src/stand/lib/dev/RCS/stt.c,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:14:52 $
 */
/*
** stt.c	- Copyright (C) JCS Computer Services - Sunnyvale CA 94089
**		- chase bailey - December 1984
**		- Any use, copy or alteration is strictly prohibited
**		- unless authorized in writing by JCS Computer Services.
**
**	Started on 12/28/84
*/

#define KERNEL 

#include "stand.h"
#include "mbenv.h"
#include "sys/types.h"
#include "sys/dir.h"
#include "sys/iobuf.h"
#include "sys/dkerror.h"
#include "sys/sysmacros.h"
#include "dklabel.h"
#include "sttreg.h"
#include "cpureg.h"
#include "ctype.h"
#include "dprintf.h"

extern struct dkerror sterrs[];

struct dkerror sttcmds[] = {
	{ 0xA0,	"retention the tape" },
	{ 0xA1, "read tape" },
	{ 0xA2, "write tape" },
	{ 0xA3, "verify tape" },
	{ 0xA4, "erase tape" },
	{ 0xA5, "write file mark" },
	{ 0xA6, "report tape status" },
	{ 0xA7, "configure tape" },
	{ 0xA9, "rewind tape" },
	{ 0xAA, "read file marks" },
	{ 0xAB, "seek tape blocks" },
	{ 0xAC, "command pass/tape drive" },
	{ 0,	0 },
};

#undef ST_DEBUG

#define STIOADDR	0x7054
#define STIOADDR2	0x73fc
#define NST		1
#define NSTT		1

int	sttspinbusy = 150;

struct	mb_device *sttdinfo[NST];
struct	mb_ctlr *sttcinfo[NSTT];

int stt_inrewind;
char *sttbuffer;
char *ststat_mbva;

stopen(io, ext, file)
register struct iob *io;
register char *ext;
register char *file;
{
	register struct inode *ip = io->i_ip;
	register short unit = UNIT(ip->i_dev);
	register struct softc_tape *st;
	register short device = DEVICE(ip->i_dev);
	register flag = io->i_flgs;

	if (!(sttsoftc.sc_flags & SC_INITED)) {
		if ( sttprobe( STIOADDR, STIOADDR2 ) != CONF_ALIVE )  {
			io->i_error = ENODEV;
			return(-1);
		}

		if (sttinit(unit)) {
			dprintf(("st%d: failure in config\n", unit));
			io->i_error = ENODEV;
			return(-1);
		}
	}
	st = &sttsoftc.sc_tape[unit];
	if (st->sc_flags & SC_TAPEINUSE) {
		dprintf(("st%d: tape is in use\n", unit));
		io->i_error = EBUSY;
		return(-1);
	}
	if ((unit > MAX_TAPES) || !(sttsoftc.sc_flags & SC_ALIVE)) {
		io->i_error = ENODEV;
		return(-1);
	}
	/* Read the status */
	if (sttstatus(unit))
		goto exit;
	if (st->sc_status0 & NOCARTRIDGE) {
		printf("st%d: no tape cartridge\n", unit);
exit:
		io->i_error = EIO;
		return(-1);
	}
	if ((st->sc_status0 & WRITEPROTECTED) && (flag & F_WRITE)) {
		dprintf(("st%d: write protected\n", unit));
		goto exit;
	}
	if (st->sc_status0 & NOTONLINE) {
		dprintf(("st%d: not on line\n", unit));
		goto exit;
	}
	if (!(st->sc_status1&BOT) && !(device & NOREWIND)) {
		if (sttrewind(unit))
			goto exit;
		st->sc_fileno = 0;
		if (sttstatus(unit))
			goto exit;
	}
	st->sc_blkno = 0;
	st->sc_nxrec = (daddr_t)2000000000;		/* INFINITY */
	return(0);
}

stclose(io)
register struct iob *io;
{
	register struct inode *ip = io->i_ip;
	register short unit = UNIT(ip->i_dev);

	sttrewind(unit);
}

sttrewind(unit)
{
	register timeout = 10000000;

	dprintf(("sttrewind of unit 0x%x ",unit));

	if ( simplecmd(unit, C_TPREWIND, 1) ) {
		return(1);
	}

	delay_ms(100);
	while ( (!(TPREADY() & (1<<unit))) && --timeout )
		;

	dprintf(("timeout left %d, st2 0x%x\n",timeout,TPREADY()));
	if ( timeout == 0 ) {
		return(1);
	} else {
		return(0);
	}
}

stioctl()
{
}

/*
** ststrategy(bp) - Called to do the first checking on the disk request and
*/
ststrategy(io, flag)
register struct iob *io;
int flag;
{
	register struct buf *bp = io->i_bp;
	register iopb_t *iop = sttsoftc.sc_iopb;
	register short unit = UNIT(bp->b_dev);
	register struct softc_tape *st = &sttsoftc.sc_tape[unit];
	register u_long addr;

	switch ( flag ) {

	case WRITE:
		bp->b_error = EIO;
		break;

	case READ:
		iop->i_unit = unit + TAPEUNITOFFSET;
		/* Buffer address */
		addr = (u_long) mbvtop(bp->b_iobase);
		iop->i_bufl = LB(addr);
		iop->i_bufm = MB(addr);
		iop->i_bufh = HB(addr);
		/* Number of bytes to transfer */
		addr = ((bp->b_bcount + BLKSIZE - 1) >> BLKSHIFT);
		iop->i_scch = MB(addr);
		iop->i_sccl = LB(addr);
		iop->i_filecountl = 0;
		iop->i_filecounth = 0;

    		/* start command up */
		if (sttcmd( C_TPREAD, 1, unit)) {
			dprintf(("st%d: can't start command\n", unit));
			bp->b_error = EIO;
			goto done;
		}
		break;
	}


done:
	if ( bp->b_error ) {
		bp->b_resid = bp->b_bcount - 0;		/* XXXX */
		return(-1);
	} else
		return(0);
}

/*
** sttprobe:
**	- probe the controller, and see if its there
**	- initialize it, to get things going
**	- remember that the port address has to be byte swapped 
*/
sttprobe(reg,reg2)
int reg, reg2;
{
	register int i;

	sttsoftc.sc_flags = 0;
	sttsoftc.sc_ioaddr = (caddr_t)MBIO_VBASE + reg;
	DELAY(20000);
	if (  !probe(STR0) ) {
		sttsoftc.sc_ioaddr = (caddr_t)MBIO_VBASE + reg2;
		if (  !probe(STR0) )
			return CONF_DEAD;
	}
	/* clear board */
	RESET();
	DELAY(50000);
	CLEAR();
	DELAY(50000);
	sttsoftc.sc_flags = SC_ALIVE;
	return CONF_ALIVE;
}

sttcmd(cmd, waiting, unit)
	int cmd, waiting, unit;
{
	register iopb_t *iop = sttsoftc.sc_iopb;
	register long timeout;
	register long s, i;

	dprintf(("sttcmd(%d) %s\n", cmd, dkerror((u_char)cmd, sttcmds) ));
	s = spl6();
	timeout = 200000;
	while (((i = STATUSREG()) & ST_BUSY) && --timeout)
		;
	if(timeout == 0) {
		dprintf(("st%d: timeout idle controller status %x\n", unit, i));
		return 1;
	}
	iop->i_cmd = cmd;
	iop->i_status = 0;
	iop->i_error = 0;
	if (waiting == 0) {
		START();			/* Start the command */
		splx(s);
		return 0;
	}

	DELAY(500);
	STARTWO();
	/* Wait for status */
	timeout = 4000000;
	for(;;) {
		if(iop->i_status == S_OK) {
			CLEAR();
			DELAY(500);
			splx(s);
			return 0;
		}
		if(iop->i_status == S_ERROR) {
		    dprintf(("[st%d: hard error(%x) cmd: %s, error: %s]\n",
				 iop->i_unit, iop->i_error,
				 dkerror((u_char)iop->i_cmd, sttcmds),
				 dkerror((u_char)iop->i_error, sterrs)));
			CLEAR();
			DELAY(500);
			splx(s);
			return 1;
		}
		if((--timeout) == 0) {
			dprintf(("[st%d: timeout cmd: %s, error: %s, status %x] ",
				unit,
				dkerror((u_char)iop->i_cmd, sttcmds),
				dkerror((u_char)iop->i_error, sterrs),
				iop->i_status));
			CLEAR();
			DELAY(500);
			splx(s);
			return 1;
		}
	}
}

/*
** sttinit() -- unit type and mode (check to see if inited)
*/
sttinit(unit)
	int unit;
{
	register iopb_t *iop;
	char *mbmalloc();

	if ( sttbuffer == 0 ) 
		sttbuffer = mbmalloc( sizeof(struct iopb) + 12 );
	dprintf(("sttbuffer is 0x%x\n",sttbuffer));
	sttsoftc.sc_iopb = (iopb_t *)sttbuffer;
	ststat_mbva = sttbuffer + sizeof(struct iopb);
	iop = (iopb_t *)sttbuffer;

	iop->i_unit = (unit + TAPEUNITOFFSET);
	iop->i_bufh = 0;
	iop->i_bufm = 0;
	iop->i_bufl = 0;
	/* set up the pbytes for 0 */
	iop->i_pbyte0 = 0;
	iop->i_pbyte1 = 0;
	iop->i_pbyte2 = 0;
	iop->i_pbyte3 = 0;
	iop->i_pbyte4 = 0;
	iop->i_pbyte5 = 0;
	iop->i_timeout = 6;
	/* Set up some of the IOPB which will not change */
	iop->i_rell = iop->i_relh = 0;
	iop->i_linkl = iop->i_linkh = iop->i_linkm = 0;
	iop->i_dmacount = DMACOUNT;
	/* Set up the I/O address in the IOPB */
	iop->i_ioh = MB(sttsoftc.sc_ioaddr);
	iop->i_iol = LB(sttsoftc.sc_ioaddr);
	/* update iopb */
	iop->i_tpoption = 0x04;
	iop->i_option = O_OPTIONS;
	/* Set up the iopb address in the I/O of the Controller */
	*STR1 = HB(iop);
	*STR2 = MB(iop);
	*STR3 = LB(iop);
	/* two CONFIGs cause the board has a problem */
	if (sttcmd(C_TPCONFIG, 1, unit)) {
		if (sttcmd(C_TPCONFIG, 1, unit)) {
			sttsoftc.sc_flags &= ~SC_ALIVE;
			return 1;
		}
	}
	iop->i_tpoption = 0;			/* zero out the option */
	sttsoftc.sc_flags |= SC_INITED;
	return 0;
}

/* Erase, Retention, Rewind, Write File Mark, Read File Marks, Seek Blocks */
simplecmd(unit, cmd, count)
	u_char cmd, unit;
	u_short count;
{
	register iopb_t *iop = sttsoftc.sc_iopb;

	iop->i_unit = (unit + TAPEUNITOFFSET);
	iop->i_bufh = 0;
	iop->i_bufm = 0;
	iop->i_bufl = 0;
	iop->i_option = O_OPTIONS;
	iop->i_filecountl = LB(count);
	iop->i_filecounth = MB(count);

	if (sttcmd(cmd, 1, unit))
		return 1;
	return 0;
}


sttstatus(unit)
	int unit;
{
	register iopb_t *iop = sttsoftc.sc_iopb;
	register struct softc_tape *st = &sttsoftc.sc_tape[unit];
	register u_long addr;
	register char *status;

	addr = (u_long)ststat_mbva;
	iop->i_unit = (unit + TAPEUNITOFFSET);
	iop->i_bufh = HB(addr);
	iop->i_bufm = MB(addr);
	iop->i_bufl = LB(addr);
	iop->i_option = O_OPTIONS;
	if (sttcmd(C_TPSTATUS, 1, unit)) {
		dprintf(("st%d: failure in reading tape status\n", unit));
		return 1;
	}
	status = ststat_mbva;
	st->sc_status1 = *status++;
	st->sc_status0 = *status++;
	st->sc_status3 = *status++;
	st->sc_status2 = *status++;
	st->sc_status5 = *status++;
	st->sc_status4 = *status;
	return 0;

}
