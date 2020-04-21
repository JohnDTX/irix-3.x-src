char _Origin_[] = "UC Berkeley";

/*
 * Print system stuff
 */

#define mask(x) (x&0377)
#include <sys/param.h>
#include <sys/types.h>
#include <sys/sysmacros.h>
#include <sys/conf.h>
#include <sys/tty.h>
#include <sys/var.h>
#include <a.out.h>
#include <stdio.h>

struct var v;

char	*fcore	= "/dev/mem";
char	*fnlist	= "/unix";
int	fc;

struct nlist setup[] = {
#define	SINODE	0
{"_inode"},
#define	STEXTT	1
{"_text"},
#define	SPROC	2
{"_proc"},
#define	STTY	3
{"_tty_sta"},
#define	SFIL	4
{"_file"},
#define SMEMM	5
{"_coremap"},
#define SMAXMEM	6
{"_maxmem"},
#define SCFREE	7
{"_cfreeco"},	/* _cfreecount truncated for nlist structure */
#define SCWAIT	8
{"_cwaitin"},
#define SVAR	9	/* system NFILE */
{"_v"},
#define	SBUF	10
{"_buf"},
{""}
};

int	inof;
int	txtf;
int	vflg;		/* verbose table dumps */
int	prcf;
int	ttyf;
int	usrf;
int 	dumpf;
int	memf;
int 	sysf;
int 	buff;
int 	rflg;
long	ubase;
int	filf;
int	allflg;
int	summary;
int 	rsleep = 10;

main(argc, argv)
char **argv;
{
	char _obuf[BUFSIZ];

	setbuf(stdout, _obuf);
	while (--argc && **++argv == '-') {
		while (*++*argv)
		switch (**argv) {
		case 'a': allflg++; break;
		case 'b': buff++; break;
		case 'd': dumpf++; break;
		case 'f': filf++; break;
		case 'i': inof++; break;
		case 'm': memf++; break;
		case 'p': prcf++; break;
		case 's': sysf++; break;
		case 't': ttyf++; break;
		case 'v': vflg++; break;
		case 'x': txtf++; break;
		case 'r':
		    rflg++;
		    if (argv[0][1] != 0)
			    rsleep = atoi(&argv[0][1]);
		    break;
		case 'u':
			if (--argc == 0)
				break;
			usrf++;
			ubase = xatoi(*++argv);
			break;
		}
	}
	if (buff==0 && filf==0 && inof == 0 && prcf==0 && txtf == 0 &&
	    usrf==0)
		summary++;
	if (argc>0)
		fcore = argv[0];
	if ((fc = open(fcore, 0)) < 0) {
		printf("Can't find %s\n", fcore);
		exit(1);
	}
	if (argc>1)
		fnlist = argv[1];
	nlist(fnlist, setup);
	if (setup[SINODE].n_type == 0) {
		printf("no namelist\n");
		exit(1);
	}

	pseek(fc, (long)setup[SVAR].n_value, 0);
	read(fc, (char *)&v, sizeof(v));
	if (v.v_buf <= 3 || v.v_buf >= 2000) {
		printf("System III name list out of date\n");
		exit(1);
	}
loop:
	if (buff || summary)	{ dobuf(); fflush(stdout); }
	if (inof || summary)	{ doinode(); fflush(stdout); }
	if (txtf || summary)	{ dotext(); fflush(stdout); }
	if (ttyf)		{ dotty(); fflush(stdout); }
	if (prcf || summary)	{ doproc(); fflush(stdout); }
	if (usrf)		{ dousr(); fflush(stdout); }
	if (filf || summary)	{ dofil(); fflush(stdout); }
	if (memf)		{ domem(); fflush(stdout); }
	if (sysf)		{ dosys(); fflush(stdout); }
	if (rflg) {
		fflush(stdout);
		if (rsleep != 0)
		    sleep(rsleep);
		putc('\n', stdout);
		goto loop;
	}
}

