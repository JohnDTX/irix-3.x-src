#ifndef IRISCONF
#define IRISCONF

/* Definitions for Iris Configuration Switches */

#define	HOSTSPEEDMASK	0x03	/* Select host baud rate */
#define	VERBOSEMASK	0x04	/* Verbose/Checkout */
#define HOSTSPEED(n)	((n)&HOSTSPEEDMASK)
#define VERBOSE(n)	((n)&VERBOSEMASK)

#define BOOTENVMASK	0xf8	/* Boot Environment */
#define	BOOTENVSHIFT	3	/* Shift right to word align */
#define	BOOTENV(n)	(((n)&BOOTENVMASK)>>BOOTENVSHIFT)

/*
 *	1  2  3  4  5  6  7  8  9  10 (ON PM1 Switch Settings)
 *	   1  2  3  4  5  6  7  8     (ON PM2 Switch Settings)
 *      .  .  .  .  .  .  .  .  .  .
 *			     X  X --> HostSpeed (Select host baud rate)
 *			  X --------> Checkout/Verbose  (PM1)
 *	   X  X  X  X  X -----------> BootEnv (Selects default startup mode)
 *	    	       X -----------> Supress Autoboot (using Bootenv)
 *	X --------------------------> TTL/RS-423 signal level (always closed)
 *      .  .  .  .  .  .  .  .  .  .
 *      -  9  8  7  6  5  4  3  2  1  (ON Back Panel for PM1)
 *      -  8  7  6  5  4  3  2  1  -  (ON Back Panel for PM2)
 *         7  6  5  4  3  2  1  0     (bit numbers internally (pm2) (0=lsb))
 */

#define	ENV_NOBOOT	0x01		/* Suppress auto boot */
#define ENV_FLOPPYBOOT	0x00		/* Boot from floppy disk */
#define ENV_MONITOR	0x02		/* Enter Iris Monitor */
#define ENV_NETBOOT	0x04		/* Boot using SGI Xns boot protocol */
#define ENV_TERMULATE	0x06		/* Enter Serial Terminal Emulation */
#define ENV_NETBOOT0    0x08		/* Default netboot 0 */
#define ENV_TAPEBOOT 	0x0a		/* boot from tape */
#define ENV_488BOOT	0x0c		/* Boot using SGI 488 boot protocol */
#define ENV_DISKBOOT    0x10		/* Boot from disk */
#define ENV_SMDBOOT	0x12		/* boot from smd disk */
#define AUTOBOOT(n)	(!(n & ENV_NOBOOT))

#define ENV_RPT_PGTEST	0x17		/* Repeat page map test on boot */
#define ENV_RPT_PTTEST	0x18		/* Repeat protection map test on boot */
#define ENV_RPT_CXTEST	0x19		/* Repeat context reg. test on boot */
#define ENV_RPT_DUARTST	0x1a		/* Repeat DUART test on boot */
#define ENV_RPT_TIMETST	0x1b		/* Repeat DUART timer tests on boot */
#define ENV_RPT_RAMBTST	0x1c		/* Repeat RAM boot page tests on boot */
/*  NOTE - if the bit value for ENV_SLAVE or ENV_DONTTOUCH is changed,
	it must be changed in the pm2.1start.s file - GB */
#define ENV_SLAVE	0x1d		/* pm2 board is slave processor */
#define ENV_DIAGNOSTICS	0x1e		/* Diagnostics mode */

#define ENV_DONTTOUCH	0x1f		/* Special debugging env. only
					   valid if VERBOSE set */
#define DONTTOUCH(n)	(BOOTENV(n)==ENV_DONTTOUCH&&VERBOSE(n))

extern unsigned short switches;

#endif IRISCONF
