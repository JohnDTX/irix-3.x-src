/*
* $Source: /d2/3.7/src/stand/mon/RCS/set.c,v $
* $Revision: 1.1 $
* $Date: 89/03/27 17:15:45 $
*/

#include	"sys/types.h"
#include	"setjmp.h"
#include	"cpureg.h"
#include	"common.h"
#include	"ctype.h"
#include	"parse.h"
#include	"strs.h"
#include	"tod.h"
#include	"dprintf.h"
#include	 "Xns.h"
#include 	"cntrlr.h"
#include 	"bootp/defs.h"

extern Xhost	_MyHostAddress;
extern char	_nxpresent;
extern short	_nxflags;

#define DFLT_NAME	"defaultboot"

char		btmedia[ 5 ];		/* holds the media string	*/
char		btdevspec[ 20 ];	/* holds the device spec string	*/


set( flag )
int	flag;		/* set if called during initialization	*/
{
	extern struct strinfo	mediastrs[];
	struct swregbits	swregbits;
	struct tod_dev		tod_dev;
	struct td_sginfo	*sip;
	int magic;
	register int		i,
				j;
	register char		*ptr;
	extern char		*strtok();
	extern char		*getext();
	extern char		*Version;
	long			val;
	short			revlevel;


	*(u_short *)&swregbits = *SWTCH_REG;

	if ( flag )
	{
		ptr = nlookup( swregbits.sw_bttype, mediastrs );
		if ( ptr != (char *)0 && ptr != (char *)-1 )
			strcpy( btmedia, ptr );
		else
			strcpy( btmedia, "none");

		ptr = getext( btmedia );

		switch ( ptr ) {

		    case -1:
			printf("Unknown boot type 0x%x\n",swregbits.sw_bttype);
			/* FALL THROUGH */
		    case 0:
			*btdevspec = '\0';
			break;
			
		    default:
			strcpy( btdevspec, ptr );
			break;
		}

		/* now check the battery backed up ram and clock	*/
		todread( CLK_DATA, (u_char *)&tod_dev, sizeof(struct tod_dev) );
		sip = &tod_dev.td_sgiun.td_sginfo;
		if ( sip->tds_pwrflg & P_DEADBATTERY ) {
			printf("Power has been lost to the machine\n");
		}
		/*
		** Initialize TCP boot variables
		*/
		btmyiaddr = sip->tds_iaddr[0]<<24 | sip->tds_iaddr[1]<<16 |
			    sip->tds_iaddr[2]<<8  | sip->tds_iaddr[3];
		bootp_siaddr = 0;
		bootp_giaddr = 0;
		bootp_file[0] = '\0';
		bootp_sname[0] = '\0';

		return;
	}

	if ( argc == 1 ) {

		todread( CLK_DATA, (u_char *)&tod_dev, sizeof(struct tod_dev) );
		printf( "Clock is %s:\n", tod_dev.td_regB & RB_SET ?
			"stopped" : "running" );
printf( "     Time      Alarms       Date        Registers\n" );
printf( "   %02d:%02d:%02d   %02d:%02d:%02d   %02d/%02d[%1d]/%02d   %02x %02x %02x %02x\n",
	tod_dev.td_hrs,
	tod_dev.td_min, tod_dev.td_sec, tod_dev.td_hrsalrm, tod_dev.td_minalrm,
	tod_dev.td_secalrm, tod_dev.td_month, tod_dev.td_dom, tod_dev.td_dow,
	tod_dev.td_year, tod_dev.td_regA, tod_dev.td_regB, tod_dev.td_regC,
	tod_dev.td_regD
      );
		sip = &tod_dev.td_sgiun.td_sginfo;
		magic = sip->tds_magic[0]<<24 | sip->tds_magic[1]<<16
			| sip->tds_magic[2]<<8 | sip->tds_magic[3];
	printf("Saved RAM: magic 0x%x, boot 0x%x, power 0x%x, dcr 0x%x\n",
	magic, sip->tds_btflg,sip->tds_pwrflg,sip->tds_dcrbits);

		if (  _commdat->c_memmb )
		{
			printf( "Boot Environment:\n" );
			printf( "   Media  : %s\n", btmedia );
			printf( "   Devspec: %s\n", btdevspec );
		}
		printf( "TCP/IP Network Boot Environment:\n%s",
			"   Internet address of this machine: " );
		if ( btmyiaddr )
			printf( "%s (%s %s)\n",
				inet_ntoa( ntohl( btmyiaddr ) ),
				"from",
				(sip->tds_iaddr[0] || sip->tds_iaddr[1] ||
			 	sip->tds_iaddr[2] || sip->tds_iaddr[3])
			 	? "Saved RAM" : "Boot Server" );
		else
			printf( "(not set)\n" );
		printf( "   Internet address of boot server:  %s\n",
			( bootp_siaddr ) ?
			(char *) inet_ntoa( ntohl( bootp_siaddr ) ) :
			"(not set)" );
		printf( "   Internet address of boot gateway: %s\n",
			( bootp_giaddr ) ?
			(char *) inet_ntoa( ntohl( bootp_giaddr ) ) :
			"(not set)" );
		printf( "Misc Environment:\n");
		printf( "   Version: %s\n", Version );
		printf( "   Serial Number: %d\n",
				(sip->tds_hserial << 8) | sip->tds_lserial );
		revlevel = REVLEVEL;
		printf( "   IP2 Revision %c with %d Meg of memory\n",
			(revlevel > 2) ? ('A' + (revlevel - 2)) : 'A',
			_commdat->c_memmb );
		printf("   FPA %s\n",_commdat->c_havefpa ?
				"is installed" : "not installed" );
		/*
		 * Call initialization routine to ensure that
		 * the Ethernet controller has been initialized.
		 */
		if ( ! ( _nxflags & INITED ) )
			_nxinit();
		if ( _nxpresent ) {
			register Xhost *xhp;

			xhp = &_MyHostAddress;
			printf("   Ethernet address is %x:%x:%x:%x:%x:%x\n",
				(xhp->high >> 8) & 0xff,
				xhp->high & 0xff,
				(xhp->mid >> 8) & 0xff,
				xhp->mid & 0xff,
				(xhp->low >> 8) & 0xff,
				xhp->low & 0xff);
		} else
			printf("   Ethernet controller is not installed\n");
		if ( Debug )
			printf( "   Debug:  %x\n", Debug );
		return;
	}


	if ( ! _commdat->c_memmb ) {
		printf( "Need memory to 'set'\n" );
		return;
	}

	if ( !strcmp( argv[ 1 ], "dcr") && argc == 4 ) {
		WRITEFIELD(td_sgiun.td_sginfo.tds_dcrbits, aton(argv[2], 16) );
		WRITEFIELD(td_sgiun.td_sginfo.tds_dcroption, aton(argv[3],16));
		return;
	}

	if ( !strcmp( argv[ 1 ], "display") ) {
		WRITEFIELD(td_sgiun.td_sginfo.tds_dcrbits,
					getdcrbits(swregbits.sw_discombo) );
		return;
	}

	if ( !strcmp( argv[ 1 ], "serial") && argc == 3 ) {
		if ( ( val = aton( argv[ 2 ], 10 ) ) == -1 ) {
			printf( "illegal value\n" );
		} else {
			WRITEFIELD(td_sgiun.td_sginfo.tds_hserial,
				val >> 8 );
			WRITEFIELD(td_sgiun.td_sginfo.tds_lserial,
				val );
		}
		return;
	}

	if ( !strcmp( argv[ 1 ], "servaddr") && argc == 3 ) {

		if ( ( val = inet_addr( argv[ 2 ] ) ) == -1 ) {
			printf( "illegal value\n" );
		} else {
			bootp_siaddr = val;
			bootp_sname[0] = '\0';
			if ( bootp_giaddr == 0 )
				bootp_giaddr = bootp_siaddr;
		}
		return;
	}
	
	if ( !strcmp( argv[ 1 ], "gateaddr") && argc == 3 ) {

		if ( ( val = inet_addr( argv[ 2 ] ) ) == -1 ) {
			printf( "illegal value\n" );
		} else {
			bootp_giaddr = val;
			bootp_sname[0] = '\0';
			if ( bootp_siaddr == 0 )
				bootp_siaddr = bootp_giaddr;
		}
		return;
	}
	
	if ( !strcmp( argv[ 1 ], "servname") && argc == 3 ) {
		strcpy( bootp_sname, argv[ 2 ] );
		return;
	}
	
	if ( !strcmp( argv[ 1 ], "inetaddr") && argc == 3 ) {

		if ( ( val = inet_addr( argv[ 2 ] ) ) == -1 ) {
			printf( "illegal value\n" );
		} else {
			WRITEFIELD(td_sgiun.td_sginfo.tds_iaddr[0], val >> 24 );
			WRITEFIELD(td_sgiun.td_sginfo.tds_iaddr[1], val >> 16 );
			WRITEFIELD(td_sgiun.td_sginfo.tds_iaddr[2], val >> 8 );
			WRITEFIELD(td_sgiun.td_sginfo.tds_iaddr[3], val );
			btmyiaddr = val;	/* Net order */
		}
		return;
	}
	
	if ( argc != 3 ) {
		help( SET_CMD );
		return;
	}

	if ( !strcmp( argv[ 1 ], "debug" ) ) {
		if ( ( val = aton( argv[ 2 ], 10 ) ) == -1 ) {
			printf( "illegal value\n" );
		} else
			Debug = val;
		return;
	}
	if ( !strcmp( argv[ 1 ], "media") ) {
		if ( slookup( argv[ 2 ], mediastrs ) == -1L ) {
			printf( "invalid media\n" );
			return;
		}
		strcpy( btmedia, argv[ 2 ] );
	} else
	if ( !strcmp( argv[ 1 ], "devspec") ) {
		strncpy( btdevspec, argv[ 2 ], 19 );
	} else
		help( SET_CMD );
}


