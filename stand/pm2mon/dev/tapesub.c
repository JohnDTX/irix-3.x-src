/*
** $Source: /d2/3.7/src/stand/pm2mon/dev/RCS/tapesub.c,v $
** $Date: 89/03/27 17:17:18 $
** $Revision: 1.1 $
*/

# include "pmII.h"

# include "sys/types.h"
# include "dklabel.h"
#include "dsdreg.h"

#undef  DEBUG
#define OLDTAPEUNITS
extern char do_debug;

prom_tapeinit(unit)
	UCHAR unit;
{
	register i = 0;
	register tapestat_t *tapest;

	if(prom_topen(unit))
		return 1;
#ifdef OLDTAPEUNITS
	if(prom_tstat(unit) || prom_tstat(unit))
#else
	if(prom_tstat(unit))
#endif
		return 1;

	tapest = TAPEST;

	/*
	** Remember that the bytes are twisted
	*/
	if((i = tapest->sb[1]) ) {
		printf("QIC: Hard Error: %x\n", i);
		return 1;
	}
	if((i = tapest->sb[0]) ) {
		if(i&NOTAPE) {
			printf("QIC: No cartridge\n");
			return 1;
		}
		if(i&NOTRDY) {
			printf("QIC: unit Not ready\n");
			return 1;
		}
		if(i&~WRPROT)
			return 1;
	}
	return prom_rewind(unit);
}

prom_rewind(unit)
	UCHAR unit;
{
	register iopb_t *iop = IOPB;

	iop->p_mod = M_NOINT;
	iop->p_dba = 0;
	iop->p_rbc = iop->p_atc = 0;
	iop->p_dev = D_217;
	iop->p_unit = unit;
	iop->p_func = F_TREW;
	if(prom_tapecmd())
		return 1;
	/*
	** Have to add call to check for the end of the long command
	*/
	if(prom_tapewait())
		return 1;
	return 0;
}

/*
** prom_topen(unit)	- Open and initialize the controller for the Tape unit.
*/
prom_topen(unit)
	UCHAR unit;
{
	register wub_t *wub = WUB;

	wub->w_ext = 0;
	if(minit())
		return 1;
	/*
	** Can use the same config as the disk but with the D_217 device code.
	*/
	if(mconfig(D_217, F_INIT, 0))
		return 1;
	if(mconfig(D_217, F_TINIT, 0))
		return 1;
	if(mconfig(D_217, F_TRESET, 0))
		return 1;
	return 0;
}

/*
** prom_tstat(unit) - Read the tape status and put it into the local buffer for
**	   - analysis
*/
prom_tstat(unit)
{
	register i;
	register iopb_t *iop = IOPB;
	register tapestat_t *tapest = TAPEST;

	for(i = 0; i < 11; i++) tapest->sb[i] = 0;
	iop->p_dev = D_217;
	iop->p_unit = unit;
	iop->p_func = F_TDSTAT;
	iop->p_dba = (char *)SwapW(vtop(tapest));
	iop->p_mod = M_NOINT;
	iop->p_cyl = iop->p_hd = iop->p_sec = 0;
	iop->p_rbc = iop->p_atc = 0;
	if(prom_tapecmd())
		return 1;
	iop->p_func = F_TSTAT;
	iop->p_dba = (char *)SwapW(vtop(tapest));
	if(prom_tapecmd())
		return 1;
#ifdef DEBUG
	if(DEBUG)
	{
		for(i = 0; i < 11; i++) {
			printf("%x ", tapest->sb[i]);
		}
		printf("\n");
	}
#endif
	return 0;
}

/*
** Start a tape command --
** Returns '0' if OK on a short command.
** Returns '1' if Not OK and it failed.
** Returns '-1' if long command is running and is OK
** -- or -- short command timed out.
*/
prom_tapecmd()
{
	register ccb_t *ccb = CCB;
	register cib_t *cib = CIB;
	register struct mddevice *rp = MDIOADDR;

	while(ccb->c_busy != 0)
		;
	cib->i_stsem = 0;
# ifdef DEBUG
	if(DEBUG)
	{
		register iopb_t *iop = IOPB;
		printf("qic%d: dev %x cmd %x addr %x rbc %x atc %x\n",
			iop->p_unit, iop->p_dev, iop->p_func,
			SWAPW(iop->p_dba), SWAPW(iop->p_rbc),
			SWAPW(iop->p_atc));
	}
# endif DEBUG
	START(rp);
	return(prom_tapewait());
}

prom_tapewait()
{
	register tmo = 1000000;
	register struct cib *cib = CIB;
	register j;

	while(--tmo)
		if((j = prom_quickcheck()) != 0xdef) break;
	if(tmo <= 0) {
		printf("QIC: timeout\n");
		return (-1);
	}
	if(j & (HARD)) {
# ifdef DEBUG
	if(DEBUG)
	{
		register iopb_t *iop = IOPB;
		printf("qic%d error: dev %x cmd %x addr %x rbc %x atc %x\n",
			iop->p_unit, iop->p_dev, iop->p_func,
			SWAPW(iop->p_dba), SWAPW(iop->p_rbc),
			SWAPW(iop->p_atc));
	}
# endif DEBUG
		printf("QIC: error %x\n", (UCHAR)j);
		return 1;
	}
	return 0;
}


/*
** Tape quick check to see if status is ready
*/
prom_quickcheck()
{
	register i;
	register struct cib *cib = CIB;

	if(cib->i_stsem == 0) return (0xdef);
	i = cib->i_opstat;
	cib->i_stsem = 0;
	cib->i_opstat = 0;
	return (i);
}

/*
** prom_taperead(size, buf)
**			- Actually read the tape and send back an
**			- acknowledgement.
*/
prom_taperead(size, buf)
	ULONG size;
	long *buf;
{
	/*
	** Use the base multibus address to do the loading of the files
	*/
	register iopb_t *iop = IOPB;

	iop->p_dev = D_217;
	iop->p_func = F_READ;
	iop->p_unit = 0;
	iop->p_mod = M_NOINT;
	iop->p_cyl = iop->p_sec	= iop->p_hd = 0;
	iop->p_dba = (char *)SwapW(vtop(buf));
	iop->p_rbc = SWAPW(size);
	iop->p_atc = 0;

	if(prom_tapecmd()) {
		prom_tstat(iop->p_unit);
		return 1;
	}
	return  0;
}
