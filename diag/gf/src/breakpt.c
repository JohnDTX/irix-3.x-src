/*  breakpt.c
 *
 *	breakset()
 *	breaksetall()
 *	breakclear()
 */

#include "gfdev.h"

extern unsigned short _ucode[][4];
extern short devstatus;
extern short num;		/* ucode adr */

char bpset[4096];

breakset(c)
    char c;
{
	short temp;

	switch (c) {
	    case '?':
		printf("   [set] at current adr\n");
		printf("   print current breakpoints\n");
		return(0);
	    case 'p':
		for (temp = 0; temp<4096; temp++)
		    if (bpset[temp]) printf("   %x",temp);
		putchar('\n');
		return(0);
	    default:
		bpset[num] = 1;
#ifdef GF2
		temp = (_ucode[num][3] & ~0xF8) | 0xB0;	/* GF2/UC4: INTRUPT */
#else
		temp = (_ucode[num][3] & ~0x78) | 0x20;	/* GF1/UC3: INTRUPT */
#endif
		FBCflags = WRITEMICRO;
		FBCmicroslice(3,num);
		FBCmicrocode(num) = temp;
		if (c != 'n') {
			FBCmicroslice(2,num);
			FBCmicrocode(num) = (_ucode[num][2] & ~0x40) | 0x80;
					/* LDOUT with no PUT */
		}
		hardinit();
		FBCflags = devstatus;
	}
}


breaksetall()
{
	register save;

	save = num;
	for (num=0; num<4096; num++)
		if (bpset[num]) breakset('c');
	printf("breakpoints at: "); breakset('p');
	num = save;
}


breakclear(c,adr)
    char c;
    short adr;
{
	register i;

	switch (c) {
	    case 'a':
		rewrite();
		for (i=0; i<4096; i++) bpset[i] = 0;
		printf("all bkpts cleared.\n");
		return(0);
	    case '?':
		printf("   all\n");
		printf("   loc. <n>\n");
		printf("   ---  current loc.\n");
		return(0);
	    case 'l':
		num = adr;
	    default:
		bpset[num] = 0;
		FBCflags = WRITEMICRO;
		FBCmicroslice(3,num);
		FBCmicrocode(num) = _ucode[num][3];
		FBCmicroslice(2,num);
		FBCmicrocode(num) = _ucode[num][2];
		FBCflags = devstatus;
	}
}
