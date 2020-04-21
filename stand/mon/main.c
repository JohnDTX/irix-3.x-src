/*
* $Source: /d2/3.7/src/stand/mon/RCS/main.c,v $
* $Revision: 1.1 $
* $Date: 89/03/27 17:15:41 $
*/
#include	"types.h"
#include	"cpureg.h"
#include	"common.h"
#include	"strs.h"
#include	"parse.h"
#include	"mbenv.h"
#include	"sysmacros.h"
#include	"setjmp.h"
#include	"stand.h"
#include	"dprintf.h"

#define	argc	(_commdat->c_argc)
#define	argv	(_commdat->c_argv)

#define	PROMPT		"iris> "

char	*MBioVA     = (char *)SEG_MBIO;
char	*MBmemVA    = (char *)( ONEMEG * 32 );
char	*MBmallocVA = (char *)( ONEMEG * 32 );
long	MBmallocSZ  = (ONEMEG/2) - 0x1000;	/* .5 Meg - WUB ugh!	*/
long	MBmemphys;
long	MBiophys;
char	*MBmappedVA;		/* virtual address of a multibus mapped area */
long	MBmapSZ	 = (ONEMEG/2);	/* maximum size of multibus mapped area	*/
char	*MBmappedphys;		/* multibus address of mapped area	*/
long	MBmappedstart = ONEMEG/2;

long	bss_sz;			/* size of the bss (pages) in the proms	*/

int	Inprom = 1;		/* flag which indicates if we are	*/
				/* in the PROMs or the prom code has	*/
				/* booted as a "bootable" pgm.		*/

extern	char	*Version;