/*
 * Initialize the battery backed up RAM
 */
todraminit()
{
	struct swregbits	swregbits;
	struct tod_dev		tod_dev;
	struct td_sginfo	*sip;
	int magic;

	todread( CLK_DATA, (u_char *)&tod_dev, sizeof(struct tod_dev) );
	sip = &tod_dev.td_sgiun.td_sginfo;
	magic = sip->tds_magic[0]<<24 | sip->tds_magic[1]<<16
			| sip->tds_magic[2]<<8 | sip->tds_magic[3];
	if ( !(tod_dev.td_regD & RD_VRT) || magic != SI_MAGIC ) {
		sip->tds_pwrflg = (P_DEADBATTERY | P_BADTIME);

		if ( magic != SI_MAGIC_OLD ) {

			/*
			 * Battery is really gone, init everything
			 */
			bzero( &tod_dev.td_sgiun, sizeof(tod_dev.td_sgiun) );

			*(u_short *)&swregbits = *SWTCH_REG;
			sip->tds_dcrbits = getdcrbits(swregbits.sw_discombo);
			sip->tds_1stmon = swregbits.sw_discombo >> 2;	/* XXX magic */
			sip->tds_2ndmon = swregbits.sw_discombo & 0x03;
			if ( sip->tds_2ndmon == DIS_EU )
				sip->tds_dcroption = 1;
			else
				sip->tds_dcroption = 0;
		}

		bzero( &sip->tds_notused,
			sizeof(sip->tds_notused) + sizeof(sip->tds_iaddr) +
			sizeof(tod_dev.td_sgiun) - sizeof(struct td_sginfo) );

		sip->tds_magic[0] = (SI_MAGIC >> 24) & 0xff;
		sip->tds_magic[1] = (SI_MAGIC >> 16) & 0xff;
		sip->tds_magic[2] = (SI_MAGIC >> 8)  & 0xff;
		sip->tds_magic[3] = SI_MAGIC & 0xff;

		todwrite(CLK_DATA + ((int)&tod_dev.td_sgiun - (int)&tod_dev),
			(u_char *)&tod_dev.td_sgiun, sizeof(tod_dev.td_sgiun) );
	} else {
		sip->tds_pwrflg &= ~P_DEADBATTERY;
		WRITEFIELD(td_sgiun.td_sginfo.tds_pwrflg,sip->tds_pwrflg);
	}
	/* Initialize the common area from tod ram	*/
	_commdat->c_bootflag = sip->tds_btflg;
	_commdat->c_powerflag = sip->tds_pwrflg;
	_commdat->c_dcconfig = sip->tds_dcrbits;
	_commdat->c_dcoptions = sip->tds_dcroption;
	_commdat->c_pridis = sip->tds_1stmon;
	_commdat->c_secdis = sip->tds_2ndmon;
	_commdat->c_magic = COMMON_MAGIC;
#define IADDR(p)	(*((iaddr_t *)(p)))
	IADDR( &_commdat->c_iaddr[0] ) = ntohl( IADDR( &sip->tds_iaddr[0] ) );
}


