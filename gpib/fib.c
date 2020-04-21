# include "cib.defs"
/*
 * fib.c --
 * symbolic data for debugging ib driver.
 */

# include "stdio.h"
# include "ctype.h"

# define FLGINIT(x)	{x,x,"x"}
# define FIELDINIT(x)	{0xFFFF,x,"x"}

struct flagsym bqflags[] =
{
	FLGINIT(BQ_MINE),
	FLGINIT(BQ_ONQ),
	FLGINIT(BQ_START),
	FLGINIT(BQ_END),
	FLGINIT(BQ_DONE),
	FLGINIT(BQ_WANTED),
	FLGINIT(BQ_ERROR),
	FLGINIT(BQ_ABORT),
	FLGINIT(BQ_QLSTN),
	FLGINIT(BQ_ASYNC),
	FLGINIT(BQ_PHYS),
	{0,0,0}
};

struct flagsym nflags[] =
{
	FLGINIT(IBN_VALID),
	FLGINIT(IBN_SC),
	FLGINIT(IBN_SWAB),
	FLGINIT(IBN_PPE),
	FLGINIT(IBN_PPC),
	FLGINIT(IBN_SRQ),
	{0,0,0}
};

struct flagsym connflags[] =
{
	FLGINIT(OPENF),
	FLGINIT(BUSYF),
	FLGINIT(WAITF),
	FLGINIT(COOKEDF),
	FLGINIT(RAWF),
	{0,0,0}
};

struct flagsym uflags[] =
{
	FIELDINIT(LSTNU),
	FIELDINIT(TALKU),
	FIELDINIT(ABORTU),
	FIELDINIT(RESETU),
	FIELDINIT(INITU),
	FIELDINIT(TRIGU),
	FIELDINIT(CONNECTU),
	FIELDINIT(PEEKU),
	FIELDINIT(POKEU),
	FIELDINIT(SPOLLU),
	FIELDINIT(PPOLLU),
	FIELDINIT(DCLRU),
	FIELDINIT(PASSCTLU),
	FIELDINIT(TAKECTLU),
	FIELDINIT(PPEU),
	FIELDINIT(SRQU),
	FIELDINIT(IFCU),
	FIELDINIT(IDLEU),
	FIELDINIT(PPDU),
	FIELDINIT(UNCONNECTU),
	FIELDINIT(ETALKU),
	FIELDINIT(FHSU),
	FIELDINIT(SERVU),
	FIELDINIT(RENU),
	FIELDINIT(GTLU),
	{0,0,0}
};

struct flagsym ibflags[] =
{
	FLGINIT(STARTEDF),
	FLGINIT(LOCKF),
	FLGINIT(LSTNF),
	FLGINIT(BURSTF),
	FLGINIT(EVWANTF),
	{0,0,0}
};

struct flagsym Cflags[] =
{
	FLGINIT(TLC_VOID),
	FLGINIT(TLC_HERE),
	FLGINIT(TLC_RUN),
	FLGINIT(TLC_SRQ),
	FLGINIT(TLC_SC),
	FLGINIT(TLC_CIC),
	FLGINIT(TLC_PIO),
	FLGINIT(TLC_HOFF),
	FLGINIT(TLC_CONNECT),
	FLGINIT(TLC_TALK),
	FLGINIT(TLC_LSTN),
	FLGINIT(TLC_GONG),
	FLGINIT(TLC_ERROR),
	FLGINIT(TLC_WANT),
	{0,0,0}
};

struct flagsym csrflags[] =
{
	FLGINIT(CSR_DMADONE),
	FLGINIT(CSR_INTR_),
	FLGINIT(CSR_DMAFIN_),
	FLGINIT(CSR_NXM),
	/*
	FLGINIT(CSR_BIT0_),
	 */
	{(1<<3)<<3*NBBY,(1<<3)<<3*NBBY,"CSR_BIT0_"},
	FLGINIT(CSR_NDAC_),
	FLGINIT(CSR_TICK),
	FLGINIT(CSR_POLLING),
	FLGINIT(CSR_CIC),
	FLGINIT(CSR_ATN_),
	FLGINIT(CSR_SPMS),
	FLGINIT(CSR_LPAS),
	FLGINIT(CSR_TPAS),
	FLGINIT(CSR_LA),
	FLGINIT(CSR_TA),
	FLGINIT(CSR_MINOR),
	FLGINIT(CSR_ANYI),
	FLGINIT(CSR_SRQI),
	FLGINIT(CSR_LO),
	FLGINIT(CSR_RMT),
	FLGINIT(CSR_CRI),
	FLGINIT(CSR_LOCHGI),
	FLGINIT(CSR_RMTCHGI),
	FLGINIT(CSR_ADCHGI),
	FLGINIT(CSR_CPTI),
	FLGINIT(CSR_APTI),
	FLGINIT(CSR_DETI),
	FLGINIT(CSR_ENDI),
	FLGINIT(CSR_DCI),
	FLGINIT(CSR_ERRI),
	FLGINIT(CSR_ORI),
	FLGINIT(CSR_IRI),
	{0,0,0}
};

