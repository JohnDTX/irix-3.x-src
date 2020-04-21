/*
 * ikc.c
 *
 * $Source: /d2/3.7/src/sys/multibus/RCS/ikc.c,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:31:22 $
 */

/*
 * CONFIGURATION INFORMATION:
 * note that the proper configuration is partially
 * dependent on the type of printer and cabling being used!
 *
 * 0=OFF, 1=ON, x=DON'T_CHANGE
 * VDIFF=versatec_DIFF, VTTL=versatec_TTL
 *
 * dip switches:
 *
 *	U57 = 1 2 3 4 5 6 7 8   bus style
 *	      ---------------
 *	      x			-
 *		x		-
 *		  1		word (not byte) dma
 *		    1		drive CBRQ/
 *		      1		parallel/SERIAL bus pri
 *			0	bus vectored intr
 *			  0	drive INH1/
 *			    x	-
 *
 *	U59 = 1 2 3 4 5 6 7 8   csr
 *	      ---------------
 *	      1			a7
 *		1		a6
 *		  1		a5
 *		    0		mem mapped i/o
 *		      0		24 bit addressing
 *			0	8 bit addressing
 *			  x	-
 *			    x	-
 *
 *	U60 = 1 2 3 4 5 6 7 8   csr
 *	      ---------------
 *	      0			af
 *		1		ae
 *		  1		ad
 *		    1		ac
 *		      0		ab
 *			0	aa
 *			  0	a9
 *			    0	a8
 *
 *	U68 = 1 2 3 4 5 6 7 8   csr
 *	      ---------------
 *	      0			a17
 *		0		a16
 *		  0		a15
 *		    0		a14
 *		      0		a13
 *			0	a12
 *			  0	a11
 *			    0	a10
 *
 *	U71 = 1 2 3 4 5 6 7 8   comm style
 *	      ---------------
 *	      1			long data hold
 *		0		compressed option timing
 *		  0		low byte first dma
 *		    x		test pattern select
 *		      1		dma only when not BUSY
 *			0	device BUSY until trailing ACK
 *			  VDIFF	disable NOPAP
 *			    VDIFF select VDIFF
 *
 *	U78 = 1 2 3 4 5 6 7 8   intr level
 *	      ---------------
 *	      0			int0
 *		0		int1
 *		  0		int2
 *		    0		int3
 *		      0		int4
 *			1	int5
 *			  0	int6
 *			    0	int7
 *
 * jumpers:
 * (one is near pin 1 of P1, the other is near the reset/test toggle)
 *	both jumpers IE.
 *
 * option port cable terminations:
 *	470ohm			CENTRONICS TEK4695
 *	inverted 100ohm		TEK469[^5]
 *
 * cable connections:
 *	J1	CENTRONICS TEK469X
 *	J2	VTTL
 *	J3	VDIFF
 *
 */

#include "../h/param.h"
#include "../h/systm.h"
#include "../h/dir.h"
#include "../h/user.h"
#include "../h/buf.h"
#include "../h/setjmp.h"
#include "machine/cpureg.h"
#include "../multibus/mbvar.h"

#include "ik.h"

#include "../multibus/ikcreg.h"
#include "../h/ik_ioctl.h"
#include "../h/vcmd.h"

# define STATIC


STATIC struct iksoftc iksoftc;

extern int ikcprobe(), ikcintr();
STATIC struct mb_device *ikdinfo[NIK];
struct mb_driver ikdriver = {
	ikcprobe, 0, 0, 0, ikcintr, 0, "ik", ikdinfo
};

int ik_nbufs;
STATIC short ik_ieforce = 0;	/* force ie one way or another */
STATIC short ik_autoreset = 1;	/* enable automatic resets on open */
STATIC short ik_piomax = 32;	/* max pio in one burst */
STATIC short ik_ticks = HZ;	/* ticks per ik time interval */
STATIC short ik_maxbuzz = 10;	/* buzz count waiting for ready */
STATIC short ik_maxdel = HZ/6;	/* max polling interval in ticks */
STATIC short ik_timo = 5*60;	/* timeout in secs */

