/*
* $Source: /d2/3.7/src/stand/include/RCS/tod.h,v $
* $Revision: 1.1 $
* $Date: 89/03/27 17:13:56 $
*/
/*
** Layout of the Motorola MC146818A CMOS real-time clock + RAM
**	This device has 50 bytes of low-power static RAM.
*/

/*
** static (battery back-up) RAM layout
*/
struct td_sginfo
{
	u_char	tds_magic[4];
	u_char	tds_btflg,	/* boot state flag		*/
		tds_pwrflg,	/* power fail flag		*/
		tds_wtimflg,	/* watchdog timer flag		*/
		tds_dcrbits,	/* packed bits for dc4 timing 	*/
		tds_dcroption,	/* option bits for kgl use 	*/
		tds_1stmon,	/* primary monitor type		*/
		tds_2ndmon,	/* secondary monitor type	*/
		tds_hserial,	/* serial number		*/
		tds_lserial,
		tds_notused,	/* pad to align iaddr		*/
		tds_iaddr[4];	/* internet address		*/
};

/*
** device layout
*/
struct tod_dev
{
	u_char	td_sec,		/* seconds		*/
		td_secalrm,	/* seconds alarm	*/
		td_min,		/* minutes		*/
		td_minalrm,	/* minutes alarm	*/
		td_hrs,		/* hours		*/
		td_hrsalrm,	/* hours alarm		*/
		td_dow,		/* day of week		*/
		td_dom,		/* day of month		*/
		td_month,	/* month		*/
		td_year,	/* year			*/
		td_regA,	/* register A		*/
		td_regB,	/* register B		*/
		td_regC,	/* register C		*/
		td_regD;	/* register D		*/

	union
	{
		struct td_sginfo	td_sginfo;
		char			td_rambytes[ 50 ];
	} td_sgiun;
};

/* Alarm types	*/
#define MS_ALARM	1
#define SEC_ALARM	2

/*
** Register A values (r/w except for UIP)
*/
#define	RA_UIP		0x80	/* update in progress		*/
#define	RA_DVRESET	0x70	/* reset divider chain		*/
#define	RA_DV4MHZ	0x00	/* 4.194304 MHz time base	*/
#define	RA_DV1MHZ	0x10	/* 1.048576 MHz time base	*/
#define	RA_DV32KHZ	0x20	/* 32.768 kHz time base		*/
#define	RA_RS1KHZ	0x06	/* 1K Hz periodic interrupt rate*/
#define	RA_RS512HZ	0x07	/* 512 Hz periodic interrupt rate*/
#define	RA_RS256HZ	0x08	/* 256 Hz periodic interrupt rate*/
#define	RA_RS128HZ	0x09	/* 128 Hz periodic interrupt rate*/
#define	RA_RS64HZ	0x0a	/* 64 Hz periodic interrupt rate*/
#define	RA_RS32HZ	0x0b	/* 32 Hz periodic interrupt rate*/
#define	RA_RS16HZ	0x0c	/* 16 Hz periodic interrupt rate*/
#define	RA_RS8HZ	0x0d	/* 8 Hz periodic interrupt rate*/
#define	RA_RS4HZ	0x0e	/* 4 Hz periodic interrupt rate*/
#define	RA_RS2HZ	0x0f	/* 2 Hz periodic interrupt rate*/
				/*  for any time base		*/
#define RA_DVIP2	RA_DV32KHZ

/*
** Register B values (r/w)
*/
#define	RB_SET		0x80	/* disable/enable time updates	*/
				/*  (disabled if set)		*/
#define	RB_PIE		0x40	/* enable periodic interrupts	*/
#define	RB_AIE		0x20	/* enable alarm interrupts	*/
#define	RB_UIE		0x10	/* enable end of update interval*/
#define	RB_SQWE		0x08	/* enable square wave output	*/
#define	RB_DMBIN	0x04	/* data mode - binary		*/
#define	RB_DMBCD	0x00	/* data mode - bcd		*/
#define	RB_HR24		0x02	/* hour format - 24 hour	*/
#define	RB_HR12		0x00	/* hour format - 12 hour	*/
#define	RB_DSE		0x01	/* enable daylight savings mode	*/

