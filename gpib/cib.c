#
/*
 * cib.c --
 * configuration and test program
 * for ib driver.
 */


# include "cib.defs"

# include "stdio.h"
# include "ctype.h"

# include "sys/stat.h"


char *progname = "cib";


union
{
    struct poke p1;
    struct ioreq io1;
    struct tlist t1;
    struct sgnode sg1;
    char pollstats[MAXIBSLOTS];
}	j;

int f;
int FD;
int MINOR;
int junk;

int unitno;
char *field;

char *lsetargs = "nodeno addr flags ppr TLCEI tslot lmap";
char *usage = "usage:  cib [-I [arg arg]] < ibfile";

main(rgc,rgv)
    char **rgv;
    int rgc;
{
# ifdef notdef
    extern char *basename();
# endif notdef
    extern char *mapcode();

    int iii,lf;
    register char *ap,*cp;
    struct stat stat1;

    /*derive unit number from progname*/
    if( --rgc < 0 )
	errexit("arg cnt");
    progname = *rgv++;
    {
	FD = 0;
	fstat(FD,&stat1);
	unitno = minor(stat1.st_rdev);
    }

    if( rgc <= 0 )
    {
	readregs(FD);
	exit(0);
    }

    while( rgc > 0 )
    {
	ap = *rgv++; rgc--;
	if( *ap == '-' )
	    ap++;
	else
	    ap = mapcode(ap);
	while(*ap!=000)
	{
	    if( lf = isupper(*ap) )
		*ap = tolower(*ap);
	switch(iii=ibiocode(*ap++))
	{
	case IBIOSTART:
	    if( --rgc < 0 )
		argexit(iii,"V");
	    junk = atoci(*rgv++);
	    gioctl(FD,iii,&junk);
	    break;

	case IBIOGETNODE:
	    if( --rgc < 0 )
		argexit(iii,"nodeno");
	    iii = atoci(*rgv++);
	    getnode(FD,iii,&j.sg1);
	    printnode(iii,&j.sg1.node);
	    break;

	case IBIOSETNODE:
	    if( lf )
	    {
		if( (rgc -= 8) < 0 )
		    argexit(iii,lsetargs);
		j.sg1.slotno = atoci(*rgv++);
		j.sg1.node.n_tag1 = atoci(*rgv++);
		j.sg1.node.n_flags = flagex(*rgv++,nflags);
		j.sg1.node.n_ppr = atoci(*rgv++);
		if( strlen(cp) != 5 )
		    argexit(iii,lsetargs);
		cp = *rgv++;
		j.sg1.node.n_talkresp = *cp++;
		j.sg1.node.n_lstnresp = *cp++;
		j.sg1.node.n_tctlresp = *cp++;
		j.sg1.node.n_erroresp = *cp++;
		j.sg1.node.n_idleresp = *cp++;
		j.sg1.node.n_talkslot = atoci(*rgv++);
		j.sg1.node.n_lstnmap = atoci(*rgv++);
		gioctl(FD,iii,&j.sg1);
		break;
	    }

	    if( (rgc -= 3) < 0 )
		argexit(iii,"nodeno addr flags");
	    iii = atoci(*rgv++);
	    getnode(FD,iii,&j.sg1);
	    j.sg1.node.n_tag1 = atoci(*rgv++);
	    j.sg1.slotno = iii;
	    j.sg1.node.n_flags = flagex(*rgv++,nflags);
	    gioctl(FD,IBIOSETNODE,&j.sg1);
	    break;

	case IBIOPOLL:
	    gioctl(FD,iii,j.pollstats);
	    prdata(j.pollstats,MAXIBSLOTS,stdout);
	    break;

	case IBIOSRQ:
	    if( --rgc < 0 )
		argexit(iii,"code");
	    junk = atoci(*rgv++);
	    gioctl(FD,iii,&junk);
	    break;

	case IBIOREN:
	case IBIOGTL:
	case IBIOPPU:
	case IBIOPPC:
	    if( --rgc < 0 )
		argexit(iii,"map");
	    junk = atoci(*rgv++);
	    gioctl(FD,iii,&junk);
	    break;

	case IBIOCUTOFF:
	case IBIOFLUSH:
	case IBIOINIT:
	case IBIOTAKECTL:
	case IBIOINTR:
	    gioctl(FD,iii,0);
	    break;

	case IBIOPASSCTL:
	    if( --rgc < 0 )
		argexit(iii,"nodeno");
	    junk = atoci(*rgv++);
	    gioctl(FD,iii,&junk);
	    break;

	case IBIOLOCK:
	    if( --rgc < 0 )
		argexit(iii,"V");
	    junk = atoci(*rgv++);
	    gioctl(FD,iii,&junk);
	    break;

	case IBIOPEEK:
	    if( --rgc < 0 )
		argexit(iii,"field");
	    if( valof(field = *rgv++,fields,&f) < 0 )
		errexit("unknown field %s",field);
	    printf("%03o %s == 0x%02x\n"
		    ,unitno,field,inreg(f));
	    break;

	case IBIOPOKE:
	    if( (rgc -= 2) < 0 )
		argexit(iii,"field val");
	    if( valof(field = *rgv++,fields,&f) < 0 )
		errexit("unknown field %s",field);
	    printf("%03o %s <- 0x%02x\n"
		    ,unitno,field,outreg(f,atoci(*rgv++)));
	    break;

	case IBIODEBUG:
	    if( --rgc < 0 )
		argexit(iii,"subno");
	    junk = atoci(*rgv++);
	    gioctl(FD,iii,&junk);
	    break;

# ifdef notdef
	case IBIOGETEV:
	case IBIOLISTEN:
	case IBIOTALK:
	    errexit("%c not implemented",ap[-1]);
	    break;

	case IBIOGETCONN:
	    if( --rgc < 0 )
		argexit(iii,"conno");
	    iii = atoci(*rgv++);
	    getconn(FD,iii,&j.t1);
	    printconn(&j.t1);
	    break;

	case IBIOSETCONN:
	    if( (rgc -= 2) < 0 )
		argexit(iii,"tslot lstnmap");
	    iii = atoci(*rgv++);
	    j.t1.talkslot = iii;
	    j.t1.listenmap = atoci(*rgv++);
	    gioctl(FD,IBIOSETCONN,&j.t1);
	    break;
# endif notdef

	default:
	    errexit("unknown flag %c",ap[-1]);
	    break;
	}
	}
    }

    if( rgc > 0 )
	errexit(usage);
    exit(0);
}

