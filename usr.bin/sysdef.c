/*	@(#)sysdef.c	1.3	*/
#include	"stdio.h"
#include	"sys/var.h"
#include	"a.out.h"
#include	"sys/param.h"

#define	PAGESZ	512
#define	D7	0x38000000	/* From Unibus vector table in univec.c */
#define	USRMASK	0x3fffffff
#define	MAPSZ	4
#define	MAXI	200
#define	BUS	0340
#define	CC	0037
#define	BLOCK	010
#define	INTR	0	/* l_nptr[0] for interrupt routines */
#define	ADDR	1	/* l_nptr[1] for device address     */
#define	CNT	2	/* l_nptr[2] for device count       */
#define	STRAT	3	/* l_nptr[3] for strategy if blkdev */

#define	INTSZ		sizeof (int)
#define	SYM_VALUE(ptr)	(ptr->n_value & USRMASK)
#define	N_IFC		((start->n_value - IFC) / IFC_SZ)
#define	TXT		(HDR + aout.a_data)

struct	var	v;
#ifdef vax
struct sgs {
	struct filehdr filehdr;
	struct aouthdr aout;
	struct scnhdr scnhdr[3];
} sgs;
#define	HDR		sizeof (struct sgs)
#else
#ifdef m68000
struct  bhdr	aout;
#define	HDR		sizeof (struct bhdr)
#else m68000
struct	exec	aout;
#define	HDR		sizeof (struct exec)
#endif m68000
#endif
char	*os = "/unix";
char	*mr = "/etc/master";
char	line[256], nm[20], pre[20], intrnm[20],
	devaddr[20], devcnt[20], strat[20];
int	x, address, vsz, unit, bus, deft,
	count, typ, bcnt, maus, mcnt, smem, mesg, sem,
	root, swap, dump, n_swap, power, pipe;
long	swap_low;
int	IFC = 01000;	/* Start of Interface routines */
int	*vec, nexus;
int	bndry();
int	offset;
FILE	*system, *mast, *sysseek;

struct nlist	nl[MAXI], *nlptr, *setup(), *endnm, *start,
	*Messag, *Sema, *Maus, *bdevcnt, *bdevsw, *X25, *mbacnt,
	*rootdev, *swapdev, *dumpdev, *swplo, *nswap, *Shmem,
	*pipedev, *Power, *UNIvec, *umemvad, *Mbacf,
	*vs, *smap, *cmap, *callo, *restart;

struct	link {
	char	l_cfnm[8];	/* config name from master table */
	struct	nlist	*l_nptr;/* ptr to name list structure */
	int	l_vsz;		/* vector size */
	int	l_def;		/* default unit count */
	int	l_dtype;		/* device type - BLOCK */
	int	l_bus;		/* bus request from master table */
	char	l_used;		/* set when device entry is printed */
} ln[MAXI/3], *lnptr, *setln(), *endln, *search(), *majsrch();

struct	vector {
#if	vax || m68000
	int	vaxvec;
#endif
#ifdef	pdp11
	int	v_pc;
	int	v_psw;
#endif
} vector, *addr;

struct mbacf {
	int	nexus;
	int	bus_req;
	int	dev_addr;
} mbadev;

struct	bdev {
	int	b_open;
	int	b_close;
	int	b_strategy;
	int	b_tab;
} bent;

#ifdef	pdp11
struct	interface {	/* jsr r5, call; function */
	int	i_jsr;
	int	i_call;
	int	i_func;
} ifc[MAXI/3];

struct	xinterface {	/* jsr r5, call; jmp function */
	int	i_jsr;
	int	i_call;
	int	i_jmp;
	int	i_func;
} xifc[MAXI/3];

int	ovly = 0;	/* set to 1 if overlay system */
int	IFC_SZ = sizeof(ifc[0]);
#endif

struct x25info{
	int	x25_0_0;
	int	x25_0_1;
	int	x25links;
	int	x25bufs;
	int	x25bytes;
} x25info;