dobuf()
{
#include <sys/buf.h>
	register struct buf *bp, *xbuf;
	register int nbusy, ndone, nwanted, nnodev, loc;

	nbusy = ndone = nwanted = nnodev = 0;

	if (v.v_buf <= 3 || v.v_buf >= 1000) {
		printf("System name list out of date\n");
		exit(1);
	}

	if ((xbuf=(struct buf *)malloc(v.v_buf * sizeof(*bp))) == NULL) {
		printf("pstat:could not get %d bytes for buf table\n",
			v.v_buf * sizeof(*bp));
		return;
	}

	pseek(fc, (long)setup[SBUF].n_value, 0);
	read(fc, (char *)xbuf, v.v_buf * sizeof(*bp));

	for (bp = xbuf; bp < &xbuf[v.v_buf]; bp++) {
		if (bp->b_dev == NODEV) {
			nnodev++;
			continue;
		}
		if (bp->b_flags&B_BUSY) nbusy++;
		if (bp->b_flags&B_WANTED) nwanted++;
		if (bp->b_flags&B_DONE) ndone++;
	}
	printf("%d buffers: %d busy, %d wanted, %d done, %d nodev\n",
		v.v_buf, nbusy, nwanted, ndone, nnodev);
	if (buff == 0)
		return;

	printf("   LOC     FLAGS     DEVICE    ADDR  BLKNO\n");
	loc = setup[SBUF].n_value;
	for (bp = xbuf; bp < &xbuf[v.v_buf]; bp++, loc += sizeof(*bp)) {
		if (!(vflg || (bp->b_flags&B_DONE)==0 ||
		    bp->b_flags&(B_ERROR|B_BUSY|B_PHYS|B_MAP|B_WANTED|B_AGE|B_ASYNC|B_DELWRI|B_OPEN|B_STALE)))
			continue;
		if (vflg==0 && bp->b_dev==NODEV)
			continue;
		printf("%6x ", loc);
		putc(bp->b_flags&B_READ ? 'R' : 'W', stdout);
		putf(bp->b_flags&B_DONE, 'D');
		putf(bp->b_flags&B_ERROR, 'E');
		putf(bp->b_flags&B_BUSY, 'B');
		putf(bp->b_flags&B_PHYS, 'P');
		putf(bp->b_flags&B_MAP, 'M');
		putf(bp->b_flags&B_WANTED, 'W');
		putf(bp->b_flags&B_AGE, 'A');
		putf(bp->b_flags&B_ASYNC, 'Y');
		putf(bp->b_flags&B_DELWRI, 'L');
		putf(bp->b_flags&B_OPEN, 'O');
		putf(bp->b_flags&B_STALE, 'S');
		if (bp->b_dev == NODEV)
			printf("  NODEV");
		else
			printf("%3d,%3d", major(bp->b_dev), minor(bp->b_dev));
		printf("%8x", bp->b_un.b_addr);
		if (bp->b_dev == NODEV)
			printf("       ");
		else
			printf("%7d", bp->b_blkno);
		printf("\n");
	}
	free((caddr_t)xbuf);
}

doinode()
{
#include <sys/inode.h>
	register struct inode *ip, *xinode;
	register int nin, loc;

	nin = 0;

	if (v.v_inode <= 3 || v.v_inode >= 1000) {
		printf("System name list out of date\n");
		exit(1);
	}

	if ((xinode=(struct inode *)malloc(v.v_inode * sizeof(*ip))) == NULL) {
		printf("pstat:could not get %d bytes for inode table\n",
			v.v_inode * sizeof(*ip));
		return;
	}

	pseek(fc, (long)setup[SINODE].n_value, 0);
	read(fc, (char *)xinode, v.v_inode * sizeof(*ip));

	for (ip = xinode; ip < &xinode[v.v_inode]; ip++)
		if (ip->i_count)
			nin++;
	printf("%d out of %d inodes active\n", nin, v.v_inode);
	if (inof == 0)
		return;

	printf("   LOC  FLAGS CNT DEVICE   INO   MODE NLK UID  SIZE/DEV    LOCK\n");
	loc = setup[SINODE].n_value;
	for (ip = xinode; ip < &xinode[v.v_inode]; ip++, loc += sizeof(*ip)) {
		if (vflg==0 && ip->i_count == 0)
			continue;
		printf("%6x ", loc);
		putf(ip->i_flag&ILOCK, 'L');
		putf(ip->i_flag&IUPD, 'U');
		putf(ip->i_flag&IACC, 'A');
		putf(ip->i_flag&IMOUNT, 'M');
		putf(ip->i_flag&IWANT, 'W');
		putf(ip->i_flag&ITEXT, 'T');
		printf("%4d", ip->i_count&0377);
		printf("%3d,%3d", major(ip->i_dev), minor(ip->i_dev));
		printf("%6d", ip->i_number);
		printf("%7o", ip->i_mode);
		printf("%4d", ip->i_nlink);
		printf("%4d", ip->i_uid);
		if ((ip->i_mode&IFMT)==IFBLK || (ip->i_mode&IFMT)==IFCHR)
			printf("%6d,%3d", major(ip->i_rdev), minor(ip->i_rdev));
		else
			printf("%10d", ip->i_size);
		printf(" %8x", ip->i_locklist);
		printf("\n");
	}
	free((caddr_t)xinode);
}

