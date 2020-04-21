/*	ALLRIGHTSRESERVEDSPACE	*/

/*	Author:	RSJ
 *	Date:	2-Sep-83
 *
 *	Modification history:
 *
 *	Purpose: 68000 - Host computer serial link
 *
 *		On startup, do quick memory check (see memchk()).
 *		Then become a transparent 9600 baud line to the
 *		host computer, until a load command is received
 *		from the host. Load records into 68000 memory, and
 *		then 'GO' at the address specified in the last rcvd
 *		record.
 *
 *		Return to this program is possible by a 'trap #14'
 *		instruction. The system is initialized, and transparent
 *		mode is reentered.
 */
		
#include "mem.h"
#include "duart.h"	/* order matters!!! */
#include "queue.h"


#define MAXCNT  5000000				/* time out counter	*/
						/* not implemented yet	*/
#define NOCHAR	-1				/* No character input	*/

unsigned long checksum;				/* checksum routine	*/
long sscount;					/* Saved count for debugging */
unsigned short *ssaddr;				/* Saved addr for debugging */
int cntxx;					/* cleared on startup	*/
unsigned char cxx[2];				/* Saved chars for cksum */

main()
{
	register duart *dp;
	register i;

#ifdef PROM
	dinit(0);		/* Init both duarts */
	dinit(2);
	for(i = 10000; --i; ) ;
	printf("\n\nPM2.1 Boot Proms V1.0 October 12, 1983\n");
	memchk(1);
#endif
	printf("Transparent mode\n");

/* Become the transparent 68000 - host connection (until load command seen) */

	initqueue(&fromlocal,LOCAL,(unsigned char *)0x10000);
	initqueue(&fromhost,HOST,fromlocal.eq);

	for(;;) {
		/*
		 * Until a control sequence is seen, continually poll both the
		 * host and the 68000 for characters
		 */
		slurpchars(&fromhost);		/* Fill queue from HOST */
		if(fromhost.flag == 1)		/* Saw "<CTRL-^>L" */
			doload();		/* NO RETURN */

		dp = fromlocal.dptr;		/* Output to local */
		while(fromhost.count && (dp->d_sr & SRTR))
			dp->d_thr = getqueue(&fromhost);

		slurpchars(&fromlocal);		/* Fill queue from local */
		dp = fromhost.dptr;		/* Output to HOST */
		while(fromlocal.count && (dp->d_sr & SRTR))
			dp->d_thr = getqueue(&fromlocal);
	}
}

/* The following is the layout of the packet received from the host
 * (after the control sequence <control-shift-^> <L> is received)
 *
 *	byte	function
 *	----	--------
 *	  0	Record type
 *			'L' load record, which uses all the following fields
 *			'G' go   record, which only uses/gets the type and
 *				address fields. Checksum not used.
 *
 *	1,4	Address
 *			if 'L' record, beginning address to load data
 *			if 'G' record, xfer address
 *
 *	5,6	Length
 *			byte length; always even, and 0 -> 64k bytes.
 *
 *	7,(n-2)	Data
 *			Unsigned eight bit chunks
 *
 *	(n-1),n	Checksum
 * 			See sum.c for description of checksum used.
 */

doload()
{
	register c;				/* current character	*/
	register i;
	register unsigned short length;
	register unsigned char *addr;		/* Load address		*/
	register ttype;				/* type of xfer		*/
						/* 'L' = load record
						 * 'G' = Go   record	*/
	union { unsigned long l;		/* rcvd xfer adrs	*/
		unsigned short s[2];
		char c[4];} taddr;

	union { unsigned short s;		/* rcvd xfer length	*/
		char c[2];} tlength;		/* 0 -> 64k bytes	*/

	/* we get here ONLY if the control sequence indicating
	 * an xfer function is received from the host
	 *
	 * states	0   is get the 'L' or 'G' byte
	 *		1-4 is get the address longword
	 *		5-6 is get the message length
	 *		7   is pump in the data
	 *		8-9 is check checksum
	 */
	i = 0;
	for(;;) {
		c = timedin(MAXCNT);
		if(c == -1) {
			printf("\nTimeout waiting for HOST. ");
			printf("Start addr=0x%x, cur addr=0x%x, len=%d\n  ",
				ssaddr, addr, length);
			for(i = 0; i < 5; i++)
				printf("%x ", ssaddr[i]);
			printf("\n");
			exit();
		}
		switch (i) {
		case 0:				/* RECORD TYPE - G or L	*/
			cntxx = 0;		/* Reset the world */
			checksum = 0;		/* no chksum */
			ttype = c;		/* Save packet type */
			if(c != 'G' && c != 'L') {
	errout:			while(timedin(MAXCNT) != -1) ;
				if(i == 0)
	printf("PACKET ERROR: type=0x%x != G or L\n", ttype);
				else
	printf("PACKET ERROR: first byte of address (0x%x) not == 0!\n", c);
				putcraw('N', HOST);
				putcraw('\n', HOST);
				exit();
			}
			break;			/* On to next state */
		case 1:				/* ADDRESS LONGWORD	*/
			if(c != 0)
				goto errout;
		case 2:
		case 3:
		case 4:
			taddr.c[i-1] = c;
			break;
		case 5:				/* BYTE COUNT 0	*/
			addr = (unsigned char *)taddr.l;
			ssaddr = (unsigned short *)addr;
			if ( ttype == 'G') {	/* GO RECORD? */
			    printf("Starting at %x:\n\n", addr);
			    (* ((int (*)()) addr))();	/* Start it up */
			}
			tlength.c[0] = c;
			break;
		case 6:				/* BYTE COUNT 1	*/
			tlength.c[1] = c;
			length = tlength.s;
			sscount = length;
			break;
		case 7:				/* DATA	*/
			*addr++ = c;
			if(((long)addr & 0x3FF) == 0)
				putchar('.');	/* Every 1K */
			if (--length != 0)
			    continue;		/* Loop in this state */
			break;
		case 8:				/* CHECKSUM 0 */
			break;
		case 9:				/* CHECKSUM 1 */
			if ( checksum == 0){
				i = 0;
				putchar('\n');
				putcraw('A', HOST); /* acknowledge */
				putcraw('\n', HOST);
				continue;
			}
			printf("\nError: Checksum = %x ", checksum);
			printf("Addr=0x%x, count=%d\n", ssaddr, sscount);
			putcraw('N', HOST);	/* Negative acknowledge	*/
			putcraw('\n', HOST);
			exit();			/* Will come back to us */
		default:
			printf("\nThe FSM is broken ---");
			exit();
		}
		++i;
	}
}


timedin(timelimit)
register timelimit;
{
	register duart *dp = dad[HOST];
	register c;

	while(--timelimit) {
		if(dp->d_sr & SRRR) {
			c = dp->d_rhr;
			cxx[cntxx] = c;
			if (cntxx){
				sum(cxx);
				cntxx = 0;
			} else
				cntxx++;
			return c;
		}
	}
	return -1;
}