main(argc, argv)
	int	argc;
	char	**argv;
{

	register int i;

	switch(argc) {
	case 3:	mr = argv[2];
	case 2: os = argv[1];
	case 1: break;
	default:
		fprintf(stderr, "usage: sysdef  [ /unix [ /etc/master ] ]");
		exit(1);
	}

#ifdef	pdp11
	offset = (-HDR);
#endif

	if((system = fopen(os,"r")) == NULL) {
		fprintf(stderr,"cannot read %s\n",os);
		exit(1);
	}

/* read in a.out header */
#ifdef vax
	if(fread(&sgs, sizeof sgs, 1, system) != 1) {
#else
	if(fread(&aout, sizeof aout, 1, system) != 1) {
#endif
		printf("cannot read a.out header\n");
		exit(1);
	}
#ifdef vax
	if(sgs.aout.magic != 0410) {
#else
	if(BADMAG(aout)) {
#endif
		printf("invalid a.out format\n");
		exit(1);
	}
#ifdef	vax
	offset = PAGESZ - (sgs.aout.tsize % PAGESZ) - HDR;
#endif
#ifdef pdp11
	ovly = (aout.a_magic == A_MAGIC5);
	if (ovly) printf ("* PDP11 overlay system\n");
#endif
	if((sysseek = fopen(os,"r")) == NULL) {
		fprintf(stderr,"cannot read %s\n",os);
		exit(1);
	}
	if((mast = fopen(mr, "r")) == NULL) {
		fprintf(stderr, "cannot read %s\n", mr);
		exit(1);
	}

	nlptr = nl;
	lnptr = ln;
	start = setup("start");
	restart = setup("restart");
	UNIvec = setup("_UNIvec");
	Mbacf = setup("_mbacf");
	umemvad = setup("_umemvad");
	bdevcnt = setup("_bdevcnt");
	mbacnt = setup("_mba_cnt");
	bdevsw = setup("_bdevsw");
	rootdev = setup("_rootdev");
	pipedev = setup("_pipedev");
	swapdev = setup("_swapdev");
	swplo = setup("_swplo");
	nswap = setup("_nswap");
	dumpdev = setup("_dumpdev");
	Power = setup("_pwr_clr");
	Sema = setup("_seminfo");
	Messag = setup("_msginfo");
#ifdef vax
	Shmem = setup("_shminfo");
#endif
#ifdef pdp11
	Maus = setup("_mausmap");
#endif
	X25 = setup("_x25info");
	pre[0] = '_';

	while(fgets(line, 256, mast) != NULL) {
		if(line[0] == '*')
			continue;
		if(line[0] == '$')
			break;
		if (sscanf(line, "%s %d %d %o %s %d %o %d %d %d ",
			nm,&vsz,&x,&typ,&pre[1],&x,&x,&x,&deft,&bus,&x) != 10)
			continue;
		if(!strcmp(nm, "memory") || !strcmp(nm, "tty"))
			continue;
		strcpy(intrnm, pre);
		strcpy(devaddr, pre);
		strcpy(devcnt, pre);
		strcat(devaddr, "_addr");	/* _pre_addr */
		strcat(devcnt, "_cnt");	/* _pre_cnt */
		switch(vsz) {
			case 8: strcat(intrnm, "rint"); break;
			case 1: /* massbus devices */
			case 4: strcat(intrnm, "intr"); break;
			case 0: break; /* pseudo devices */
			default: continue;
		}
		setln(nm, setup(intrnm), vsz, deft, typ & BLOCK, bus);
		setup(devaddr);
		setup(devcnt);
		if(typ & BLOCK) {
			strcpy(strat, pre);
			strcat(strat, "strategy");
			setup(strat);
		}
	}
	endnm = setup("");
	endln = lnptr;
	nlist(os, nl);

#ifdef	DEBUG
/*
*	Print out the namelist
*/
	for(lnptr=ln; lnptr != endln; ++lnptr) {
		printf(NBPW==4 ? "%s\t%o\t%x\n" : "%s\t%o\t%o\n" ,
			(lnptr->l_nptr[INTR]).n_name,
			(lnptr->l_nptr[CNT]).n_type,
			(lnptr->l_nptr[CNT]).n_value);
		printf(NBPW==4 ? "%s\t%o\t%x\n" : "%s\t%o\t%o\n" ,
			(lnptr->l_nptr[ADDR]).n_name,
			(lnptr->l_nptr[ADDR]).n_type,
			(lnptr->l_nptr[ADDR]).n_value);
	}
#endif

	fseek(system, (long) SYM_VALUE(bdevcnt) - offset, 0);
	fread(&bcnt, INTSZ, 1, system);
	fseek(system, (long) SYM_VALUE(mbacnt) - offset, 0);
	fread(&mcnt, INTSZ, 1, system);
	fseek(system, (long) SYM_VALUE(rootdev) - offset, 0);
	fread(&root, INTSZ, 1, system);
	fseek(system, (long) SYM_VALUE(pipedev) - offset, 0);
	fread(&pipe, INTSZ, 1, system);
	fseek(system, (long) SYM_VALUE(swapdev) - offset, 0);
	fread(&swap, INTSZ, 1, system);
	fseek(system, (long) SYM_VALUE(dumpdev) - offset, 0);
	fread(&dump, INTSZ, 1, system);
	fseek(system, (long) SYM_VALUE(Power) - offset, 0);
	fread(&power, INTSZ, 1, system);
	fseek(system, (long) SYM_VALUE(Maus) - offset, 0);
	fread(&maus, INTSZ, 1, system);

#ifdef	vax
	vaxdevs();
#else
	pdpdevs();
#endif

	printf("*\n* Pseudo Devices\n*\n");
	pseudo_devs();

	/* rootdev, swapdev, pipedev, dumpdev, power */

	printf("*\n* System Devices\n*\n");

	if((lnptr = majsrch((root >> 8) & 0377)) == NULL)
		fprintf(stderr, "unknown root device\n");
	else
		printf("root\t%s\t%o\n", lnptr->l_cfnm, root & 0377);

	if((lnptr = majsrch((swap >> 8) & 0377)) == NULL)
		fprintf(stderr, "unknown swap device\n");
	else {
		printf("swap\t%s\t%o\t", lnptr->l_cfnm, swap & 0377);
		fseek(system, (long) SYM_VALUE(swplo) - offset, 0);
		fread(&swap_low, 4, 1, system);
		fseek(system, (long) SYM_VALUE(nswap) - offset, 0);
		fread(&n_swap, 2, 1, system);
		printf("%ld\t%d\n", swap_low, n_swap);
	}

	if((lnptr = majsrch((pipe >> 8) & 0377)) == NULL)
		fprintf(stderr, "unknown pipe device\n");
	else
		printf("pipe\t%s\t%o\n", lnptr->l_cfnm, pipe & 0377);

	if((lnptr = majsrch((dump >> 8) & 0377)) == NULL)
		fprintf(stderr, "unknown dump device\n");
	else
		printf("dump\t%s\t%o\n", lnptr->l_cfnm, dump & 0377);

	/* easy stuff */

	nlptr = nl;
	vs = setup("_v");
	smap = setup("_swapmap");
	callo = setup("_callout");
	endnm = setup("");
	nlist(os, nl);
	for(nlptr = vs; nlptr != endnm; nlptr++) {
#ifdef vax
		if(nlptr->n_value == 0) {
#else
		if(nlptr->n_type == 0) {
#endif
			fprintf(stderr, "namelist error\n");
			exit(1);
		}
	}
	fseek(system, (long) SYM_VALUE(vs) - offset, 0);
	fread(&v, sizeof v, 1, system);
	printf("*\n* Tunable Parameters\n*\n");
	printf("buffers\t%11d\n", v.v_buf);
	printf("calls\t%11d\n", v.v_call);
	printf("inodes\t%11d\n", v.v_inode);
	printf("files\t%11d\n", v.v_file);
	printf("mounts\t%11d\n", v.v_mount);
	printf("procs\t%11d\n", v.v_proc);
	printf("texts\t%11d\n", v.v_text);
	printf("clists\t%11d\n", v.v_clist);
	printf("sabufs\t%11d\n", v.v_sabuf);
	printf("maxproc\t%11d\n", v.v_maxup);
#ifdef	pdp11
	printf("coremap\t%11d\n", v.v_cmap);
#endif
	printf("swapmap\t%11d\n", v.v_smap);
	printf("hashbuf\t%11d\n", v.v_hbuf);
	printf("physbuf\t%11d\n", v.v_pbuf);
#ifdef	pdp11
	printf("iblocks\t%11d\n", v.v_iblk);
#endif
	if(X25->n_value != 0) {
		fseek(system, (long) SYM_VALUE(X25) - offset, 0);
		fread(&x25info, sizeof(x25info), 1, system);
		printf("x25links%11d\n", x25info.x25links);
		printf("x25bufs\t%11d\n", x25info.x25bufs);
		printf("x25bytes  (%d*1024)\n",(x25info.x25bytes/1024));
	}
	printf("power\t%11d\n", power ? 1 : 0);
	printf("mesg\t%11d\n", Messag->n_value ? 1 : 0);
	printf("sema\t%11d\n", Sema->n_value ? 1 : 0);
#ifdef vax
	printf("shmem\t%11d\n", Shmem->n_value ? 1 : 0);
#endif
#ifdef	pdp11
	printf("maus\t%11d\n", (maus != -1) ? 1 : 0);
#endif
	exit(0);
}

/*
 * setup - add an entry to a namelist structure array
 */
struct	nlist	*
setup(nam)
	char	*nam;
{
	if(nlptr >= &nl[MAXI]) {
		fprintf(stderr, "internal name list overflow\n");
		exit(1);
	}
	strncpy(nlptr->n_name, nam, 8);
	nlptr->n_type = 0;
	nlptr->n_value = 0;
	return(nlptr++);
}

/*
 * setln - set up internal link structure for later
 * function look-up.  Records useful information from the
 * /etc/master table description.
 */
struct	link	*
setln(cf, np, vz, dft, dt, br)
	char	*cf;
	struct	nlist	*np;
	int	vz;
	int	dft;
	int	dt;
	int	br;
{
	if(lnptr >= &ln[MAXI/3]) {
		fprintf(stderr, "internal link array overflow\n");
		exit(1);
	}
	strcpy(lnptr->l_cfnm, cf);
	lnptr->l_nptr = np;
	lnptr->l_vsz = vz;
	lnptr->l_def = dft;
	lnptr->l_dtype = dt;
	lnptr->l_bus = br;
	return(lnptr++);
}

/*
 * search - search for a function indirectly through
 * the link structure.
 */
struct	link	*
search(vctr,sub)
#ifdef	vax
	int	vctr;
#else
	struct	interface	*vctr;
#endif
	int	sub;
{
	register struct	link	*lnkptr;
#ifdef pdp11
	struct xinterface xi;
	long	off;
#endif

	for(lnkptr = ln; lnkptr != endln; lnkptr++) {
#ifdef	vax
#ifdef	DEBUG
printf("link val=%x vctr=%x\n",(lnkptr->l_nptr[sub]).n_value, (vctr & ~D7));
#endif
		if((lnkptr->l_nptr[sub]).n_value == (vctr & ~D7)) {
#else
#ifdef	DEBUG
printf("link val=%o vctr=%o\n",(lnkptr->l_nptr[sub]).n_value, vctr->i_func);
#endif
		if((lnkptr->l_nptr[sub]).n_value == (unsigned)vctr->i_func) {
#endif
			lnkptr->l_used = (char)1;
			return(lnkptr);
		}
	}
#ifdef pdp11
       /*
	* if overlay-loaded system, use interface table address to follow
	* chain into overlay segments and give search a second chance.
	*/
	if (ovly) {
#ifdef DEBUG
		printf("in overlay search\n");
#endif
		off = (unsigned) vctr->i_func;
		fseek(system, off + TXT, 0);
		fread(&xi, sizeof(xi), 1, system);
		for(lnkptr = ln; lnkptr != endln; lnkptr++) {
#ifdef	DEBUG
printf("link val=%o vctr=%o\n",(lnkptr->l_nptr[sub]).n_value, xi.i_func);
#endif
			if((lnkptr->l_nptr[sub]).n_value == (unsigned)xi.i_func-4) {
				lnkptr->l_used = (char)1;
				return(lnkptr);
			}
		}
	}
#endif
	return(NULL);
}

/*
 * majsrch - search for a link structure given the major
 * device number of the device in question.
 */
struct	link *
majsrch(maj)
	int	maj;
{
	register struct	link	*lnkptr;
#ifdef pdp11
	struct xinterface	xi;
	long	off;
#endif

	if(maj > bcnt - 1)
		return(NULL);

	fseek(system,(long)SYM_VALUE(bdevsw)+(maj*sizeof(bent))-offset,0);
	fread(&bent, sizeof bent, 1, system);

	for(lnkptr = ln; lnkptr != endln; lnkptr++)
		if(lnkptr->l_dtype) {
#ifdef	DEBUG
#ifdef	pdp11
printf("link val=%o bent=%o\n",(lnkptr->l_nptr[STRAT]).n_value, bent.b_strategy);
#endif
#ifdef	vax
printf("link val=%x vctr=%x\n",(lnkptr->l_nptr[STRAT]).n_value, bent.b_strategy);
#endif
#endif
			if((lnkptr->l_nptr[STRAT]).n_value == bent.b_strategy)
				return(lnkptr);
		}
#ifdef pdp11
       /*
	* if overlay-loaded system, use interface table address to follow
	* chain into overlay segments and give search a second chance.
	*/
	if (ovly) {
#ifdef DEBUG
		printf("in overlay search\n");
#endif
		off = (unsigned) bent.b_strategy;
		fseek(system, off + TXT, 0);
		fread(&xi, sizeof(xi), 1, system);
		for(lnkptr = ln; lnkptr != endln; lnkptr++) {
#ifdef	DEBUG
printf("link val=%o vctr=%o\n",(lnkptr->l_nptr[STRAT]).n_value, xi.i_func);
#endif
			if((lnkptr->l_nptr[STRAT]).n_value == (unsigned)xi.i_func-4) {
				lnkptr->l_used = (char)1;
				return(lnkptr);
			}
		}
	}
#endif
	return(NULL);
}

#ifdef vax
vaxdevs()
{
	register int i;
	if(UNIvec->n_value == 0) {
		fprintf(stderr, "%s %s\n", "symbol \"UNIvec\" undefined; ",
			"invalid /unix file");
		exit(1);
	}
	if(Mbacf->n_scnum == 0) {
		fprintf(stderr, "%s %s\n", "symbol \"_mbacf\" undefined; ",
			"invalid /unix file");
		exit(1);
	}

	printf("*\n* MASSBUS\n* dev\tnexus\taddr\tbus\n*\n");

/* MASSBUS devices from _mbacf */
	if(fseek(system, (long)SYM_VALUE(Mbacf) - offset, 0) != NULL) {
		fprintf(stderr, "sysdef: seek failed on %s\n",system);
		exit(1);
	}

	for(i=0; i < mcnt; ++i) {
		fread(&mbadev, sizeof mbadev, 1, system);
		if(mbadev.nexus == NULL) continue;
		if((lnptr = search(mbadev.dev_addr,ADDR)) == NULL) {
			fprintf(stderr,
				"unknown device at NEXUS %3d\n", mbadev.nexus);
			continue;
		}

		printf("%s\t%3d\t%d\t%d",
			lnptr->l_cfnm,mbadev.nexus,0,mbadev.bus_req);
		printf("\n");
	}

	if(fseek(system, (long)SYM_VALUE(UNIvec) - offset, 0) != NULL) {
		fprintf(stderr, "sysdef: seek failed on %s\n",system);
		exit(1);
	}

/* on a VAX the following works only for UNIBUS devices */

	printf("*\n* UNIBUS\n* dev\tvector\taddr\tbus\tcount\n*\n");

	for(vec=0; vec < (int *)(INTSZ * 128); ++vec) {
		fread(&vector, sizeof vector, 1, system);
		if(vector.vaxvec == NULL) continue;
		if((lnptr = search(vector.vaxvec,INTR)) == NULL) {
			fprintf(stderr,"unknown device at vector %3o\n", vec);
			continue;
		}
		printf("%s\t%3o", lnptr->l_cfnm, vec);
		unit = (vector.vaxvec & D7) >> 27;
		if(fseek(sysseek,
			(long)(((lnptr->l_nptr[ADDR]).n_value & USRMASK)
			- ftell(sysseek) + (INTSZ * unit) - offset), 1) != NULL) {
			fprintf(stderr,"bad seek for device address\n");
		}
		if(fread(&address, sizeof(address), 1, sysseek) != 1) {
			fprintf(stderr,"cannot read dev_addr\n");
		}

		printf("\t%lo\t%1o",(address), lnptr->l_bus);

		fseek(sysseek, (long)(lnptr->l_nptr[CNT].n_value & USRMASK)
			- ftell(sysseek) - offset, 1);
		fread(&count, INTSZ, 1, sysseek);
		if((unit + 1) * lnptr->l_def > count)
			printf("\t%d", count % lnptr->l_def);
		else
			printf("\t%d", lnptr->l_def);
		printf("\n");

		if(lnptr->l_vsz == 8) {
			vec++;
			fread(&vector, sizeof vector, 1, system);
		}
	}
}
#endif

#ifdef	pdp11
pdpdevs()
{
	register int i;
	if(start->n_type == 0) {
		fprintf(stderr, "%s %s\n",
			"symbol \"start\" undefined; ",
			"invalid /unix file");
		exit(1);
	}
	if(restart->n_value) {
		IFC = 01020;
		IFC_SZ = sizeof(xifc[0]);
		if((start->n_value - IFC) > (MAXI / 3 * (sizeof ifc[0]))) {
			printf("internal interface array overflow\n");
			exit(1);
		}
		fseek(system, (long) IFC + TXT, 0);
		fread(xifc, sizeof xifc[0], N_IFC, system);
		for(i=0; i < N_IFC; ++i) {
			ifc[i].i_jsr = xifc[i].i_jsr;
			ifc[i].i_call = xifc[i].i_call;
			ifc[i].i_func = (xifc[i].i_func + IFC + 
				( i + 1) * 8);
		}
	}
	else {
		if((start->n_value - IFC) > (MAXI / 3 * (sizeof ifc[0]))) {
			fprintf(stderr, "internal interface array overflow\n");
			exit(1);
		}
		fseek(system, (long) IFC - offset, 0);
		fread(ifc, sizeof ifc[0], (start->n_value - IFC) /
			sizeof ifc[0], system);
	}

	printf("*\n* dev\tvector\taddr\tbus\tcount\n*\n");
	for(addr = (struct vector *)0; addr != (struct vector *)01000; addr++) {
		if(addr == (struct vector *)060)
			addr = (struct vector *)070;	/* skips console */
		fseek(system, (long)addr - offset, 0);
		fread(&vector, sizeof vector, 1, system);
		if((vector.v_pc <= IFC) || (vector.v_pc >= start->n_value))
			continue;	/* skips clio, traps, jmp, etc */
		if((lnptr = search(&ifc[(vector.v_pc - IFC) / IFC_SZ],INTR))
			== NULL) {
			fprintf(stderr,
				"unknown device interrupts at vector %3o\n",
				addr);
			continue;
		}
		printf("%s\t%3o", lnptr->l_cfnm, addr);
		unit = vector.v_psw & CC;
		bus = (vector.v_psw & BUS) >> 5;
		fseek(system,(long)(lnptr->l_nptr[ADDR].n_value+2*unit-offset),0);
		fread(&address, 2, 1, system);
		printf("\t%6o\t%1o", address, bus);
		fseek(system, (long)(lnptr->l_nptr[CNT].n_value - offset), 0);
		fread(&count, NBPW, 1, system);
		if((unit + 1) * lnptr->l_def > count)
			printf("\t%d", count % lnptr->l_def);
		else
			printf("\t%d", lnptr->l_def);
		printf("\n");
		if(lnptr->l_vsz == 8)
			addr++;
	}
}
#endif

pseudo_devs()
{
	register struct link	*lnkptr;
	int			count;

	for(lnkptr = ln; lnkptr != endln; lnkptr++) {
		if( ! (lnkptr->l_nptr[CNT]).n_value) {  /* dev_cnt undefined*/
			continue;
		}
		if(lnkptr->l_used) {	/*has already been printed*/
			continue;
		}
		if(strcmp(lnkptr->l_cfnm,"dl11") == 0) { /* /dev/console */
			continue;
		}

		fseek(system, (long)
			((lnkptr->l_nptr[CNT]).n_value & USRMASK) - offset, 0);
		fread(&count, INTSZ, 1, system);
		printf("%s\t%d\t%d\t%d\t%d\n",lnkptr->l_cfnm,0,0,0,count);
	}
}
