# include "cib.defs"
/*
 * dib.c --
 * dump kernel data structures
 * pertaining to the ib driver.
 */

# include "stdio.h"
# include "ctype.h"
# include "nlist.h"

char *progname = "dib";


struct nlist uvars[] =
{
# define K_IBCONN	((caddr_t)uvars[0].n_value)
	{"_ibconn"},
# define K_IBVARS	((caddr_t)uvars[1].n_value)
	{"_ibvars"},
# define K_IBNGETEBLK	((caddr_t)uvars[2].n_value)
	{"_IBNGETE"},
# define K_IBNBRELSE	((caddr_t)uvars[3].n_value)
	{"_IBNBREL"},
#
	{0}
};

int unitno = 0;
char lflg = 0;
char cflg = 0;
char vflg = 0;
char dflg = 0;
char fflg = 0;
int memfd = -1;
char *mem = "/dev/kmem";
char *unic = "/vmunix";

char *usage = "usage:  dib [-cdfklnuv ...]";

main(rgc,rgv)
    int rgc;
    char **rgv;
{
    register char *ap;

    if( --rgc < 0 )
	errexit("arg cnt");

    progname = *rgv++;

    while( rgc > 0 && *(ap = *rgv) == '-' )
    {
	rgc--; rgv++; ap++;
	while(*ap != 000)
	switch(*ap++)
	{
	case 'c':
	    cflg++;
	    break;
	case 'd':
	    dflg ++;
	    break;
	case 'f':
	    fflg++;
	    break;
	case 'k':
	    if( --rgc < 0 )
		errexit("missing -k {mem}");
	    mem = *rgv++;
	    break;
	case 'l':
	    lflg++;
	    break;
	case 'n':
	    if( --rgc < 0 )
		errexit("missing -n {unix}");
	    unic = *rgv++;
	    break;
	case 'u':
	    if( --rgc < 0 )
		errexit("missing -u {unit}");
	    unitno = atoci(*rgv++);
	    break;
	case 'v':
	    vflg++;
	    break;
	default:
	    errexit("unknown flag %c",ap[-1]);
	    break;
	}
    }

    if( (unsigned)unitno >= NIB )
	errexit("illegal unit %d (max %d)\n",unitno,NIB);

    if( rgc > 0 )
	errexit(usage);

    if( !(cflg || dflg || fflg || vflg) )
	cflg++ , fflg++ , vflg++;

# ifdef notdef
    if( dflg )
	printoff();
# endif notdef

    if( cflg || fflg || vflg )
    if( gnlist(unic,uvars) < 0 )
	errexit("bad nlist %s",unic);

    if( cflg )
	dumpconn();

    if( vflg )
	dumpvars();

    if( fflg )
	dumpfree();

    exit(0);
}

struct ibvars ib1;
struct ibconn ibconn1;
struct buf buf1;
struct bufq bq1;
int junk1;

dumpconn()
{
    int iii;
    caddr_t kaddr;

    kaddr = K_IBCONN;
    kaddr += unitno*NIBCONN*sizeof (struct ibconn);
    for( iii = 0; iii < NIBCONN; iii++ )
    {
	printf("ibconn 0x%03x:  ",iii);
	kread(kaddr,(char*)&ibconn1,sizeof ibconn1);
	dump1conn(&ibconn1);
	kaddr += sizeof (struct ibconn);
    }
}

dump1conn(cp)
    register struct ibconn *cp;
{
    printf(" .c_dev 0x%03x",cp->c_dev);
    printf(" .c_Tslot %d",cp->c_Tslot);
    printf(" .c_Lmap 0%05o",cp->c_Lmap);
    nn();
    printf(" .c_flags (0x%x) ",cp->c_flags);
    pflags(cp->c_flags,connflags);
    nn();
    printf(" .c_rq:");
    dumpq(&cp->c_rq);
}

dumpvars()
{
    register caddr_t kaddr;

    kaddr = K_IBVARS + unitno*sizeof (struct ibvars);
    kread(kaddr,(char*)&ib1,sizeof ib1);
    printf("ib%d:\n",unitno);
    dump1vars(&ib1);
}

dump1vars(vp)
    register struct ibvars *vp;
{
    dumptlc(RP);
    printf(" .nconn %d, .nrawconn %d"
	    ,vp->nconn,vp->nrawconn);
    nn();
    printf(" .udev 0x%x, .use (%d) "
	    ,vp->udev,vp->use);
    pflags(vp->use,uflags);
    nn();
    printf(" .ibflags (0x%x) ",vp->ibflags);
    pflags(vp->ibflags,ibflags);
    nn();
    printf(" .curbuf 0x%x",vp->curbuf);
    nn();
    printf(" .ioq:");
    dumpq(&vp->ioq);
    printf(" .nbufs %d, .nfreebufs %d, .maxfreebufs %d,"
	    ,vp->nbufs,vp->nfreebufs,vp->maxfreebufs
		);
    nn();
    printf(" .raq:");
    dumpq(&vp->raq);
}

