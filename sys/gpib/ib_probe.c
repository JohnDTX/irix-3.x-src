# undef  DEBUG
#include "ib.h"

#include "../h/param.h"
#include "../h/types.h"
#include "../h/systm.h"
#include "../h/buf.h"
#include "../vm/vm.h"
#include "machine/cpureg.h"
#include "../multibus/mbvar.h"

# ifdef DEBUG
# undef DEBUG
# define DEBUG ib_probe_debug
# endif DEBUG

#include "../h/ib_ioctl.h"
#include "../gpib/ib_ieee.h"
#include "../gpib/ib_reg.h"
#include "../gpib/ib.defs"
#include "../gpib/ib_dbg.h"


extern struct ibvars ibvars[];
extern struct ibconn ibconn[];
extern struct tlc *tlcp[];

extern int ibprobe(), tlcintr(), ibuintr();
extern char *ibsname();
struct mb_ctlr *ibcinfo[NIB];
struct mb_driver ibdriver = {
	ibprobe, 0, 0, 0, tlcintr,
	0, "ib", 0, "ib", ibcinfo
};

int ib_nprobes;

/*
 * ibprobe() --
 * called at boot time.
 *
 * - tests for presence of iface.
 * - initialises driver variables.
 *
	addr		base address of dev regs
 *
 */
ibprobe(addr)
    long addr;
{
    register struct ibvars *vp;
    register short unit;

    dprintf((" ibprobe([$%x])",addr));
    unit = ib_nprobes++;
    if( unit >= NIB )
    {
	dprintf((" unit %d out of range",unit));
	return CONF_DEAD;
    }
    vp = ibvars+unit;
    vp->baseconn = ibconn+maketlc(unit,0);
    RP->base = (char*)(MBIO_VBASE+addr);
    RP->unit = unit;
    tlcp[unit] = RP;
    ibcinit(vp);

    RP->Cflags = 0;
    tlc_reset(RP);
    /*if no fault from above*/
    ibinit(vp);

    return CONF_ALIVE;
}