# define IBIO_GETNODE IBIOGETNODE
# define IBIO_SETNODE IBIOSETNODE
# define IBIO_GETCONN IBIOGETCONN
# define IBIO_SETCONN IBIOSETCONN
# define IBIO_TAKECTL IBIOTAKECTL
# define IBIO_PASSCTL IBIOPASSCTL
# define IBIO_LOCK IBIOLOCK
# define IBIO_PPC IBIOPPC
# define IBIO_PPU IBIOPPU
# define IBIO_POLL IBIOPOLL
# define IBIO_SRQ IBIOSRQ
# define IBIO_START IBIOSTART
# define IBIO_GETEV IBIOGETEV
# define IBIO_INIT IBIOINIT
# define IBIO_POKE IBIOPOKE
# define IBIO_PEEK IBIOPEEK
# define IBIO_DEBUG IBIODEBUG
# define IBIO_INTR IBIOINTR
# define IBIO_FLUSH IBIOFLUSH
# define IBIO_LISTEN IBIOLISTEN
# define IBIO_TALK IBIOTALK
# define IBIO_REN IBIOREN
# define IBIO_GTL IBIOGTL
# define IBIO_CUTOFF IBIOCUTOFF
struct flagsym iocodes[] =
{
	FIELDINIT(IBIOGETNODE),
	FIELDINIT(IBIOSETNODE),
	FIELDINIT(IBIOGETCONN),
	FIELDINIT(IBIOSETCONN),
	FIELDINIT(IBIOTAKECTL),
	FIELDINIT(IBIOPASSCTL),
	FIELDINIT(IBIOLOCK),
	FIELDINIT(IBIOPPC),
	FIELDINIT(IBIOPPU),
	FIELDINIT(IBIOPOLL),
	FIELDINIT(IBIOSRQ),
	FIELDINIT(IBIOCUTOFF),
	FIELDINIT(IBIOSTART),
	FIELDINIT(IBIOGETEV),
	FIELDINIT(IBIOINIT),
	FIELDINIT(IBIOPOKE),
	FIELDINIT(IBIOPEEK),
	FIELDINIT(IBIODEBUG),
	FIELDINIT(IBIOINTR),
	FIELDINIT(IBIOFLUSH),
	FIELDINIT(IBIOLISTEN),
	FIELDINIT(IBIOTALK),
	FIELDINIT(IBIOREN),
	FIELDINIT(IBIOGTL),
	FIELDINIT(IBIO_GETNODE),
	FIELDINIT(IBIO_SETNODE),
	FIELDINIT(IBIO_GETCONN),
	FIELDINIT(IBIO_SETCONN),
	FIELDINIT(IBIO_TAKECTL),
	FIELDINIT(IBIO_PASSCTL),
	FIELDINIT(IBIO_LOCK),
	FIELDINIT(IBIO_PPC),
	FIELDINIT(IBIO_PPU),
	FIELDINIT(IBIO_POLL),
	FIELDINIT(IBIO_SRQ),
	FIELDINIT(IBIO_CUTOFF),
	FIELDINIT(IBIO_START),
	FIELDINIT(IBIO_GETEV),
	FIELDINIT(IBIO_INIT),
	FIELDINIT(IBIO_POKE),
	FIELDINIT(IBIO_PEEK),
	FIELDINIT(IBIO_DEBUG),
	FIELDINIT(IBIO_INTR),
	FIELDINIT(IBIO_FLUSH),
	FIELDINIT(IBIO_LISTEN),
	FIELDINIT(IBIO_TALK),
	FIELDINIT(IBIO_REN),
	FIELDINIT(IBIO_GTL),
	{0,0,0}
};

