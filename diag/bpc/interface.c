/*
 *	Kurt Akeley					6 December 1983
 *
 *	Moved subroutines and variables formerly in uctest.h into this
 *	  file.
 */

short _ucbuffer;	/* used for requests only */
short dc_dcr;
short uc_ucr;
short uc_cfb;
short uc_edb;
short uc_ecb;
short uc_xsb;
short uc_xeb;
short uc_ysb;
short uc_yeb;
short uc_fmab;
short uc_rpb;
short uc_mdb;
short uc_ddasaf;
short uc_ddasai;
short uc_ddaeaf;
short uc_ddaeai;
short uc_ddasdf;
short uc_ddasdi;
short uc_ddaedf;
short uc_ddaedi;

#ifdef INTER2
#define GFBETA
#include <gfdev.h>
#include <betacodes.h>
#include "uctest.h"
#include <m68000.h>	/* for intlevel() only */

short ReadFont() {
    short i;

    intlevel (7);		/* doesn't work otherwise */
    SEND(BPCreadbus);
    SEND(8);
    FBCclrint;
    FBCflags = FORCEREAD;

    i = FBCdata;
    FBCflags = FORCEWAIT;
    FBCclrint;
    while (FBCdata != 0x40) ;	/* wait for idle state	*/
    intlevel (1);
    return i;
    }
#endif INTER2

/* not special declarations for interface 3 */