putf(v, n)
{
	putc(v?n:' ', stdout);
}

dotext()
{
#include <sys/text.h>
	register struct text *xp, *xtext;
	register loc, ntx;

	ntx = 0;

	if (v.v_text <= 3 || v.v_text >= 1000) {
		printf("System name list out of date\n");
		exit(1);
	}

	if ((xtext = (struct text *)malloc(v.v_text * sizeof(*xp))) == NULL) {
		printf("pstat:could not get %d bytes for text table\n",
			v.v_text * sizeof(*xp));
		return;
	}

	pseek(fc, (long)setup[STEXTT].n_value, 0);
	read(fc, (char *)xtext, v.v_text * sizeof(*xp));

	for (xp = xtext; xp < &xtext[v.v_text]; xp++)
		if (xp->x_iptr!=NULL)
			ntx++;
	printf("%d out of %d text segments active\n", ntx, v.v_text);
	if (txtf == 0)
		return;

	printf("    LOC  FLAGS DADDR  CADDR SIZE  IPTR  CNT CCNT\n");
	loc = setup[STEXTT].n_value;
	for (xp = xtext; xp < &xtext[v.v_text]; xp++, loc += sizeof(*xp)) {
		if (vflg==0 && xp->x_iptr == NULL)
			continue;
		printf("%7x", loc);
		printf(" ");
		putf(xp->x_flag&XTRC, 'T');
		putf(xp->x_flag&XWRIT, 'W');
		putf(xp->x_flag&XLOAD, 'L');
		putf(xp->x_flag&XLOCK, 'K');
		putf(xp->x_flag&XWANT, 'w');
		printf("%5u", xp->x_daddr);
		printf("%7x", xp->x_caddr);
		printf("%5d", xp->x_size);
		printf("%8x", xp->x_iptr);
		printf("%4d", xp->x_count&0377);
		printf("%4d", xp->x_ccount);
		printf("\n");
	}
	free((caddr_t)xtext);
}

doproc()
{
#include <sys/proc.h>
	register struct proc *pp, *xproc;
	register loc, np;

	np = 0;

	if (v.v_proc <= 3 || v.v_proc >= 1000) {
		printf("System name list out of date\n");
		exit(1);
	}

	if ((xproc = (struct proc *)malloc(v.v_proc * sizeof(*pp))) == NULL) {
		printf("pstat:could not get %d bytes for proc table\n",
			v.v_proc * sizeof(*pp));
		return;
	}

	pseek(fc, (long)setup[SPROC].n_value, 0);
	read(fc, (char *)xproc, v.v_proc * sizeof(*pp));

	for (pp=xproc; pp < &xproc[v.v_proc]; pp++)
		if (pp->p_stat)
			np++;

	printf("%d out of %d processes active\n", np, v.v_proc);
	if (prcf == 0)
		return;

	printf("  LOC S  F  PRI  SIGNAL UID TIM CPU NI  PGRP   PID  PPID  ADDR SIZ WCHAN  LINK TEXTP  CLKT\n");
	loc = setup[SPROC].n_value;
	for (pp = xproc; pp < &xproc[v.v_proc]; pp++, loc += sizeof(*pp)) {
		if (vflg==0 && pp->p_stat==0 && allflg==0)
			continue;
		printf("%5x", loc);
		printf("%2d", pp->p_stat);
		printf("%3x", pp->p_flag);
		printf("%5d", pp->p_pri);
		printf("%8x", pp->p_sig);
		printf("%4d", pp->p_uid&0377);
		printf("%4d", pp->p_time&0377);
		printf("%4d", pp->p_cpu&0377);
		printf("%3d", pp->p_nice);
		printf("%6d", pp->p_pgrp);
		printf("%6d", pp->p_pid);
		printf("%6d", pp->p_ppid);
		printf("%6x", pp->p_addr);
		printf("%4d", pp->p_size);
		printf("%6x", pp->p_wchan);
		printf("%6x", pp->p_link);
		printf("%6x", pp->p_textp);
		printf(" %u", pp->p_clktim);
		printf("\n");
	}
	free((caddr_t)xproc);
}

