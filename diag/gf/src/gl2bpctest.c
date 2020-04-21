/* bpctest.c
 *
 */

#include "m68000.h"
#define FBCsend
#define INTER4
#include "uctest.h"
#include "dcdev.h"

#ifdef PM2
#  define MODE1	*(unsigned char *)0xfb0004
#  define MODE2	*(unsigned char *)0xfb0006
#endif
#ifdef PM3
#  define u_long	unsigned long
#  define u_short	unsigned short
#  define u_char	unsigned char
#  include "cpureg.h"
#  include "common.h"
#  define MODE1	(_commdat->c_dcconfig & 0xff) << 4
#  define MODE2	(_commdat->c_dcconfig & 0xff00) >> 4
#endif

extern char line[];
extern short ix;
static mon = 0;
extern short dcr;

#ifndef EPROM
bpctest()
{
	short num,cmd,which;

    which = line[ix++];
    cmd = getnum();
    num = getnum();
    switch (which)  {
    case 's':
	LOAD(cmd,num);
	break;
    case 'c':
	REQUEST(cmd,num);
	break;
    case 'r':
	intlevel(7);
	SEND(BPCreadbus);
	SEND(8);
	FBCclrint;
	SEND(cmd|0x8000);
	FBCflags = FORCEREAD;
	printf("%x\n",FBCdata);
	FBCflags = FORCEWAIT;
	FBCclrint;
	while (FBCdata != 0x40) ;	/* wait for idle state	*/
	intlevel(2);
	break;
    case 'b':
	intlevel(7);
	SEND(BPCreadbus);
	SEND(8);
	FBCclrint;
	SEND(cmd);
	FBCflags = FORCEREAD;
	printf("%x\n",FBCdata);
	FBCflags = FORCEWAIT;
	FBCclrint;
	while (FBCdata != 0x40) ;	/* wait for idle state	*/
	intlevel(2);
	break;
    case 't':
	bpciftest();
	break;
    default:
	printf("   test interface\n");
	printf("   strobe <regno> <data>\n   command <cmdno> <data>\n");
	printf("   readcmd <cmdno>\n   buffer read <regno>\n");
#ifdef UC3
	printf("regno:\t8 config\n\t9 ED\n\ta EC\n\tb XS\n\tc XE\td YS\n\te YE\n\tf FA\n");
	printf("cmdno:\t0 ld vp\t8 wrpix\n\t1 wfont\t9 char\n\t2 color\ta rect");
	printf("\n\t3 no op\tb clear\n\t4 wrtwd\tc lin 0\n\t5 rd wd\td lin 1");
	printf("\n\t6 rotwd\te lin 2\n\t7 ld XY\tf lin 3\n");
#else
	printf("regno:\t1 ED\t2 EC\n\t3 XS\t4 XE\t5 YS\t6 YE\t7 FA\n\t8 SAF");
	printf("\t9 SAI\ta EAF\tb EAI\n\tc SDF\td SDI\te EDF\tf EDI\n\t10 mode");
	printf("\t11 repeat\t12 config\n");
	printf("cmdno:\t0 rfont\t1 wfont\n\t2 rd rept\t3 setaddrs\n\t4 savewd\t5 drawwd");
	printf("\n\t6 rd stip\t7 noop\n\t9 char\ta rect\tb trapz\n\tc lin 0");
	printf("\td lin 1\te lin 2\tf lin 3\n\t10 maskX\t11 maskY");
	printf("\n\t14 colorCD\t15 colorAB\t16 weCD\t17 weAB\n\t18 rdpixCD");
	printf("\t19 rdpixAB\t1a pixCD\t1b pixAB\n\t1c lin 0R\t1d lin1R");
	printf("\t1e lin 2R\t1f lin3R\n");
#endif UC3
    }
}
#endif EPROM
	
int bpciftest()	/* interface test - float a 1 thru font ram 0 */
{
	register i;
	int errs = 0;

	initall('b');
	intlevel(7);
	LOAD(7,0);	/* loadFA */
	for (i=1; i!=0x10000; i<<=1) {
		REQUEST(3,0);	/* setaddrs */
		REQUEST(1,i);	/* write */
		REQUEST(3,0);	/* setaddrs */
		SEND(BPCreadbus);
		SEND(8);
		buzz(1);
		FBCclrint;
		buzz(1);
		SEND(0x8000);
		buzz(1);
		FBCflags = FORCEREAD;
		buzz(1);
		if (FBCdata != i) {
			printf("BPC i/f: wrote %x  read %x\n",i,FBCdata);
			++errs;
		}
		FBCflags = FORCEWAIT;
		FBCclrint;
	}
	return(errs);
}

monselect()	/* toggle monitor selection */
{
	short i;

	if (i=getnum()) dcr = i;
	else {
		mon = 1 - mon;
		if (mon==0) dcr = (MODE2)<<7;
		else dcr = (MODE1)<<7;
	}
	dcr |= DCMULTIMAP;
printf("%d -> %x\n",mon,dcr);
	DCflags = dcr;
}