ikcprobe(reg)
	int reg;
{
	jmp_buf jb;
	int *onofault;
	register int t;

	onofault = nofault;
	nofault = jb;
	/*
	 * if the given address responds,
	 * we have the "old" ikon configuration
	 * with interrupts disabled.
	 * if ieforce>0 use interrupts anyway.
	 */
	if (setjmp(jb) == 0) {
		t = ik_probe1(reg, ik_ieforce>0);
		nofault = onofault;
		goto out;
	}

	/*
	 * otherwise,
	 * if the adjacent address responds, 
	 * we have the "new" ikon configuration
	 * with interrupts enabled.
	 * if ieforce<0, don't use interrupts.
	 */
	reg += IKNREGS;
	nofault = onofault;
	t = ik_probe1(reg, ik_ieforce>=0);
out:
	printf("(interrupts %s) ", iksoftc.sc_flags & SC_IENABLE
			? "enabled" : "disabled");
	return t;
}

/*
 * ik_probe1() --
 * called at hi pri.
 * probe for ikon board at given address,
 * and reset the interface.
 */
ik_probe1(reg, ie)
	int reg;
	int ie;
{
	iksoftc.sc_ioaddr = (caddr_t)MBIO_VBASE + reg;

	iksoftc.sc_flags = 0;
	iksoftc.sc_dev = CENTRONICS;
	ik_mode(0);

	/* if no fault from above */
	iksoftc.sc_flags = SC_ALIVE;
	if (ie)
		iksoftc.sc_flags |= SC_IENABLE;

	ik_ifreset();

	return CONF_ALIVE;
}

ikcopen(dev, flag)
	register dev_t dev;
	int flag;
{
	register int printer = DEVICE(dev);

	if (!(iksoftc.sc_flags & SC_ALIVE)) {
		u.u_error = ENXIO;
		return;
	}

	if (printer == RAW)
		return;

	if (iksoftc.sc_flags & SC_OPEN) {
		if (iksoftc.sc_dev == printer)
			return;
		u.u_error = EBUSY;
		return;
	}
	iksoftc.sc_flags |= SC_OPEN;
	iksoftc.sc_dev = printer;
	if (!(iksoftc.sc_flags & SC_TICKING))
		ik_gong(0);

	ik_lock();

	switch (printer) {

	case CENTRONICS:
	case VERS:
		if (ik_autoreset)
			ik_reset();
		else
			ik_mode(0);
		break;

	case TEK:
	case TEKPOLL:
		if (ik_tekopen() < 0)
			u.u_error = EIO;
		break;

	default:
		u.u_error = ENXIO;
		break;
	}

	if (u.u_error != 0)
		iksoftc.sc_flags &= ~(SC_OPEN|SC_HUNG);
	ik_unlock();
}

ikcclose(dev)
	dev_t dev;
{
	register int printer = DEVICE(dev);

	if (printer == RAW)
		return;

	ik_lock();

	switch (printer) {

	case CENTRONICS:
	case VERS:
		break;

	case TEKPOLL:
	case TEK:
		(void)ik_tekclose(printer);
		break;
	}

	iksoftc.sc_flags &= ~(SC_OPEN|SC_HUNG);
	ik_unlock();
}

ikcread(dev)
	dev_t dev;
{
	u.u_error = EIO;
}

ikcwrite(dev)
	dev_t dev;
{
	int printer = DEVICE(dev);

	ik_lock();

	switch (printer) {

	case RAW:
	case CENTRONICS:
	case VERS:
		ik_genwrite();
		break;

	case TEKPOLL:
	case TEK:
		ik_tekwrite(printer);
		break;
	}

	ik_unlock();
}
	
ik_genwrite()
{
	ik_blast(!(iksoftc.sc_flags & SC_PIOMODE));
}

ik_blast(dmamode)
	int dmamode;
{
	register struct buf *bp;
	register unsigned maxsize, count;

	GETEBLK(bp);
	ik_nbufs++;

	maxsize = dmamode ? IKDMASIZE : ik_piomax;

	while (!u.u_error && u.u_count > 0) {
		count = MIN(u.u_count, maxsize);
		iomove(KVADDR(bp), count, B_WRITE);
		if (u.u_error)
			break;

		if (dmamode)
			ik_dma((long)MBVADDR(bp), count);
		else
			ik_pio((UCHAR *)KVADDR(bp), count);
	}

	ik_nbufs--;
	BRELSE(bp);
}

