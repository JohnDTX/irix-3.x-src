int
numargs(argc,argv,vec,minvec,maxvec)
    int argc; register char **argv;
    register long *vec; int minvec,maxvec;
{
    register char *ap;

    if( !(minvec <= argc && argc <= maxvec) )
	return argcnt() , -1;

    while( --argc >= 0 )
    {
	ap = *argv++;
	if( strcmp(ap,"-") == 0 )
	    continue;
	if( !isnum(ap,vec++) )
	    return illegalnum(ap) , -1;
    }

    return 0;
}
