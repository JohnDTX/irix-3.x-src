/*
 * $Source: /d2/3.7/src/sys/ipII/RCS/tod.h,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:31:03 $
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
		tds_lserial;
};

/*
** device layout
*/
struct tod_dev
{
	char	td_sec,		/* seconds		*/
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


/*
** some defines to make accessing the SGI specific info a little
** easier
*/
#define	td_btflg	td_sgiun.td_sginfo.tds_btflg
#define	td_pwrflg	td_sgiun.td_sginfo.tds_pwrflg
#define	td_wtimflg	td_sgiun.td_sginfo.tds_wtimflg

extern char	todsetflg();

/* defines for battery backed up RAM : */

#define SI_MAGIC	0x08161958	/* Scott's birthday	*/

#define	TD_BTFLG	0	/* booted flag		*/
#define	TD_PWRFLG	1	/* power fail flag	*/
#define	TD_WTIMFLG	2	/*watchdog timeout flag	*/

/* bits for tds_pwrflg	*/
#define P_DEADBATTERY	0x01	/* this is set on hard reset if power has been
				 * lost to the tod chip. It is reset on reset
				 * if this is not true
				 */
#define P_BADTIME	0x02	/* this is set when DEADBATTERY is set but it
				 * will only be cleared by some other software
				 */