dotty()
{
	struct tty tty[1], *ttyaddr, *loc;
	struct ttyptr tty_ptr[250];
	register struct ttyptr *tp;
	register char *mesg;

	ttyaddr = (struct tty *)setup[STTY].n_value;
	if (ttyaddr == 0)
		return;
	mesg = "  LOC RAW CAN OUT  PROC IFLAG OFLAG CFLAG LFLAG STATE  PGRP LN DEL COL ROW IX\n";
	printf(mesg);
	for (;;) {
		pseek(fc, ttyaddr, 0);
		ttyaddr = (struct tty *)((int)ttyaddr + sizeof(loc));
		if (read(fc, (char *)&loc, sizeof(ttyaddr)) != sizeof(ttyaddr))
			break;
		if (loc == 0)
			break;
		pseek(fc, (long)loc, 0);
		if (read(fc, (char *)tty_ptr, sizeof(tty_ptr)) != sizeof(tty_ptr))
			break;
		for (tp = tty_ptr; tp->tt_addr; tp++) {
			loc = tp->tt_tty;
			pseek(fc, (long)loc, 0);
			if (read(fc, (char *)tty, sizeof(tty)) != sizeof(tty))
				break;
			ttyprt(loc, &tty[0]);
		}
	}
}

ttyprt(loc, tp)
register struct tty *loc, *tp;
{
	printf("%5x", loc);
	printf("%4d", tp->t_rawq.c_cc);
	printf("%4d", tp->t_canq.c_cc);
	printf("%4d", tp->t_outq.c_cc);
	printf("%6x", tp->t_proc);
	printf("%6o", tp->t_iflag);
	printf("%6o", tp->t_oflag);
	printf("%6o", tp->t_cflag);
	printf("%6o", tp->t_lflag);
	printf("%6o", tp->t_state);
	printf("%6d", tp->t_pgrp);
	printf("%3d", tp->t_line);
	printf("%4d", tp->t_delct);
	printf("%4d", tp->t_col);
	printf("%4d", tp->t_row);
	printf("%3d", tp->t_index);
	printf("\n");
}

dousr()
{
#include <sys/dir.h>
#include <sys/signal.h>
#include <sys/user.h>
	union {
		struct	user rxu;
		char	fxu[4096];
	} xu;
	register struct user *up;
	register i;

	pseek(fc, (long)USTART, 0);
	read(fc, (char *)&xu, sizeof(xu));
	up = &xu.rxu;
	labprnt("rsav ", up->u_rsav);
	labprnt("ssav ", up->u_ssav);
	labprnt("qsav ", up->u_qsav);
	printf("segflg, error %d, %d\n", up->u_segflg, up->u_error);
	printf("uids %d,%d,%d,%d\n", up->u_uid,up->u_gid,up->u_ruid,up->u_rgid);
	printf("procp %.1o\n", up->u_procp);
	printf("base, count, offset %.1o %.1o %ld\n", up->u_base,
		up->u_count, up->u_offset);
	printf("cdir %.1o\n", up->u_cdir);
	printf("dirp %.1o\n", up->u_dirp);
	printf("dent %d %.14s\n", up->u_dent.d_ino, up->u_dent.d_name);
	printf("pdir %.1o\n", up->u_pdir);
	printf("dseg");
	for (i=0; i<10; i++)
		printf("%8.1o", up->u_ofile[i]);
	printf("\n    ");
	for (i=10; i<NOFILE; i++)
		printf("%8.1o", up->u_ofile[i]);
	printf("\nargs");
	for (i=0; i<5; i++)
		printf(" %.1o", up->u_arg[i]);
	printf("\nsizes %.1o %.1o %.1o\n", up->u_tsize, up->u_dsize, up->u_ssize);
	printf("sigs");
	for (i=0; i<NSIG; i++)
		printf(" %.1o", up->u_signal[i]);
	printf("\ntimes %ld %ld\n", up->u_utime/60, up->u_stime/60);
	printf("ctimes %ld %ld\n", up->u_cutime/60, up->u_cstime/60);
	printf("ar0 %.1o\n", up->u_ar0);
/*
	printf("prof");
	for (i=0; i<4; i++)
		printf(" %.1o", up->u_prof[i]);
*/
	printf("\nintflg %d\n", up->u_intflg);
	printf("ttyp %.1o\n", up->u_ttyp);
	printf("ttydev %d,%d\n", major(up->u_ttyd), minor(up->u_ttyd));
	printf("comm %.14s\n", up->u_comm);
}