selectstring(prefix,spp,nums)
char *prefix;
char **spp;
int nums;
{
	int item, item_len, tot_len;
	int i, cpos;
	char *sp, c;

	printf("%s(",prefix);
	tot_len = 0;

	for (i=0; i<nums; i++) {
		sp = (char *)*(spp+i);
		printf("%s,", sp);
		tot_len += (strlen(sp) + 1);
	}

startit:
	printf("\b)");
	for ( i=0; i< tot_len; i++ )
		putchar('\b');

	item = 0;
	sp = *(spp+item);
	item_len = strlen(sp);
	cpos = 0;
	while ( 1 ) {
		c = getchar();
		switch (c) {

		case '\t':
			printf("%s,",(sp+cpos));
			cpos = 0;
			item++;
			if ( item >= nums )
				goto startit;
			sp = *(spp+item);
			item_len = strlen(sp);
			break;

		case '\n':
			printf("\n");
			return(item);
			break;

		case '\b':
			if ( cpos > 0 ) {
				cpos--;
				putchar('\b');
			} else {
				if ( item != 0 ) {
					item--;
					printf("\b\b");
					sp = *(spp+item);
					item_len = strlen(sp);
					cpos = item_len - 1;
				}
			}
			break;

		case ' ':
		default:
			if( cpos < item_len ) {
				putchar( *(sp+cpos) );
				cpos++;
			} else {
				putchar(',');
				cpos = 0;
				item++;
				if ( item >= nums )
					goto startit;
				sp = *(spp+item);
				item_len = strlen(sp);
			}
		}
	}
}