dumptlc(rp)
    register struct tlc *rp;
{
    printf(" .unit %d,",rp->unit);
    printf(" .base 0x%x,",rp->base);
    printf(" .iTslot %d, .iLmap 0%o,",rp->iTslot,rp->iLmap);
    nn();
    printf(" .Cflags (0x%x) ",rp->Cflags);
    pflags(rp->Cflags,Cflags);
    nn();
    printf(" .csr (0x%x) ",rp->csr);
    pflags(rp->csr,csrflags);
    nn();
    printf(" .events (0x%x) ",rp->events);
    pflags(rp->events,csrflags);
    nn();
    printf(" .cmdalarm 0x%x, .retryalarm 0x%x"
	,rp->cmdalarm,rp->retryalarm);
    nn();
    printf(" .atclkint %d, .atclk %d"
	    ,rp->atclkint,rp->atclk);
    nn();
    printf(" .atdone %d, .atdoneintr %d,"
	    ,rp->atdone,rp->atdoneintr);
    printf(" .atgonged %d"
	    ,rp->atgonged);
    nn();
    dumpsteps(rp);
}

dumpsteps(rp)
    register struct tlc *rp;
{
    register struct step *xp;
    caddr_t krp;
    int nstep;

    krp = K_IBVARS + rp->unit*sizeof (struct ibvars);
    if( rp->atac == 0 )
	nstep = 0;
    else
	nstep = ( (long)rp->atac - (long)((struct tlc*)krp)->steps )
		/ sizeof*xp;
    xp = rp->steps;
    printf(" .steps %d/%d",rp->atcnt,nstep);
    nn();
    while( nstep > 0 )
    {
	printf("%c",nstep==rp->atcnt?'*':' ');
	printf("[$%08x|$%08x]",xp->cmask,xp->cval);
	switch(xp->rreg&CODEMASK)
	{
	case OUTREG_CODE:
	    printf(" $%x <- $%02x",xp->rreg&REGMASK,xp->rval);
	    break;
	default:
	case INREG_CODE:
	    printf(" $%x :  $%02x",xp->rreg&REGMASK,rp->dir);
	    break;
	}
	printf(" [$%08x]",xp->pmask);
	nn();
	nstep--;
	xp++;
    }
}

dumpfree()
{
    caddr_t kaddr;
    int iii;
    kread(K_IBNGETEBLK,&junk1,sizeof junk1);
    printf(" IBNGETEBLK %d,",junk1);
    kread(K_IBNBRELSE,&junk1,sizeof junk1);
    printf(" IBNBRELSE %d",junk1);
    nn();
    for( iii = 0; iii < NIB; iii++ )
    {
	kaddr = K_IBVARS + iii*sizeof (struct ibvars);
	kread(kaddr,(char*)&ib1,sizeof ib1);
	printf("%2d:  ",iii);
	printf(" .nbufs %d, .nfreebufs %d, .maxfreebufs %d"
		,ib1.nbufs,ib1.nfreebufs,ib1.maxfreebufs);
	nn();
	printf(" .freeq:");
	dumpq(&ib1.freeq);
    }
}

dumpq(qp)
    register struct bufq *qp;
{
    register caddr_t bp;
    caddr_t obp;
    printf(" .bq_head 0x%x, .bq_tail 0x%x,"
	    ,qp->bq_head,qp->bq_tail);
    printf(" .bq_flags (0x%x) "
	    ,qp->bq_flags);
    pflags(qp->bq_flags,bqflags);
    nn();
    bp = (caddr_t)qp->bq_tail;
    obp = 0;
    while( bp != 0 )
    {
	if( bad(bp) )
	{
	    printf(" (bad bufptr 0x%x)",bp);
	    break;
	}
	printf(" (0x%x: ",bp);
	kread(bp,(char*)&buf1,sizeof buf1);
	dumpbuf(&buf1);
	printf(")");
	if( (caddr_t)buf1.b_tail != obp )
	    printf(" (bad tailptr 0x%x)",buf1.b_tail);
	if( (buf1.b_head == 0 && bp != (caddr_t)qp->bq_head) )
	    printf(" (bad headptr 0x%x)",buf1.b_head);
	obp = bp;
	nn();
	bp = (caddr_t)buf1.b_head;
    }
}

dumpbuf(bp)
    register struct buf *bp;
{
    printf(" .DEV 0x%x",DEV(bp));
    printf(" .b_use (%d) ",USE(bp));
    pflags(USE(bp),uflags);
    printf(" .b_flags (0x%x) ",bp->b_flags);
    pflags(bp->b_flags,bqflags);
}

int kread(addr,buf,cnt)
    caddr_t addr;
    char *buf;
    int cnt;
{
    if( memfd < 0 )
    {
	if( (memfd = open(mem,0)) < 0 )
	    scerrexit("can't open %s",mem);
    }
    lseek(memfd,(off_t)addr,0);
    if( read(memfd,buf,cnt) != cnt )
	scerrexit("mem read err @0x%x",addr);
    return cnt;
}

int bad(p)
    caddr_t p;
{
    return (int)p&01 || p==0;
}

int gnlist(unic,vars)
    char *unic;
    struct nlist *vars;
{
    register struct nlist *np;
    char botch;

    nlist(unic,vars);

    botch = 0;
    for( np = vars; np->n_name != 0 && *np->n_name != 000; np++ )
	if( np->n_type == -1 || np->n_value == 0 )
	{
	    botch--;
	    errwarn("kvar %s not found",np->n_name);
	}
    return botch;
}

# ifdef notdef
#
# define OFFSET(t,f)	((char*)&(t 0)->f)
# define SIZE(t,f)	(sizeof (t 0)->f)
# define PRINT(t,f)	pfield1("t","f",OFFSET(t,f),SIZE(t,f))
# define PRINTSIZE(t)	printf("0x%03x %s\n","t",sizeof t)

pfield1(t,f,o,s)
    char *t,*f;
    int o,s;
{
    printf("+0x%03x %s.%s (0x%x)\n",s,t,f,o);
}

# endif notdef