main()
{
	register char		*cptr;
	register long		*lptr;
	register int		i;
	register long		pchunks;
	int			savsz;
	int			nptes;
	int			revlevel;
	struct pte		pte;
	struct swregbits	swregbits;
	extern			end,
				bstart;

	/*
	** If we running the prom code as just another bootable program
	** skip all the configuring stuff
	*/
	if ( !Inprom )
		goto skipit;

	LEDS(8);

	*TDBASE_REG = 0;	/* text/data at pte 0	*/
	*TDLMT_REG  = 0;	/* no text/data limit	*/

	/*
	** set the page table map.  We go thru the info that tells
	** us which megabytes exist.  For each megabyte that exists
	** we program the page table map.
	**
	** We use mfill() to program the actual page table entries
	** with the correct arguments.
	**
	** 0x1	increments by one the physical page number
	**
	** number of items is just the number of ptes needed to represent
	** 1megabyte.
	*/
	*(u_long *)&pte = PTE_SACC;

	for ( i = 0, lptr = (long *)PTMAP_BASE; i < MAXMEMMB; i++ )
	{
		if ( _commdat->c_mem & ( 1 << i ) ) {
			mfill( lptr,			/* starting addr      */
			       (short)(sizeof (long)),	/* length of an entry */
			       *(u_long *)&pte,		/* starting value     */
			       0x1,			/* increment	      */
			       ONEMEGPG );		/* # of items to do   */
			pte.pg_page += ONEMEGPG;
			lptr += ONEMEGPG;		/* next megabyte      */
		}
	}
	/*
	** zero out the remaining ptes here ????????
	*/
	*(u_long *)&pte = PTE_NOACC;
	for ( ; i < MAXMEMMB; i++ ) {
		mfill( lptr,			/* starting addr      */
		       (short)(sizeof (long)),	/* length of an entry */
		       *(u_long *)&pte,		/* starting value     */
		       0x0,			/* don't increment    */
		       ONEMEGPG );		/* # of items to do   */
		lptr += ONEMEGPG;		/* next megabyte      */
	}

	*(u_long *)&pte = PTE_SACC;
	/*
	** set the multibus map to point to the physical memory that the
	** multibus is allowed to use (The last physical megabyte).
	** We also setup the virtual addresses to point to these pages (MBmemVA)
	** If no physical memory - forget it.
	*/
	if ( _commdat->c_memmb ) {	/* check if we have any memory */
		for ( cptr = (char *)_commdat->c_mbmapadr, i = 0; i < ONEMEGPG;
		      cptr += NBPG, i++ )
			*(u_short *)cptr = _commdat->c_mbspg + i;

		pte.pg_page = _commdat->c_mbspg;

		lptr = (long *)vtoptv( MBmemVA );
		mfill( lptr, sizeof (long), *(u_long *)&pte,
		       0x1, ONEMEGPG );

		/*
		** seems to be a good time to program the bss ptes.
		** we allocate bss memory in chunks of a megabyte at
		** a time.
		*/
		/* number of bytes for bss */
		pchunks = (long)&end - (long)&bstart;

		/* to rounded number of pages */
		savsz = pchunks = btopr( pchunks );

		/*
		** point the the beginning of the last megabyte chunk in
		** the page map.
		*/
		lptr += ( ( pchunks + ONEMEGPG - 1 ) >> 8 ) << 8;

		/*
		** calc index into the memory bit array.  We start looking
		** before the megabyte assigned to the multibus memory area.
		** convert starting page to index into the memory map.
		*/
		i = ( _commdat->c_mbspg >> 8 ) - 1;
		if ( ! ( nptes = pchunks % ONEMEGPG ) )
			nptes = ONEMEGPG;
		for ( ; i >= 0 && pchunks; i-- )
		{
			/*
			** if memory present, suck up this chunk
			*/
			if ( _commdat->c_mem & ( 1 << i ) )
			{
				pte.pg_page = i << 8;
				if ( pchunks < ONEMEGPG )
					pte.pg_page += ONEMEGPG - nptes - 1;
				mfill( lptr, sizeof (long), *(u_long *)&pte,
				       0x1, nptes );
				pchunks -= nptes;
				nptes = ONEMEGPG;
				lptr -= ONEMEGPG;
			}
		}
skipit:
		LEDS(9);
		bss_sz = savsz;
		/*
		** set up the multibus memory and io addresses as viewed
		** from the multibus
		*/
		MBmemphys = ( (long)_commdat->c_mbmemadr & ~SEG_MSK );
		MBiophys = ( (long)_commdat->c_mbmapadr & ~SEG_MSK );
		MBmappedphys = (char *)(MBmemphys + MBmappedstart);

		/* only configure for graphics if there is memory */
		con_config();

	}

	LEDS(10);

	revlevel = REVLEVEL;
	printf( "IRIS (IP2 - Revision %c) Monitor Version %s",
			(revlevel > 2) ? ('A' + (revlevel - 2)) : 'A',
			Version);
	if ( !Inprom )
		printf( "(@ 0x%x)\n",main );
	else
		printf("\n");

	*(u_short *)&swregbits = *SWTCH_REG;
	if ( !(swregbits.sw_quite) )
		motormouth = 1;
	else
		motormouth = 0;

	if ( motormouth )
		printf( "Memory Size %dmb (Physical Map (1mb/bit) 0x%08x)\n",
			_commdat->c_memmb, _commdat->c_mem);

	if ( motormouth )
		printf( "Configuration Switch: 0x%04x\n", *SWTCH_REG );

	if ( ( i = swregbits.sw_mstrslv ) )
		i++;
	if ( motormouth )
		printf( "  Multibus Window (2mb) at Megabytes %d and %d.\n"
							, i, i + 1 );

	if ( _commdat->c_memmb ) {
		if ( motormouth ) {
			printf( "  Multibus accessible memory (1mb) begins\n" );
			printf( "    at Physical memory page %x,\n"
							,_commdat->c_mbspg);
			printf( "    at Virtual address %x.\n", MBmemVA );

			if ( _commdat->c_memmb == 1 ) {
				printf( "  WARNING: Multibus accessible memory overlaps all\n" );
				printf( "           Physical memory\n" );
			}
		}

		/*
		** set up the default boot state from the switch
		*/
		set( 1 );
	}
	else
		if ( motormouth )
			printf( "  No Multibus accessible memory\n" );

	while ( load(swregbits.sw_autobt &&
				(swregbits.sw_bttype != BT_MONITOR) ) < 0 )
		;
}

char buffi[ 80 ];

