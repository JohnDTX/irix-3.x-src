/*
 * prom / standalone library test program.
 * checks whether loaded OK,
 * then loops echoing typein.
 * "dm ADDR [COUNT [WIDTH]]"
 * "q"
 */
main()
{
    int argc; char **argv;
    long aa[5];

    printf("hello, world!\n");

    loadcheck();

    for( ;; )
    {
	printf("test>");
	readargs(&argc,&argv);
	if( argc <= 0 )
	    continue;
	if( strcmp(*argv,"q") == 0 )
	    break;
	if( strcmp(*argv,"dm") == 0 )
	{
	    argc--; argv++;
	    aa[1] = 0xFFFFFF;
	    aa[2] = 2;
	    if( numargs(argc,argv,aa,1,3) < 0 )
		continue;
	    wdumpmem(aa[0],aa[2],aa[1]);
	}
	if( strcmp(*argv,"x") == 0 )
	{
	    extern char GLX[];
	    extern short DumberTerm;

	    DumberTerm = !DumberTerm;
	    printf("glx = $%x\n",GLX);
	}
    }

    printf("goodbye, world!\n");
}