int inreg(f)
    int f;
{
    char ibregs1[20];
    int junk;
    junk = f;
    gioctl(FD,IBIOPEEK,&junk);
    return junk;
}

int outreg(f,v)
    int f;
    int v;
{
    static struct poke poke1;
    poke1.f = f;
    poke1.v = (int)v;
    if( ioctl(FD,IBIOPOKE,&poke1) < 0 )
	return-1;
    return (int)v;
}

readregs(fd)
    int fd;
{
    char ibregs1[NIBIREGS];
    int iii,jjj;

    for( iii = 0; iii < NIBIREGS; iii++ )
    {
	jjj = iii;
	ioctl(fd,IBIOPEEK,&jjj);
	ibregs1[iii] = jjj;
    }
    prdata(ibregs1,NIBIREGS,stdout);
    fflush(stdout);
}

getnode(fd,n,sp)
    int fd;
    int n;
    struct sgnode *sp;
{
    sp->slotno = n;
    gioctl(fd,IBIOGETNODE,sp);
}

printnode(n,np)
    int n;
    struct ibnode *np;
{
    extern char *prc();

    printf("slot%d:  addr %03o.%03o, flags (0x%x) "
	    ,n
	    ,np->n_tag1,np->n_tag2,np->n_flags);
    pflags(np->n_flags,nflags);
    nn();
    printf(" ppr 0x%x,",np->n_ppr);
    printf(" pollstat 0x%x,",np->n_pollstat);
    printf("  TLCEI %s",prc(np->n_talkresp));
    printf("%s",prc(np->n_lstnresp));
    printf("%s",prc(np->n_tctlresp));
    printf("%s",prc(np->n_erroresp));
    printf("%s",prc(np->n_idleresp));
    printf(", tslot %d, lmap 0%o\n",np->n_talkslot,np->n_lstnmap);
    fflush(stdout);
}

argexit(code,str)
    int code;
    char *str;
{
    char *codesym;
    if( symof(code,iocodes,&codesym) < 0 )
	codesym = "???";
    errexit("missing/bad arg to %s (-%c %s)"
	    ,codesym,code&0xFF,str);
}

getconn(fd,n,tp)
    int fd;
    int n;
    struct tlist *tp;
{
    gioctl(fd,IBIOGETCONN,tp);
}

int
gioctl(fd,cmd,ptr)
    int fd;
    int cmd;
    char *ptr;
{
    if( ioctl(fd,cmd,ptr) < 0 )
	scerrwarn("failed ioctl");
}

printconn(tp)
    struct tlist *tp;
{
    printf("tslot %d, lmap 0x%x\n"
	    ,tp->talkslot,tp->listenmap);
}

char *prc(c)
    int c;
{
    static char bertha[5];
    register char *cp;
    cp = bertha;
    c &= BYTEMASK;
    if( isprint(c) && c != '\\' )
    {
	*cp++ = c;
    }
    else
    {
	*cp++ = '\\';
	*cp++ = ((c>>6)&07) + '0';
	*cp++ = ((c>>3)&07) + '0';
	*cp++ = ((c>>0)&07) + '0';
    }
    *cp++ = 000;
    return bertha;
}

char *mapcode(str)
    char *str;
{
    static char bertha[3];
    char upper;
    int iii;
    upper = isupper(*str);
    if( valof(str,iocodes,&iii) < 0 )
	errexit("unknown code %s",str);
    bertha[0] = iii&BYTEMASK;
    if( upper && islower(bertha[0]) )
	bertha[0] = toupper(bertha[0]);
    return bertha;
}
