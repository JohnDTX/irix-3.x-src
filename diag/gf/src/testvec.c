/* testvec.c */

#ifdef _FBC
#include "fbc/fbc.h"
#include "GEsystem.h"
#endif

#ifdef _GF1
#include "fbcld.h"
#include "gfdev.h"
#endif

unsigned short testvecin[100];
unsigned short testvecout[100];
short testvi, testvo;

extern char line[];	/* command line buffer */
extern short intoccurred;
extern short expecting_interrupt;
extern short expecting_output;
extern short devstatus;	/* copy of currently written status reg */
extern short ix;	/* command line index */
extern char cmd,which,how;
extern short num;	/* current ucode addr */
extern short val;	/* field designator for dostore */
extern short low,high;	/* limits for block store */

testvec()
{
	short i;

switch (line[ix++]) {
    case '?':	printf("   define <i,o> vector from wd <n>\n");
		printf("   input <val>\n   output <val> expected\n   print <i,o>\n");
		for (i=0; i<100;) testvecin[i++] = 8;
		break;

    case 'd': switch(line[ix++]) {
		case 'i': testvi = getnum();
			break;
		case 'o': testvo = getnum(); break;
		default: printf("vd which?\n");
		}
	    break;
    case 'i': testvecin[testvi++] = getnum();
		break;
    case 'o': testvecout[testvo++] = getnum();
		break;
    case 'p': switch(line[ix++]) {
		case 'i': for (i=0; i<testvi; i++) {
				if (i%4==0) printf("\n%02x  ",i);
				printf("   %04x",testvecin[i]);
				}
			  putchar('\n');
			  break;
		case 'o': for (i=0; i<testvo; i++) {
				if (i%4==0) printf("\n%02x  ",i);
				printf("   %04x",testvecout[i]);
				}
			  putchar('\n');
			  break;
		default: printf("vp which?\n");
		}
	    break;
    default: illcmd();
    }
}
