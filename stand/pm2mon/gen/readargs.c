#

# define MAXARGS	50
/*static*/ char *argvec[MAXARGS];

int
readargs(_argc,_argv)
    int *_argc;
    char ***_argv;
{
    extern char *getstr();

    register int nvec;
    register char *cp;

    if( (cp = getstr()) == 0 )
	return -1;
    if( (nvec = splitline(cp,argvec,MAXARGS)) < 0 )
	return -1;

    *_argv = argvec;
    *_argc = nvec;
    return nvec;
}


# include "ctype.h"

# define isquot(x)	(ccc=='\''||ccc=='"')

int
splitline(cp,vec,maxvec)
    register char *cp;
    register char * *vec;
    int maxvec;
{
    register int nvec;
    register unsigned char ccc;
    char blash,quotc;


    for( nvec = 0; nvec < maxvec; nvec++ )
    {
	while( isspace(*cp) )
	    cp++;
	if( *cp == 000 )
	    break;
	*vec = cp;

	blash = 0; quotc = 0;

	while( (ccc = *cp) != 000 )
	{
	    if( blash )
	    {
		blash = 0;
		cp++;
		continue;
	    }
	    if( quotc )
	    {
		if( ccc == quotc )
		    quotc = 0;
		cp++;
		continue;
	    }
	    if( isspace(ccc) )
		break;
	    blash = ccc == '\\';
	    if( isquot(ccc) )
		quotc = ccc;
	    cp++;
	}

	if( *cp != 000 )
	    *cp++ = 000;

	unquote(*vec++);
    }

    return nvec;
}

int
unquote(str)
    register char *str;
{
    register char *tp;
    register unsigned char ccc;
    char blash,quoted;
    char quotc;

    tp = str;
    blash = 0; quoted = 0;
    quotc = 0;

    while( (ccc = *str++) != 000 )
    {
	if( blash )
	{
	    quoted = 1;
	    tp[-1] = tp[0];
	    blash = 0;
	    continue;
	}
	if( quotc )
	{
	    quoted = 1;
	    *tp++ = ccc;
	    if( ccc == quotc )
	    {
		quotc = 0;
		tp--;
	    }
	    continue;
	}
	blash = ccc == '\\';
	*tp++ = ccc;
	if( isquot(ccc) )
	{
	    quotc = ccc;
	    tp--;
	}
    }

    *tp = 000;
    return quoted;
}