ikcioctl(dev, cmd, arg, mode)
	dev_t dev;
	int cmd;
	caddr_t arg;
	int mode;
{
	int printer = DEVICE(dev);
	union {
		struct poke p;
		struct vstate v;
		int i;
	} j;

	switch (cmd) {

	case IKIOPEEK:
		if (!suser())
			return;
		if (copyin(arg, (caddr_t)&j.i, sizeof j.i)) {
			u.u_error = EFAULT;
			return;
		}
		j.i = IK_INREG(j.i);
		if (copyout((caddr_t)&j.i, arg, sizeof j.i)) {
			u.u_error = EFAULT;
			return;
		}
		break;

	case IKIOPOKE:
		if (!suser())
			return;
		if (copyin(arg, (caddr_t)&j.p, sizeof j.p)) {
			u.u_error = EFAULT;
			return;
		}
		IK_OUTREG(j.p.f, j.p.v);
		break;

	case IKIOPIOMODE:
		if (copyin(arg, (caddr_t)&j.i, sizeof j.i)) {
			u.u_error = EFAULT;
			return;
		}
		iksoftc.sc_flags &= ~SC_PIOMODE;
		if (j.i)
			iksoftc.sc_flags |= SC_PIOMODE;
		break;

	case IKIORESET:
		ik_lock();
		ik_reset();
		ik_unlock();
		break;

	case VGETSTATE:				/* XXX */
	case IKIOGETVSTATE:
		ik_getvstate(&j.v);
		if (copyout((caddr_t)&j.v, arg, sizeof j.v)) {
			u.u_error = EFAULT;
			return;
		}
		break;

	case VSETSTATE:				/* XXX */
	case IKIOSETVSTATE:
		if (copyin(arg, (caddr_t)&j.v, sizeof j.v)) {
			u.u_error = EFAULT;
			return;
		}

		ik_lock();
		(void)ik_setvstate(&j.v);
		ik_unlock();
		break;

	default:
		u.u_error = EINVAL;
		break;
	}
}

ikcintr()
{
	register int r;

	if (!(iksoftc.sc_flags & SC_ALIVE))
		return 0;

	IK_OUTREG(IK_IMR, IMR_POLL);		/* poll the device */
	r = IK_INREG(IK_ISR);
	if (!(r & (ISR_INTR|ISR_COND)))
		return 0;
	IK_OUTREG(IK_IMR, IMR_ACK);		/* ack it */
	iksoftc.sc_nintr++;

	if (iksoftc.sc_status) {
		wakeup((caddr_t)&iksoftc.sc_status);
		iksoftc.sc_status = 0;
	}
	return 1;
}


/* ----- auxiliary routines */
ik_gong(arg)
	int arg;
{
	if (!(iksoftc.sc_flags & SC_OPEN)) {
		iksoftc.sc_flags &= ~SC_TICKING;
		return;
	}

	timeout(ik_gong, 0, ik_ticks);
	iksoftc.sc_flags |= SC_TICKING;

	if (iksoftc.sc_timer > 0 && --iksoftc.sc_timer == 0)
		wakeup((caddr_t)&iksoftc.sc_status);
}

ik_lock()
{
	USEPRI;

	RAISE;
	while (iksoftc.sc_flags & SC_BUSY) {
		iksoftc.sc_flags |= SC_WANTED;
		sleep((caddr_t)&iksoftc.sc_flags, IKPRI);
	}
	iksoftc.sc_flags |= SC_BUSY;
	LOWER;
}

ik_unlock()
{
	if (iksoftc.sc_flags & SC_WANTED)
		wakeup((caddr_t)&iksoftc.sc_flags);
	iksoftc.sc_flags &= ~(SC_WANTED|SC_BUSY);
}

ik_reset()
{
	USEPRI;

	RAISE;
	ik_ifreset();

	if (iksoftc.sc_dev == VERS) {
		IK_OUTREG(IK_CR, CR_CLEAR);
	}
	else {
		ik_mode(MR_IPRIME);
		delay(2);
		ik_mode(0);
	}
	(void)ik_buzz(2);
	LOWER;
}