/*
** Register C values (read only)
*/
#define	RC_IRQF		0x80	/* interrupt request:		*/
				/*  IRQF = PF*PIE + AF*AIE + UF*UIE	*/
#define	RC_PF		0x40	/* periodic interrupt flag	*/
#define	RC_AF		0x20	/* alarm interrupt flag		*/
#define	RC_UF		0x10	/* update-ended interrupt flag	*/

/*
** Register D values (read only)
*/
#define	RD_VRT		0x80	/* valid RAM and time bit	*/
				/*  set by reading register D	*/
/*
** defines for the device status
*/
#define	TD_TODRUN	0x01	/* set if time is running	*/
#define	TD_CLKRUN	0x02	/* set if clock is running	*/
#define	TD_VALID	0x04	/* set if device holds valid info	*/
#define	TD_INVALID	0x08	/* set if device holds invalid info	*/

/*
** defines for driving the addressing logic. (see the IP2 spec)
*/
#define	TOD_AS		0x01
#define	TOD_DS		0x02
#define	TOD_RDENAB	0x04
#define	TOD_DRVCLK	0x08

/*
** misc defines
*/
#define	TOD_CLKSZ	10	/* size of the update area	*/
#define	MEMBERADR(f)	((char)(&(((struct tod_dev *)0)->f))&0x3f)

#define	READFIELD(f,v)	{\
			*CLK_DATA = MEMBERADR(f);\
			*CLK_CTL = TOD_RDENAB | TOD_DRVCLK | TOD_AS;\
			*CLK_CTL = TOD_RDENAB | TOD_DRVCLK;\
			*CLK_CTL = TOD_RDENAB | TOD_DS;\
			v = *CLK_DATA; *CLK_CTL = TOD_RDENAB; *CLK_CTL = 0;\
			}
#define	WRITEFIELD(f,v) {\
			*CLK_DATA = MEMBERADR(f);\
			*CLK_CTL = TOD_DRVCLK | TOD_AS;\
			*CLK_CTL = TOD_DRVCLK;\
			*CLK_DATA = v;\
			*CLK_CTL = TOD_DRVCLK | TOD_DS;\
			*CLK_CTL = TOD_DRVCLK | TOD_DS;\
			*CLK_CTL = TOD_DRVCLK;\
			*CLK_CTL = 0;\
			}

#define	RDTODADR(a,v)	{\
			*CLK_DATA = a;\
			*CLK_CTL = TOD_RDENAB | TOD_DRVCLK | TOD_AS;\
			*CLK_CTL = TOD_RDENAB | TOD_DRVCLK;\
			*CLK_CTL = TOD_RDENAB | TOD_DS;\
			v = *CLK_DATA; *CLK_CTL = TOD_RDENAB; *CLK_CTL = 0;\
			}
#define	WRTTODADR(a,v)	{\
			*CLK_DATA = a;\
			*CLK_CTL = TOD_DRVCLK | TOD_AS;\
			*CLK_CTL = TOD_DRVCLK;\
			*CLK_DATA = v;\
			*CLK_CTL = TOD_DRVCLK | TOD_DS;\
			*CLK_CTL = TOD_DRVCLK | TOD_DS;\
			*CLK_CTL = TOD_DRVCLK;\
			*CLK_CTL = 0;\
			}

extern u_char	todregB;

/* defines for battery backed up RAM : */

#define SI_MAGIC_OLD	0x08161958	/* Scott's birthday	*/
#define SI_MAGIC	0x08161959	/* Scott's 1st birthday	*/

/* bits for tds_pwrflg	*/
#define P_DEADBATTERY	0x01	/* this is set on hard reset if power has been
				 * lost to the tod chip. It is reset on reset
				 * if this is not true
				 */
#define P_BADTIME	0x02	/* this is set when DEADBATTERY is set but it
				 * will only be cleared by some other software
				 */
