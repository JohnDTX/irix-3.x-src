/*
* $Source: /d2/3.7/src/stand/simon/RCS/help.c,v $
* $Revision: 1.1 $
* $Date: 89/03/27 17:20:45 $
*/

#include	"sys/types.h"
#include	"strs.h"
#include	"parse.h"
#include	"cpureg.h"
#include	"common.h"

#define	HELPS_SZ	(sizeof helps/(sizeof (struct helps)))
#define	SHELPS_SZ	(sizeof shelps/(sizeof (struct helps)))

/*
** the struct that holds the help messages for usage strings as well as
** general help
*/
struct helps
{
	char	*h_msg;
	short	h_lines;
};

/*
** Contains all the help messages we print
** Each command is located at the index the represents its command id.
*/
struct helps	helps[] = {
{ "[MEDIA.DEVSPEC:][file]	load and begin execution of the named file\n\
				    file defaults to defaultboot\n\
				    SPECs are from switch settings\n",
  3	},

{ "b [MEDIA.DEVSPEC:][file]	same as above\n",
  1	},

{ "n [DEVSPEC:][file]		same as b with MEDIA = xns\n",
  1	},

{ "ls [MEDIA.DEVSPEC:][file]	list the files on the device \n",
  1	},

{ "l [MEDIA.DEVSPEC:][file]	load but don't begin execution of the file\n",
  1	},

{ "g address [stack]		start executing at specified address.\n\
				the stack address is an option.\n",
 2	},

{ "h|?				print this help message.\n",
   1	},

{ "set				print the current set values\n\
set media MEDIA			set the default boot media.\n\
set devspec DEVSPEC		set the default boot device spec.\n\
set debug 0/1			set the debug mode.\n\
set display 			set display options from switch settings.\n\
set dcr BITS OPTION		set DC4 bits and option.\n",
  6	},

{ "exit				reset the PROMS\n",
  1	},

{ "fm{b|w|l} ADDR VALUE [INCR] [CNT]\n\
				fill memory as byte, word or long\n\
				starting at ADDR, with initial VALUE, \n\
				incrementing VALUE by INCR for CNT times.\n\
				(INCR defaults to 0; CNT defaults to 1)\n",
  5	},
{ "dm{b|w|l} ADDR [CNT]		display memory as byte, word or long.\n\
				(CNT defaults to 1)\n",
  2	},

{ "em{b|w|l} ADDR			edit memory interactively as byte,\n\
				word or long.\n",
  2	},

{ "dpr				display processor registers.\n",
 1	},

{ "epr REGISTER			edit the given processor register.\n\
				(sr, vbr, cacr, caar, sfc, dfc).\n",
  2	},
};

/*
** help
**   print help messages.  If 'id' has a value, we print the string
**   as a usage message, otherwise we print the help info.
*/
help( cmd )
register short	cmd;
{
	register int	i;
	register int	lines;
	register int	maxlines;

	if ( _commdat->c_flags & TTY_DUMB )
		maxlines = 24 - 1;
	else
		maxlines = 40 - 1;

	if ( cmd >= 0 )
	{
		printf( Euse );
			printf( Fmtstr1, helps[ cmd ].h_msg );
	}
	else
	{
		lines = 23;
printf( "General Monitor Commands (All numeric values in hex):\n" );
printf( "  MEDIA is one of the following:\n" );
printf( "      hd	- hard disk.(look for ip, sd, then md)\n" );
printf( "      ct	- cartridge tape.(look for st, then mt)\n" );
printf( "      fd	- floppy disk (look for sf, then mf)\n" );
printf( "      ip	- interphase disk.\n" );
printf( "      sd(or si)- storager disk.\n" );
printf( "      md	- midas disk.\n" );
printf( "      st(or sq)- storager cartridge tape.\n" );
printf( "      mt(or mq)- midas cartridge tape.\n" );
printf( "      sf	- storager floppy disk.\n" );
printf( "      mf	- midas floppy disk.\n" );
printf( "      xns	- network.\n" );
printf( "      rom	- EPROM board.\n" );
printf( "  DEVSPEC is the one of the following:\n" );
printf( "      host name		- Name of the host. (MEDIA must be xns)\n" );
printf( "      unit		- unit number of device (0, 1, ...).\n" );
printf( "			    (MEDIA must be a tape or disk device.)\n" );
printf( "      <unit><fs>	- unit number and filesystem (a - h).\n" );
printf( "			    (MEDIA must be a disk device.)\n" );
printf( "      address		- multibus address.\n" );
printf( "			    (MEDIA must be the EPROM board)\n\n" );
		for ( i = 0; i < HELPS_SZ; i++ )
		{
			lines += helps[ i ].h_lines;
			if ( lines >= maxlines )
				if ( ! yes( Contfmt ) )
					return;
				else
					lines = 0;
			printf( Fmtstr1, helps[ i ].h_msg );
			putchar( '\n' ); lines++;
		}

	}
}
