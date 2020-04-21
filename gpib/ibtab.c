#
/*
 * ibtab.c --
 * package for dealing with the ibtab file.
 * ala fstab.
 */
# include "sys/param.h"
# include "sys/types.h"
# include "sys/sysmacros.h"
# include "sys/ib_ioctl.h"


# include "stdio.h"
# include "ctype.h"


# include "ibtab.h"


# define MAXIBLINE	256	/*max # chars in 1 line of IBTAB*/
# define SEP		':'	/*field separator chr in IBTAB*/

static FILE *IBF = 0;
static struct ibtab T;
static char ibline[MAXIBLINE];
static char *ibtabfile = IBTAB;


setibfile(name)
    char *name;
{
    ibtabfile = name;
    endibent();
}

/*
 * setibent() --
 * position the ibtab file at 0,
 * opening it if necessary.
 * returns neg iff failure.
 */
int
setibent()
{
    if( IBF != 0 )
    {
	fseek(IBF,0L,0);
	return 0;
    }
    IBF = fopen(ibtabfile,"r");
    return IBF == 0 ? -1 : 0;
}

/*
 * endibent() --
 * close the ibtab file.
 */
endibent()
{
    if( IBF != 0 )
	fclose(IBF);
    IBF = 0;
}

/*
 * getibent() --
 * returns a ptr to the next entry
 * in the ibtab file.
 */
struct ibtab *
getibent()
{
    static char *zp;
    extern char *skip();

    int iii,node;
    register char *ap,*cp;

    if( IBF == 0 )
	if( setibent() < 0 )
	    return 0;

    /*skip comments and blank lines*/
    for( ;; )
    {
	if( fgets(ibline,MAXIBLINE,IBF) == 0 )
	    return 0;
	ap = ibline+strlen(ibline);
	if( ap > ibline && *--ap == '\n' )
	    *ap = 000;
	ap = ibline;
	while( isspace(*ap) )
	    ap++;
	if( *ap != '#' && *ap != 000 )
	    break;
    }

    zp = ibline;
    T.ibt_node = -1;

    if( (T.ibt_file = skip(&zp)) == 0 )
	return &T;

    if( (T.ibt_cfile = skip(&zp)) == 0 )
	return &T;

    if( (cp = skip(&zp)) == 0 || cnum(cp,&iii) <= 0 )
	return &T;
    node = iii;

    if( (T.ibt_flags = skip(&zp)) == 0 )
	return &T;

# ifdef notdef
    for( cp = &T.ibt_tag; cp <= &T.ibt_I; cp++ )
    {
	if( (ap = skip(&zp)) == 0 || cnum(ap,&iii) <= 0 )
	    return &T;
	*cp = iii;
    }
# endif notdef

    if( (ap = skip(&zp)) == 0 || cnum(ap,&iii) <= 0 )
	return &T;
    T.ibt_tag = iii;

    if( (ap = skip(&zp)) == 0 || cnum(ap,&iii) <= 0 )
	return &T;
    T.ibt_ppr = iii;

    T.ibt_node = node;
    T.ibt_comment = zp;
    return &T;
}

# ifdef notdef
/*
 * getibname() --
 * returns an (the) ibtab file entry with
 * file name, 0 if none.
 */
struct ibtab *
getibname(name)
    char *name;
{
    register struct ibtab *ip;

    setibent();
    while( (ip = getibent()) != 0 )
	if( strcmp(ip->ibt_file,name) == 0 )
	    break;
    endibent();
    return ip;
}

/*
 * getibnode() --
 * returns the ibtab file entry with
 * node node, or 0 if none.
 */
struct ibtab *
getibnode(node)
    int node;
{
    register struct ibtab *ip;

    setibent();
    while( (ip = getibent()) != 0 )
	if( ip->ibt_node == node )
	    break;
    endibent();
    return ip;
}
# endif notdef


# define FLGINIT(x)	{x,x,"x"}
struct flagsym
{
    int fmask;
    int fval;
    char *fname;
};

static
struct flagsym nflags[] =
{
	FLGINIT(IBN_VALID),
	FLGINIT(IBN_SWAB),
	FLGINIT(IBN_PPE),
	FLGINIT(IBN_PPC),
	FLGINIT(IBN_SRQ),
	FLGINIT(IBN_SC),
	{0,0,0}
};

int ibnflags(str,_val)
    char *str;
    int *_val;
{
    return flagex(str,nflags,_val);
}


/*----- auxiliary routines*/
static char *
skip(_ptr)
    char **_ptr;
{
    register char *ap,*cp;

    ap = cp = *_ptr;
    if( *ap == 000 )
	return 0;

    while( *ap != SEP && *ap != 000 )
	ap++;

    if( *ap != 000 )
	*ap++ = 000;

    *_ptr = ap;
    return cp;
}


static
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

static
int fleval(name,tab,_val)
    char *name;
    struct flagsym *tab;
    int *_val;
{
    if( cnum(name,_val) == strlen(name)
     || lookup(name,tab,1,_val) == 0 )
	return 0;
    return -1;
}

static
int flagex(str,tab,_val)
    char *str;
    struct flagsym *tab;
    int *_val;
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
	if( fleval(tok,tab,&tokval) < 0 )
	    return-1;
	sum |= tokval;
    }
    *_val = sum;
    return 0;
}

static int peekc;
static char *tokstr,*itokstr;

static
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

static
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