/*
** setdflts
**   set the defaults for the prefix and extension based on the
**   switch register
*/
setdflts( prfx, ext, name )
char	**prfx,
	**ext,
	**name;
{
	register char *cp;
	extern char	*getext();
	extern char	btmedia[];
	extern char	btdevspec[];

	/* hack for now */
	if ( **prfx == 0 )
		*prfx = btmedia;

	if ( **ext == 0 ) {
		if ( strcmp(*prfx,btmedia) == 0 ) {
			*ext = btdevspec;
		} else {
			cp = getext( *prfx );
			switch ( cp ) {
		    	case -1:
		    	case 0:
				*ext = "";
				break;
		    	default:
				*ext = cp;
				break;
			}
		}
	}

	if ( *name == 0 || **name == 0 )
		*name = DFLT_NAME;
}

char *
getext(media)
char	*media;
{
	int btype;		/* boot type	*/
	extern struct strinfo	mediastrs[];

	if ( media == 0 )
		return( (char *)0 );

	btype = slookup( media, mediastrs);

	switch ( btype ) {

	    /* the ethernet */
	    case BT_TCP:
	    case BT_XNS:
		return( "*" );
		break;

	    /* cartridge tapes */
	    case BT_TAPE:
	    case BT_ST:
	    case BT_MT:

	    /* all disks (get root slice from label)*/
	    case BT_HD:
	    case BT_MD:
	    case BT_SD:
	    case BT_IP:
	    case BT_FD:
	    case BT_MF:
	    case BT_SF:
		return( "0" );
		break;

	    /* misc */
	    case BT_ROM:
	    case BT_MONITOR:
		return( (char *)0 );
		break;

	    default:
		return( (char *)-1 );
		break;
	}
}