load(autoboot)
{
	register long	val1,	/* various parameters go here	*/
			val2,
			val3,
			val4;
	register short	cmd;	/* holds the command 		*/
	short		id,	/* holds the command id		*/
			len;	/* length of the command	*/
	int		trpval;
	int 		i;

	while ( 1 )
	{
		if ( !autoboot ) {
			getargv( PROMPT ); /* parse the command line	*/
			if ( argc == 0 )   /* no args typed, try again	*/
				continue;
			dprintf(("argc %d\n",argc));
			for ( i = 0; i < argc; i++ )
				dprintf(("argv[%d] -%s-\n",i,argv[i]));
			/*
			** lookup the command
			** save the cmd length (if any)
			** do the requested action
			*/
			id = clookup( argv[ 0 ] );
			len = ( id >> LNSHIFT ) & LNMSK;
		} else {
			delay_sec(2);	 /* delay autoboot tries */
			id = BOOT_CMD;
			argv[1] = "";
			printf("Autoboot\n");
		}

		switch ( cmd = ( id & CMDMSK ) )
		{
		   case ILL_CMD:

#ifdef TESTINGDEBUG
if ( !strcmp(argv[0], "testit" ) )
{
int c;
#include "iriskeybd.h"
extern char kbdtype;

	switch ( argv[ 1 ][ 0 ] )
	{
	    case '0':
		doit( aton( argv[ 2 ], 16 ) );
		break;

	    case '1':
		if ( argv[ 2 ][ 0 ] == '0' )
			kbdtype = KBD_IRIS;
		else
		if ( argv[ 2 ][ 0 ] == '1' )
			kbdtype = KBD_4D60STD;
		else
		if ( argv[ 2 ][ 0 ] == '2' )
			kbdtype = KBD_4D60ISO;
		break;

	    case '2':
		ttyconfig();
		c = con_config();
		printf( "2nd byte %x\n", c  );
		break;

	    default:
		printf( "0 - print hex value as a charcter\n" );
		printf( "1 - change kbd type (0: iris; 1: std; 2: iso)\n" );
		printf( "2 - config the keyboard\n" );
		break;
	}
	continue;
}
		if ( !strcmp(argv[0], "malloc" ) ) {

			printm();
			continue;
		}

		if ( !strcmp(argv[0], "screen" ) ) {
			int j;

			j = ScreenInit();
			printf("Screeninit returned %d\n",j);
			continue;
		}

		if ( !strcmp(argv[0], "cat" ) )
		{
			int fd;
			int cnt;

			if ( ( fd = open( argv[ 1 ], 0 ) ) < 0 )
			{
				perror( argv[ 1 ] );
				continue;
			}
			printf("cat opened %s\n",argv[1]);
			while ( 1 )
			{
				if ( ( cnt = read( fd, buffi, 80 ) ) < 0 )
				{
					perror( "read" );
					break;
				}
				if ( cnt == 0 )
					break;
				if ( write( 2, buffi, cnt ) < 0 )
				{
					perror( "write" );
					break;
				}
			}
			close( fd );
			continue;
		}
#endif

			return ( loadobj( argv[ 0 ] ) );

		   case BOOT_CMD:			/* PM2 compatability */
			/* get rid of "b" */
			for ( i = 1; i < argc; i++ )
				argv[ i - 1 ] = argv[ i ];
			argc--;
			argv[argc] = 0;
			return ( loadobj( argv[ 0 ] ) );

		   case XNS_CMD:			/* PM2 compatability */
			return ( netboot( XNS_CMD ) );

		   case LOAD_CMD:

			loadobj( argv[ 1 ] );
			printf("program loaded (not really jumping)\n");
			continue;

		   case HELP_CMD:
			help( -1 );
			continue;

		   case FILL_MEM:
			if ( argc < 3 || argc > 5 )
			{
				help( cmd );
				continue;
			}

			/*
			** cnt: default (1) or given
			*/
			val4 = 1;
			if ( argc > 4 )
			{
				if ( ( val4 = aton( argv[ 4 ], 16 ) ) == -1L )
				{
					printf( Fmtstr2, argv[ 4 ],  Einval );
					continue;
				}
			}
			/*
			** increment: default (0) or given
			*/
			val3 = 0;
			if ( argc > 3 ) 
			{
				if ( ( val3 = aton( argv[ 3 ], 16 ) ) == -1L )
				{
					printf( Fmtstr2, argv[ 3 ],  Einval );
					continue;
				}
			}
			if ( ( val2 = aton( argv[ 2 ], 16 ) ) == -1L )
			{
				printf( Fmtstr2, argv[ 2 ],  Einval );
				continue;
			}
			if ( ( val1 = aton( argv[ 1 ], 16 ) ) == -1L )
			{
				printf( Fmtstr2, argv[ 1 ],  Einval );
				continue;
			}

			if ( ( trpval = mfill( val1, len, val2, val3, val4 ) ) )
				trpprt( trpval );
			continue;

		   case DISP_MEM:
			if ( argc < 2 || argc > 3 )
			{
				help( cmd );
				continue;
			}

			val2 = 1;
			if ( argc > 2 )
			{
				if ( ( val2 = aton( argv[ 2 ], 16 ) ) == -1L )
				{
					printf( Fmtstr2, argv[ 2 ],  Einval );
					continue;
				}
			}

			if ( ( val1 = aton( argv[ 1 ], 16 ) ) == -1L )
			{
				printf( Fmtstr2, argv[ 1 ],  Einval );
				continue;
			}

			mdump( val1, len, (int)val2 );
			continue;
			
		   case EDIT_MEM:
			if ( argc != 2 )
			{
				help( EDIT_MEM );
				continue;
			}

			if ( ( val1 = aton( argv[ 1 ], 16 ) ) == -1L )
			{
				printf( Fmtstr2, argv[ 1 ],  Einval );
				continue;
			}
			medit( val1, (int)len, 0 ); 
			continue;

		   case DISP_PREG:
			prdump();
			continue;

		   case EDIT_PREG:
			if ( argc != 2 )
			{
				help( cmd );
				continue;
			}

			predit( argv[ 1 ] );
			continue;

		   case LS_CMD:
			if ( argc == 1 )
				listfile( 0 );
			else
				listfile( argv[1] );
			continue;

		   case RET_CMD:
			printf("PROMs exiting\n");
			return( 0 );

		   case SET_CMD:
			set( 0 );
			continue;

		   case GO_CMD:
			if ( argc < 2 || argc > 3 )
			{
				help( cmd );
				continue;
			}
			
			if ( ( _commdat->c_entrypt = aton( argv[ 1 ], 16 ) ) == -1L )
			{
				printf( Fmtstr2, argv[ 1 ],  Einval );
				continue;
			}
			_commdat->c_gostk = _commdat->c_memmb *
					    ( NBPG * ONEMEGPG );
			if ( argc > 2 )
				if ( ( _commdat->c_gostk = aton( argv[ 2 ], 16 ) ) == -1L )
				{
					printf( Fmtstr2, argv[ 2 ],  Einval );
					continue;
				}
			printf( "Go @ %08x (sp: %08x)\n", _commdat->c_entrypt,
				_commdat->c_gostk );
			return ( 1 );

		   case TCP_CMD:
			return ( netboot( TCP_CMD ) );

		}
	}
}

/*
** netboot
**
** common code shared by XNS and TCP boot commands that rearranges
** things so that argv looks as expected to loadobj()
*/
netboot(netcmd)
int netcmd;
{
	char	*pre, *ext, *path;
	int	i;

	/* all of this grotty code is to fake up argv */
	splitspec(argv[1], &pre, &ext, &path);
	pre = (netcmd == TCP_CMD) ? "tcp" : "xns";
	setdflts(&pre, &ext, &path);
	strcpy(buffi,pre);
	strcat(buffi,".");
	strcat(buffi,ext);
	strcat(buffi,":");
	strcat(buffi,path);

	/* get rid of "n" or "t" */
	for ( i = 1; i < argc; i++ )
		argv[ i - 1 ] = argv[ i ];
	/* set up argv */
	if ( argc > 2 ) {
		argc--;
		argv[argc] = 0;
		argv[0] = buffi;
	} else {
		argc = 1;
		argv[0] = argbuf;
		strcpy(argbuf,buffi);
	}

	return ( loadobj( buffi ) );
}

#ifdef TESTINGDEBUG
doit( val )
long	val;
{
	unsigned char	c;

	c = val & 0xff;

	printf( ">%c< (0x%x)\n", c & 0xff, c );
}
#endif