/*
 * ik_ifreset() --
 * called at hi pri.
 * reset the interface.
 */
ik_ifreset()
{
	register int t;

	t = IK_INREG(IK_SR);
	IK_OUTREG(IK_CR, CR_RESET|CR_SOFTACK);
	delay(5);

	if (iksoftc.sc_flags & SC_IENABLE) {
		t = IK_INREG(IK_SR);
		IK_OUTREG(IK_IMR, IMR_IENABLE);
		IK_OUTREG(IK_ICR, 0xFF);
		IK_OUTREG(IK_ICR, ~COND_RDY);
		ik_mode(MR_IENABLE);
	}
	else {
		ik_mode(0);
	}
}

ik_mode(mode)
	UCHAR mode;
{
	USEPRI;

	mode |= MR_OPTIMIZE;
	if (iksoftc.sc_flags & SC_IENABLE)
		mode |= MR_IENABLE;
	if (iksoftc.sc_dev != VERS)
		mode |= MR_OPTION;
	RAISE;
	IK_OUTREG(IK_MR, mode);
	LOWER;
}

int
ik_dma(mbva, count)
	register long mbva;
	register unsigned count;
{
	USEPRI;
	register UCHAR c;

	if (count == 0)
		return 0;

	RAISE;
	if (ik_wait() < 0)
		goto failed;

	c = mbva>>16;			/* high byte of address */
	IK_OUTREG(IK_MAR2, c);

	IK_OUTREG(IK_FLCLR, 0);		/* low and middle byte */

	c = mbva;
	IK_OUTREG(IK_MAR0, c);

	c = mbva>>8;
	IK_OUTREG(IK_MAR1, c);

	count--;
	IK_OUTREG(IK_FLCLR, 0);		/* low and middle byte of count */

	c = count;
	IK_OUTREG(IK_BCR0, c);

	c = count>>8;
	IK_OUTREG(IK_BCR1, c);

	IK_OUTREG(IK_DMASR, 0);		/* dma start! */

	if (ik_wait() < 0)
		goto failed;
	LOWER;
	return 0;
failed:
	LOWER;
	return -1;
}

int
ik_pio(ptr, count)
	register UCHAR *ptr;
	register unsigned count;
{
	while (count > 0) {
		if (ik_putc(*ptr++) < 0)
			return -1;
		count--;
	}

	return 0;
}

int
ik_putc(c)
	UCHAR c;
{
	USEPRI;

	RAISE;
	if (ik_wait() < 0)
		goto failed;
	IK_OUTREG(IK_DOR, c);
	if (ik_wait() < 0)
		goto failed;
	LOWER;
	return 0;
failed:
	LOWER;
	return -1;
}

/*
 * ik_wait() --
 * called at hi pri.
 * wait for ikon to be not busy, checking for faults.
 * if faults, handle by setting u.u_error & return -1.
 * return 0 when tek is ready.
 */
int
ik_wait()
{
	register int t;

	if (iksoftc.sc_flags & SC_HUNG) {
		u.u_error = EIO;
		return -1;
	}

	if (!(iksoftc.sc_flags & SC_IENABLE)) {
		if (ik_buzz(ik_timo) < 0)
			goto hung;
		return 0;
	}

	for (iksoftc.sc_timer = ik_timo; iksoftc.sc_timer != 0;) {
		if ((t = IK_INREG(IK_SR)) & SR_READY)
			return 0;

		iksoftc.sc_status = 1;
		sleep((caddr_t)&iksoftc.sc_status, IKPRI);
	}

hung:
	printf("Ikon hung\n");
	iksoftc.sc_flags |= SC_HUNG;

	u.u_error = EIO;
	return -1;
}

int
ik_buzz(maxsecs)
	int maxsecs;
{
	register time_t buzzer;
	register int t;
	int delaytime;

	for (buzzer = ik_maxbuzz; --buzzer >= 0;)
		if ((t = IK_INREG(IK_SR)) & SR_READY)
			return 0;

	delaytime = 0;
	for (buzzer = maxsecs * hz; (buzzer -= delaytime) >= 0;) {
		if ((t = IK_INREG(IK_SR)) & SR_READY)
			return 0;
		delay(delaytime);
		if (delaytime < ik_maxdel)
			delaytime++;
	}

	return -1;
}
/* ----- */