struct flagsym fields[] =
{
	FLGINIT(IB_DIR),
	FLGINIT(IB_ISR1),
	FLGINIT(IB_ISR2),
	FLGINIT(IB_SPSR),
	FLGINIT(IB_ADSR),
	FLGINIT(IB_CPTR),
	FLGINIT(IB_ADR0),
	FLGINIT(IB_ADR1),
	FLGINIT(IB_BCR0),
	FLGINIT(IB_BCR1),
	FLGINIT(IB_SR),
	FLGINIT(IB_CCFR),
	FLGINIT(IB_DOR),
	FLGINIT(IB_IMR1),
	FLGINIT(IB_IMR2),
	FLGINIT(IB_SPMR),
	FLGINIT(IB_ADMR),
	FLGINIT(IB_AUXMR),
	FLGINIT(IB_ASLR),
	FLGINIT(IB_EOSR),
	FLGINIT(IB_CR0),
	FLGINIT(IB_CR1),
	FLGINIT(IB_MAR0),
	FLGINIT(IB_MAR1),
	FLGINIT(IB_MAR2),
	{0,0,0}
};


int lookup(name,tab,uflag,_val)
    char *name;
    register struct flagsym *tab;
    int uflag;
    int *_val;
{
    extern char *index();

    register char *up;

    for( ; tab->fname != 0; tab++ )
	if( strcmp(name,tab->fname) == 0
	 || (uflag
	    && (up = index(tab->fname,'_')) != 0
	    && * ++up != 000
	    && strcmp(name,up) == 0) )
	{
	    *_val = tab->fval;
	    return 0;
	}
    return-1;
}

int valof(name,tab,_val)
    char *name;
    struct flagsym *tab;
    int *_val;
{
    register char *cp;

    if( cnum(name,_val) == strlen(name) )
	return 0;
    for( cp = name; *cp != 000; cp++ )
	if( islower(*cp) )
	    *cp = toupper(*cp);
    if( lookup(name,tab,1,_val) >= 0 )
	return 0;
    return -1;
}

int symof(val,tab,_sym)
    int val;
    struct flagsym *tab;
    char **_sym;
{
    for( ; tab->fname != 0; tab++ )
	if( val == tab->fval )
	{
	    *_sym = tab->fname;
	    return 0;
	}
    return-1;
}

int flagex(str,tab)
    char *str;
    struct flagsym *tab;
{
    extern char *gettok();

    int sum,tokval;
    char *tok;

    sum = 0;
    settok(str);
    while( (tok = gettok()) != 0 )
    {
	if( *tok == '|' || *tok == '+' )
	    continue;
	if( valof(tok,tab,&tokval) < 0 )
	    errexit("unknown token %s",tok);
	sum |= tokval;
    }
    return sum;
}

int peekc;
char *tokstr,*itokstr;

settok(str)
    char *str;
{
    extern char *newstr();
    if( itokstr != 0 )
	free(itokstr);
    itokstr = 0;
    itokstr = tokstr = newstr(str);
    peekc = *tokstr;
}

char *gettok()
{
    register char *s,*t;
    s = tokstr;
    *s = peekc;
    while( isspace(*s) )
	s++;
    if( *s == 000 )
	return 0;
    t = s;
    if( isalpha(*s) || *s == '_' )
    {
	while( isalpha(*s) || isdigit(*s) || *s == '_' )
	    s++;
    }
    else
    if( isdigit(*s) )
    {
	if( s[0] == '0' && s[1] == 'x' )
	    s += 2;
	while( isdigit(*s) )
	    s++;
    }
    else
    {
	s++;
    }
    peekc = *s;
    *s = 000;
    tokstr = s;
    return t;
}

pflags(flags,vec)
    int flags;
    struct flagsym *vec;
{
    register struct flagsym *fp;
    int uflags;
    char *sep;

    uflags = flags;
    sep = "";
    for( fp = vec; fp->fname != 0; fp++ )
	if( (flags&fp->fmask) == fp->fval )
	{
	    uflags &= ~fp->fmask;
	    printf("%s%s",sep,fp->fname);
	    sep = "|";
	}
    if( uflags != 0 )
	printf("%s$%x",sep,uflags);
}

nn()
{
    putchar('\n');
}