labprnt(s, v)
char *s;
register int *v;
{
	register int i;

	printf(s);
	for (i=0; i < sizeof(label_t)/sizeof(int); i++)
		printf("%9x", v[i]);
	printf("\n");
}

xatoi(s)
register char *s;
{
	register v;

	v = 0;
	while (*s)
		v = (v<<4) + *s++ - '0';
	return(v);
}

dofil()
{
#include <sys/file.h>
	register struct file *fp, *xfile;
	register nf, loc;

	nf = 0;

	if (v.v_file <= 3 || v.v_file >= 1000) {
		printf("System name list out of date\n");
		exit(1);
	}

	if ((xfile = (struct file *)malloc(v.v_file * sizeof(*fp))) == NULL) {
		printf("pstat:could not get %d bytes for file table\n",
			v.v_file * sizeof(*fp));
		return;
	}
	pseek(fc, (long)setup[SFIL].n_value, 0);
	read(fc, (char *)xfile, v.v_file * sizeof(*fp));

	for (fp=xfile; fp < &xfile[v.v_file]; fp++)
		if (fp->f_count)
			nf++;

	printf("%d out of %d files active\n", nf, v.v_file);
	if (filf == 0)
		return;

	printf("  LOC FLG  CNT  INO      OFFS\n");
	loc = setup[SFIL].n_value;
	for (fp = xfile; fp < &xfile[v.v_file]; fp++,loc += sizeof(*fp)) {
		if (vflg==0 && fp->f_count==0)
			continue;
		printf("%5x ", loc);
		putf(fp->f_flag&FREAD, 'R');
		putf(fp->f_flag&FWRITE, 'W');
		printf("%4d", mask(fp->f_count));
		printf("%6x", fp->f_inode);
		printf("%10d\n", fp->f_offset);
	}
	free((caddr_t)xfile);
}

/* seek to proper place in core file adjusting for dumps */

pseek(file, offset, flag)
int file;
long offset;
int flag;
{
	if (dumpf) offset = offset - 0xE0000 + (24 * 512);
	lseek(file, offset, flag);
}


domem()
{
#include <sys/map.h>
	register struct map *mp, *xcoremap, *loc;
	long xmaxmem;
	long memfree = 0;

	if ((xcoremap = (struct map *)malloc(v.v_cmap * sizeof(*mp))) == NULL) {
		printf("pstat:could not get %d bytes for coremap table\n",
			v.v_cmap * sizeof(*mp));
		return;
	}

	pseek(fc, (long)setup[SMEMM].n_value, 0);
	read(fc, (char *)xcoremap, v.v_cmap * sizeof(*mp));

	pseek(fc, (long)setup[SMAXMEM].n_value, 0);
	read(fc, (char *)&xmaxmem, sizeof(xmaxmem));

	loc = (struct map *)setup[SMEMM].n_value;
	printf("%5s%5s%5s\n", "LOC", "ADDR", "SIZE");
	for (mp=xcoremap; mp < &xcoremap[v.v_cmap] && mp->m_size; mp++,loc++) {
		printf("%5x%5x%5d\n", loc, mp->m_addr, mp->m_size);
		memfree += mp->m_size;
	}
	printf("\nTotal memory = %dK, currently free = %dK\n",
		xmaxmem, memfree);
	free((caddr_t)xcoremap);
}

dosys()
{
	int cfreecount;
	printf("\n");

	pseek(fc, (long)setup[SCFREE].n_value, 0);
	read(fc, (char *)&cfreecount, sizeof cfreecount);
	printf("CLISTS:\t%d total, %d free, %d chars, %x wait address\n",
		v.v_clist, cfreecount/v.v_clsize, cfreecount,
		setup[SCWAIT].n_value);
}