/* ----- versatec specific code */
ik_getvstate(vp)
	register struct vstate *vp;
{
	register int t;

	bzero((caddr_t)vp, sizeof *vp);
	t = IK_INREG(IK_SR);
	if (t & SR_PLOT)
		vp->f |= VPLOT;
	else
		vp->f |= VPRINT;
	if (t & SR_PPLOT)
		vp->f |= VPRINTPLOT;
	vp->timo = ik_timo;
}

int
ik_setvstate(vp)
	register struct vstate *vp;
{
	register int t;

	t = 0;
	if (vp->f & (VPRINTPLOT|VPLOT|VPRINT))
		t |= ik_vmode(vp->f);
	if (vp->f & (VLF|VFF|VREOT))
		t |= ik_vfunc(vp->f);
	if (vp->timo != 0)
		ik_timo = vp->timo;
	return t;
}

int
ik_vmode(f)
	int f;
{
	register int t;

	t = 0;
	if (f & VPRINTPLOT) {
		t |= MR_VPPLOT;
		if (IK_INREG(IK_SR) & SR_PLOT)
			t |= MR_VPLOT;
	}
	if (f & VPRINT)
		t &= ~MR_VPLOT;
	if (f & VPLOT)
		t |= MR_VPLOT;
	ik_mode(t);
	return 0;
}

int
ik_vfunc(f)
	int f;
{
	register int t;
	USEPRI;

	t = 0;
	if (f & VLF)
		t |= CR_EOL;
	if (f & VFF)
		t |= CR_FF;
	if (f & VREOT)
		t |= CR_EOT;
	RAISE;
	if (ik_wait() < 0)
		goto failed;
	IK_OUTREG(IK_CR, t);
	if (ik_wait() < 0)
		goto failed;
	LOWER;
	return 0;
failed:
	LOWER;
	return -1;
}
/* ----- */


/* ----- tek specific code */
/*
 * tektronix 4692 goo.
 *	- on open, reset and output tek init commands.
 *	  flag next write as tek header.
 *	- on first write, output header in pio mode.
 *	- each write thereafter is one raster line.
 *	  output tek eol command after each line.
 *	- on close, output tek close commands.
 *
 * all this is better done by utility code using
 * CENTRONICS mode.
 */
STATIC int ik_tekstate = 0;		/* tek dev state */
STATIC int ik_teklinesize = 0;		/* max bytes in a tek line */

int
ik_tekopen()
{
	ik_reset();
	if (ik_putc(T_ABORT) < 0 || ik_putc(T_RESERVE) < 0)
		return -1;
	ik_tekstate = TS_COM;
	return 0;
}

ik_tekclose()
{
	/*
	 * if we were spewing out raster lines,
	 * terminate page with EOT (normal condition)
	 */
	if (ik_tekstate == TS_RAST)
		(void)ik_putc(T_EOT);
	(void)ik_putc(T_ABORT);
}

ik_tekwrite(printer)
	int printer;
{
	if (ik_tekstate == TS_COM) {
		ik_tekcom();
		return;
	}
	ik_tekraster(printer);
}

ik_tekcom(printer)
	int printer;
{
	UCHAR tekarr[TEK_HEADSIZE];
	register int colorbits;

	if (u.u_count < TEK_HEADSIZE) {
		u.u_error = EINVAL;
		return;
	}

	iomove((caddr_t)tekarr, TEK_HEADSIZE, B_WRITE);
	if (u.u_error)
		return;
	u.u_count = 0;

	tekarr[1] &= ~0x40;	/* don't use streaming mode */

	colorbits = (tekarr[1]>>4) & 03;
	ik_teklinesize = TEK_PIXPERLINE << (colorbits-1);
	if (ik_pio(tekarr, (unsigned)TEK_HEADSIZE) < 0)
		return;
	ik_tekstate = TS_RAST;
}

ik_tekraster(printer)
	int printer;
{
	register unsigned count;

	if (u.u_count > ik_teklinesize)
		u.u_count = ik_teklinesize;

	ik_blast(printer==TEK);
	ik_putc(T_EOL);
}
/* ----- */
