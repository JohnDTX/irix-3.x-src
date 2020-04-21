/*
 * %s:
 *	- front end for MC68000 C compiler
 */
#include <sys/types.h>
#include <stdio.h>
#include <ctype.h>
#include <signal.h>
#include <sys/times.h>
#include <sys/kversion.h>

#ifndef KVERS_CPUTYPE
#define KVERS_CPUTYPE 3
#endif
#define MAXNEWARGS 20
#define MAXAF 4
#define MAXCF 4

#define BinDirectory 	"/bin/"
#define LibDirectory 	"/lib/"
#define InclDirectory	"/usr/include"

#define NPASSES 8 
#define CPP 0
#define CCOM 1		/* constants used by the timing function (-q) */
#define C2 2
#define AS 3
#define LD 4
#define FORT0 5
#define FORT1 6
#define FORT2 7
#define PASCAL 8

#define NONE_SPECIFIED 0
#define IRIS 1
#define JUNIPER 2
#define NMACHINES 3

#define add_libpath(str) strspl(ldlibpath,str)

char	*passnames[NPASSES+1] = {"cpp ","ccom","c2  ","as  ",
				  "ld  ","fort","code","ulnk","pascal"};

char	*cpp = "cpp";		/* C preprocessor */
char	*ccom = "ccom";		/* portable C compiler */
char	*c2 = "c2";		/* Unisoft's .s->.s optimizer */
char 	*as[NMACHINES] = {"/bin/as","/bin/as10","/bin/as20"};
char	*ld = "ld";		/* 68k link editor */
char	*cat = "cat";
char 	*fort0[NMACHINES] = {"/usr/lib/fortran","/usr/lib/fortran10","/usr/lib/fortran20"};
char 	*fort1[NMACHINES] = {"/usr/lib/code","/usr/lib/code10","/usr/lib/code20"};
char	*fort2[NMACHINES] = {"/usr/lib/jlinker","/usr/lib/jlinker10","/usr/lib/jlinker20"};
char	*pascal[NMACHINES] = {"/usr/lib/pascal","/usr/lib/pascal10","/usr/lib/pascal20"};
char	*swfpfortlib0[NMACHINES] = {"-lf","-lf10","-lf20"}; /* SVS Fortran general lib */
char	*hwfpfortlib0[NMACHINES] = {"-lfhw","-lfhw10","-lfhw20"}; /* ditto with fpa support */
char	*swfpfortlib1[NMACHINES] = {"-lp","-lp10","-lp20"};/* SVS Fortran lib with fp s/w */
char	*hwfpfortlib1[NMACHINES] = {"-lphw","-lphw10","-lphw20"}; /* ditto with fpa support */
char 	*libmhwfp[NMACHINES] = {"-lmhw","-lmsky","-lmfpa"};
char	**fortlib1 = swfpfortlib1;
char	**fortlib0 = swfpfortlib0;

	/* FORTRAN block data for graphics */

#ifdef GL1
char	*fblkdat = "/usr/lib/fgldat.o"; 
char	*pascalglue = "/usr/lib/pjmptbl.o";
#else
char	*pascalglue = "/usr/lib/p2cstr.o";
#endif
#define JSTRLEN  0x20
char	*juniperhw = "/usr/lib/juniperhw.o";
char	*juniperhw_p = "/usr/lib/juniperhw_p.o";

#ifdef HWFPAUTOCONF
char	*hwfptest = "/bin/hwfp";
#endif

char	*astest[NMACHINES] = {"/usr/src/bin/as/as","/usr/src/bin/as/as","/usr/src/bin/as20/as20"};
char  	*ccomtest = "/usr/src/lib/ccom/ccom";
char 	*ooptions = "";			/* options for test optimizer */
				/* run-time start-up */
char	*crt0 = "crt0.o";
char	*crt0stand = "crtstand.o";	/* run-time start-up for standalone */

char	*defines;			/* tell cpp this is a 68000 */
char 	*includes;			/* includes string for cpp */
char	*entrypt;
char 	*progstr;
char 	*ldlibpath;
char 	*altdiv = "/usr/lib/swdiv.o";
char	binsuffix='o';

char	tmp0[30];		/* big enough for /tmp/ctm%05.5d */
char	*tmp1, *tmp2, *tmp3, *tmp4, *tmp5, *tmp6;
char	*infile=0, *outfile=0, *fortobj=0;
char	*savestr(), *strspl(), *setsuf(), *setlongsuf();
char	*type;
int	idexit();
char	**av, **clist, **llist, **flist, **ulist, **plist, **svslist;
char  	**fflist, **aflist ,**cflist, **pflist;
char	**cpplist;
int	cflag=0, 	/* skip link editing, result is filename.o */
	eflag=0, 	/* error flag; records errors */
	lflag=0,	/* 1 -> as generates listing */
	oflag=0, 	/* optimization flag; 1 -> invoke c2 */
	pflag=0,	/* 1 -> don't delete temporary files */
	Rflag=0,	/* 1 -> ld preserves relocation bits */
	sflag=0,  	/* 1 -> skip assembly, result is filename.s */
	zflag=0,	/* print exec() trace */
	exflag=0, 	/* 1 -> apply cpp only, result to stdout */
	Mflag=0, 	/* 1 -> apply cpp -M only, result to stdout */
	noxflag=0,	/* 1 -> -x flag off -> output local symbols */
	timeflg=0,	/* 1 -> time all processes */
	hwfpflag=0,	/* use hardware floating point processor for 
			   fp instructions*/
	libmindex = (-1), /* index into llist[] of -lm, for possible 
			     modification due to hardware floating point */
	seenlibc=0,	/* 1 -> libc seen. Otherwise, give as default */
	glflag=0,	/* 1 -> using the graphics library */
	rglflag=0,	/* 1 -> using the remote graphics library */
	noshareflag=0,	/* if set, don't pass the shared text flag to ld */
	verbose = 0,
	standalone = 0,	/* if set, use standalone startup and library */
	fxref = 0,	/* generate a cross reference listing for FORTRAN */
	dbgflag = 0,/* generate debugger information */
	svsprog = 0,
	haspascal = 0, /* 1 if pc called, .p files seen, or the ZP flag given. 
					  0 otherwise */
	hasfortran = 0, /* 1 if f77 called, .f files seen, or the ZF flag given.
					   0 otherwise */
	preserve_dot_js = 0, /* if set (by -ZU flag), preserve the .j files
				in a Pascal compilation (for USES) */
	precise_divide = 0,  /* if set, use hacked software divide routines
				on Juniper when running h/w floating point,
				rather than the board's hardware divide. */
	/* host type is JUNIPER if running on a juniper, IRIS otherwise */
	/* machine type is JUNIPER if 
	   assembling for mc68020, IRIS otherwise */
#ifdef juniper
	host_machine = JUNIPER,
	target_machine = JUNIPER, 
#else
	host_machine = IRIS,
	target_machine = IRIS, 
#endif
	target_machine_specification = NONE_SPECIFIED,	/* set if machine specified by -ZI or -ZJ */
	usetest = 0,
	binaries=0, /* binaries have been seen in this load */
	seenlibs=0, /* set when libraries have been seen */
	altlibpath=(-1), /* index into flist of an alternate library 
			    path (for ld) has been seen.
			    The path is in ldlibpath. */
	usercrt0=0,	/* user specified own startup. */
	suppress_warnings=0, /* set if -w specified in cc cmd */
	proflag=0;	/* profile flag: generate jbsr mcount for each fctn */
int	
	exfail;
char	*chpass=0,
	*Org = 0,
	*npassname=0;

int	nc=0, 	/* no of programs for ccom */
	nl=0, 	/* no of .o inputs for ld (including libraries) */
	nm=0,	/* no of modules (excluding libraries) */
	nf=0,	/* no of flags for ld */
	nsvs=0,/* no of fortran source files */
	nu=0,	/* no of files of unknown type */
	np=0, 	/* no of args for cpp */
	nff=0,	/* no of flags for fortran */
	npf=0,	/* no of flags for pascal */
	naf=0,	/* no of flags for as */
	ncpp=0,	/* no of flags for cpp */
	ncf=0,	/* no of flags for ccom */
	one_source_to_executable=0,
	one_source =0,
	firstCbin= (-1),
	na=0;	/* no of args to each callsys */
	firstFPsrc = -1; /* first fortran or pascal source */
	usermask = 0;	/* the default user mask for files */

time_t	stime[NPASSES+1]={0},	/* total system time this pass (if timing) */
	utime[NPASSES+1]={0} ;   /* total user time this pass (if timing) */

struct tms sttime,*times();

#ifdef NOCALLSYS
#define cunlink(S)	if (S && !pflag) printf("unlink %s\n",S)
#else
#define	cunlink(s)	if (s&&!pflag) unlink(s)
#endif

/******************************************************************
   
	The following options characters are currently in use in cc:

	c	- dont load
	g	- generate debugging information
	-n	- dont pass the 'shared text' flag to ld
	o	- name output
	p	- profile
	-x	- dont pass x flag to loader (delete local symbols)
	C	- pass to cpp
	D	- pass to cpp
	E	- pass input thru cpp, send result to stdout
	I	- pass to cpp
	L	- make .lst files at assembly and from the Fortran compiler
	N	- pass to cpp
	O	- optimize .s files
	P	- pass input thru cpp, send result to file.i
	S	- generate .s files
	U	- pass to cpp
	Z	- the following defines a local switch


	The following are passed on to or used by ld and should not be used:

	d,e,l,n,r,s,u,x,F,L<x>,N,X

	The following are local switches.  These are prefixed with the Z switch:

	i	- use the attached string as the startup
	f	- generate instructions for the Sky floating point processor
	g	- link with all the goo necessary for the graphics library with FORTRAN
	q	- time C compilation passes
	v	- verbose mode - generate all warnings and pass to ccom
	x	- generate cross reference file in all FORTRAN listings
	z	- print exec() trace
	F	- some of the .j files in the input to be linked are from FORTRAN.
			Load the appropriate goo.
	P	- some of the .j files in the input to be linked are 
			from Pascal. Load the appropriate goo.
	R	- the following argument is the root of the library tree
			for ld.
	U	- preserve .j files in Pascal compilations
	Y	- use test versions of ccom, as.
	Z	- standalone program.  Load with libstand.a and
		     standalone startup.

	Anyone putting new switches in cc should consult (and update)
	this list so that conflicts can be avoided.
*/

main(oargc, oargv)
	char **oargv;
{
	char *t;
	char *assource;
	char **argv;
	int argc;
	int arg0len;
	char *cptr;

	int i, j, c;
/* CV:	use the full user's umask for all created files */
	usermask = umask(0);

	argv = (char **)calloc(oargc+MAXNEWARGS, sizeof (char **));
	for (i=0;i<oargc;i++) argv[i] = oargv[i];
	argc = setenv(oargc,argv);

	/* ld currently adds upto 5 args; 20 is room to spare */
	av = (char **)calloc(argc+20, sizeof (char **));
	clist = (char **)calloc(argc, sizeof (char **));
	llist = (char **)calloc(argc, sizeof (char **));
	flist = (char **)calloc(argc, sizeof (char **));
	ulist = (char **)calloc(argc, sizeof (char **));
	plist = (char **)calloc(argc, sizeof (char **));
	svslist = (char **)calloc(argc, sizeof (char **));
	fflist =(char **)calloc(argc, sizeof (char **)); 
	cflist =(char **)calloc(argc, sizeof (char **)); 
	aflist =(char **)calloc(argc, sizeof (char **)); 
	pflist =(char **)calloc(argc, sizeof (char **)); 
	includes = strspl("-I",InclDirectory);
	crt0 = strspl(LibDirectory,crt0);
	crt0stand = strspl(LibDirectory,crt0stand);
	cpp = strspl(LibDirectory,cpp);
	ccom = strspl(LibDirectory,ccom);
	c2 = strspl(LibDirectory,c2);
	arg0len = strlen(argv[0]);
	cptr = argv[0];
	if (arg0len>=3)
		cptr = &argv[0][arg0len-3];
	hasfortran = !(strncmp("f77",cptr,3));
	cptr = &argv[0][arg0len-2];
	haspascal = !(strncmp("pc",cptr,2));
	progstr = (haspascal)?"pc":(hasfortran?"f77":"cc");
	i = 0;
	while (++i < argc) {
		if (*argv[i] == '-') switch (argv[i][1]) {

		case '-':	/* negate some default */
			switch(argv[i][2]) 
			  {
				case 'x':
					noxflag++;
					break;
				case 'n':
					noshareflag++;
					break;
		    	  }
			break;

		case 'C':
			plist[np++] = argv[i];
			break;
		case 'D':
			plist[np++] = argv[i];
			break;
		case 'E':
			exflag++;
			pflag++;
			break;
		case 'I':
			plist[np++] = argv[i];
			break;
		case 'L':
			if (!argv[i][2]) {
			    lflag++;
			} else {
				flist[nf++] = argv[i++];
				flist[nf++] = argv[i];
			}
			break;
		case 'M':
			exflag++;
			Mflag++;
			pflag++;
			break;
		case 'O':
			oflag++;
			ooptions = &argv[i][2];
			break;
		case 'P':
			pflag++;
			break;

		case 'T':
		case 'R':
			if (++i < argc)
				Org = argv[i];
			break;
		case 'S':
			sflag++;
			cflag++;
			break;
		case 'U':
			plist[np++] = argv[i];
			break;

		case 'V':
			/* ignored */
			break;

		case 'Z': 
			/* local switch */
			switch (argv[i][2]) {

		        case '1':
				/* cpp flag */
				cpplist[ncpp++] = &argv[i][3];
				break;

		    	case 'A':
				/* as flag */
				if (naf > MAXAF)
					fprintf(stderr,
	"%s: maximum number of assembler arguments (max = %d) exceeded.\n",
						progstr,MAXAF);
				else 
					aflist[naf++] = &argv[i][3];
				break;

		    	case 'C':
				/* ccom flag */
				cflist[ncf++] = &argv[i][3];
				break;

			case 'D':
				/* use software divide for added precision */
				precise_divide++;
				break;

			case 'F':
				/* fortran front end flag */
				hasfortran++;
				if (argv[i][3]) fflist[nff++] = &argv[i][3];
				break;

			case 'H':
				/* Use running machine as target. 
				*/
#ifdef juniper
				if (target_machine_specification == IRIS)
					fprintf(stderr,
	"%s: respecification of target machine.  Was 68010, now 68020\n",
					progstr);
				target_machine = JUNIPER;
				target_machine_specification = JUNIPER ;
#else
				if (target_machine_specification == JUNIPER)
					fprintf(stderr,
	"%s: respecification of target machine.  Was 68020, now 68010\n",
					progstr);
				target_machine = IRIS;
				target_machine_specification = IRIS ;
#endif
				break;

			case 'I':
				/* Iris processor flag. Reset Juniper flag. */
				if (target_machine_specification == JUNIPER)
					fprintf(stderr,
	"%s: respecification of target machine.  Was 68020, now 68010\n",
					progstr);
				target_machine = IRIS;
				target_machine_specification = IRIS ;
				break;

			case 'J':
				/* Juniper processor flag */
				if (target_machine_specification == IRIS)
					fprintf(stderr,
	"%s: respecification of target machine.  Was 68010, now 68020\n",
					progstr);
				target_machine = JUNIPER;
				target_machine_specification = JUNIPER;
				break;

			case 'N':
				plist[np++] = "-N";
				break;

			case 'P':
				/* pascal front end flag */
				haspascal++;
				if (argv[i][3]) pflist[npf++] = &argv[i][3];
				break;

			case 'R':
				/* alternate lib path is in argument following.
				   pass to ld.
				*/
				if (++i < argc) {
					ldlibpath = argv[i];
					if (altlibpath>=0) {
						fprintf(stderr,
			"%s: second -R path of %s overrides previous of %s\n", 
							progstr,argv[i],
							flist[altlibpath]);
						
						flist[altlibpath] = ldlibpath;
					}
					else {
						flist[nf++] = "-R";
						altlibpath = nf;
						flist[nf++] = ldlibpath;
					}
				}
				else {
					fprintf(stderr,
						  "%s: -R requires argument\n",
						  progstr);
					exit(-1);
				}
				break;

			case 'U':
				preserve_dot_js++;
				break;

			case 'Y':	/* use test passes of the compiler */
				usetest++;
				break;
			
			case 'Z':
				standalone++;
				noshareflag++;
				break;
			case 'f':
				if (argv[i][3] == '-')
					hwfpflag = (-1);
				else 
					hwfpflag = 1;
				break;
			case 'g':
				glflag++;
				if (rglflag){
					rglflag = 0;
	   	fprintf(stderr,"%s : -Zg supercedes -Zr.\n",progstr);
				}
				break;
			case 'i':
				crt0 = argv[i]+3;
				usercrt0++;
				break;
			case 'q':	/* time all processes.*/
					/* report time on stdout.*/
				timeflg++;
				break;
			case 'r':
				/* remote graphics library */
				if ( glflag){
	   	fprintf(stderr,"%s : -Zg supercedes -Zr.\n",progstr);
				}
				else rglflag++;
				break;

			case 'v':
				verbose++;
				break;

			case 'x':
				/* make FORTRAN generate a cross reference listing */
				fxref++;
				break;
	
			case 'z':	/* trace exec() calls */
				zflag++;
				break;
	
			default:
				fprintf(stderr,
			"%s: Unrecognized local switch %s -ignored.\n",progstr,
					argv[i]);
			}
			break;

		case 'c':
			cflag++;
			break;

		case 'e':
			if (++i < argc)
				entrypt = argv[i];
			break;

		case 'g':
			dbgflag++;
			noxflag++;
			break;

		case 'l':
			/* parse the library... */
			if (argv[i][2]=='m' && !argv[i][3]) 
			{
				libmindex = nl;
			}
			llist[nl++] = argv[i];
			seenlibs++;
			break;
		case 'o':
			if (++i < argc) {
				outfile = argv[i];
				switch (getsuf(outfile)) {

				case 'c':
					error("-o would overwrite %s",
					    outfile);
					exit(8);
				}
			}
			break;
		case 'p':
			proflag++;
			break;
		case 'r':
			Rflag++;
			break;
		case 'u':
			if (++i < argc) {
				llist[nl++] = "-u";
				llist[nl++] = argv[i];
			}
			break;
		case 'w':
			suppress_warnings++;
			break;

		default:
deflt:
			flist[nf++] = argv[i];
			break;
		}
		else {			/* not a flag */
			t = argv[i];
			c = getsuf(t);
			if (c == binsuffix) binaries++;
			if (c=='c' || c=='s' || exflag) {
				clist[nc++] = t;
				if (((c == 'c')||(c == 's'))&&(firstCbin < 0))
					firstCbin = nl;
				t = setsuf(t, binsuffix);
				c = binsuffix;
			}
			if ( c == 'j') {
				fprintf(stderr,
		  "%s: warning: %s is old-type fortran binary - ignored.\n",
					progstr,t);
				continue;
			}
			if ( (c == 'f') || (c == 'p') ) {
				if (c == 'p') haspascal=1; 
				else if (c == 'f') hasfortran=1;
				svslist[nsvs++] = t;

/* dar: put fortran and pascal objects in loader list
 * in the order they come
 */
			        t = setsuf(t, binsuffix);
			        if (nodup(llist, t)) {
					llist[nl++] = t;
					nm++;	/* count programs */
			  	}
/* dar firstFPsrc:  indexs source in llist - if only one source
 */
				if (firstFPsrc > 0)
					firstFPsrc = 0;
				else
					if (firstFPsrc < 0) 
						firstFPsrc = nl - 1;
				
			}
			if (c=='a' || c== binsuffix) 
			  {
				if (nodup(llist, t))
				  {
					llist[nl++] = t;
					nm++;	/* count programs */
				  }
				/* here is the place to catch libc's, 
				   if desired.
				*/
			  }
			else if ( (c != 'f') && (c != 'p') ) {
				ulist[nu++] = t; /* Unrecognized suffix */
			}
		}
	}	/* End of loop to process arguments */

	for (i=0; i<nu; i++) {
		type = "ignored.";
		/* if we are loading, put it on the loader list */
		if ((!exflag) && (!sflag) && (!cflag))
		{
			llist[nl++] = ulist[i];
			nm++;	/* count programs */
			type = 
	"taken to be relocatable binary,\n\tabsolute or library (types .o, .a or a.out)";
		}

		/* if we are only compiling, and it has a suffix, assume it
		   is C source.
		*/
		if ((getsuf(ulist[i]))&&(exflag||sflag||cflag)) {
			clist[nc++] = ulist[i];
			type = "assumed to be C code (type .c)";
		}

		fprintf(stderr,
		  "%s: warning: %s has unrecognized suffix - %s\n",
		  	progstr,ulist[i],type);
	}

	if (zflag) {
		printf("\t");
		for (i=0;i<argc;i++)
			printf("%s ",argv[i]);
		printf("\n");
	}
	one_source_to_executable = 
		(((nc + nsvs) == 1) && (!cflag) && (!binaries)) ;
	one_source = 
		((nc + nsvs) == 1) ;
/* dar delete one fortran or pascal source from loader list
 * if it is the only one
 */
 if (firstFPsrc > 0 && one_source_to_executable) {
  for (i=firstFPsrc+ 1; i<=nl; i++)
 	llist[i-1] = llist[i];
  nl--;

 }

	if (dbgflag && oflag) {
		fprintf(stderr,"%s: -g conflicts with -O. -O ignored.\n",progstr);
		oflag = 0;
	}
	if (dbgflag && haspascal) {
		fprintf(stderr,
		    "%s: -g not available with pascal in 2.4/3.4\n",progstr);
		dbgflag = 0;
	}
	if (rglflag && (haspascal||hasfortran)) {
		fprintf(stderr,
			"%s: -Zr not available with pascal or fortran\n",progstr);
		rglflag = 0;
	}

	if (!nsvs && (!nl || cflag) && !nc && !infile) {
		fprintf(stderr,"%s: no input specified\n",progstr);
		exit(8);
	}
	if (proflag)
		crt0 = strspl(LibDirectory,"mcrt0.o");

	if (signal(SIGINT, SIG_IGN) != SIG_IGN)
		signal(SIGINT, idexit);
	if (signal(SIGTERM, SIG_IGN) != SIG_IGN)
		signal(SIGTERM, idexit);
	if (pflag==0)
		sprintf(tmp0, "/tmp/ctm%05.5d", getpid());
	tmp1 = strspl(tmp0, "1");
	tmp2 = strspl(tmp0, "2");
	tmp3 = strspl(tmp0, "3");
 	if (one_source_to_executable)
	{
 		tmp6 = strspl(tmp0, "6");
 		/* replace the loader input C file with the temporary .o file
 	   	   if there is a single C binary (sigh) */
 		if (nc) llist[firstCbin] = tmp6;
	}
	if (pflag==0)
		tmp4 = strspl(tmp0, "4");
	if (oflag)
		tmp5 = strspl(tmp0, "5");
	/* ok, test for hardware floating point option installed 
	   on this system if the user has not specified it.
	*/
#ifdef HWFPAUTOCONF
	if (hwfpflag == 0) {
		na=0;
		av[na++] = "hwfp";
		av[na] = 0;
		if (callsys(0, hwfptest, av, "Check for HWFP")) {
			hwfpflag = (-1);
		}
		else hwfpflag = 1;
	}
#endif
	/* if there is hardware floating point and the math library
	   has been specified, change it to be the hardware one */
	if (hwfpflag>0) {
		if (libmindex >=0)
			llist[libmindex] = libmhwfp[target_machine_specification];
		fortlib0 = hwfpfortlib0;
		fortlib1 = hwfpfortlib1;
	}
	if (usetest) {
		if (*ccomtest) {
			ccom = ccomtest;
			if (verbose)
				fprintf(stderr,
					"%s : Test version of ccom invoked.\n",
					progstr);
		}
		if (*astest) {
			as[target_machine_specification] = astest[target_machine_specification];
			if (verbose)
	 			fprintf(stderr,
					"%s : Test version of as invoked.\n",
					progstr);
		}
	}
	if ((precise_divide) && (hwfpflag == 1) && 
	    (target_machine == JUNIPER) && (hasfortran||haspascal) ){
		llist[nl++] = (altlibpath >= 0)?add_libpath(altdiv):altdiv;
		pflist[npf++] = "+F";
		fflist[nff++] = "+F";
	}
	if (nc==0)
		goto dofort;
	for (i=0; i<nc; i++) {
		if (nc > 1) {
			printf("%s:\n", clist[i]);
			fflush(stdout);
		}
		if (getsuf(clist[i]) == 's') {
			assource = clist[i];
			goto assemble;		/* thereby skipping ccom */
		} else
			assource = tmp3;
		if (pflag)
			tmp4 = setsuf(clist[i], 'i');
		av[0] = "cpp";
		if (Mflag) {
			av[1] = "-M";
			av[2] = clist[i]; av[3] = exflag ? "-" : tmp4;
			na = 4;
		} else {
			av[1] = clist[i]; av[2] = exflag ? "-" : tmp4;
			na = 3;
		}
		for (j = 0; j < ncpp; j++)
			av[na++] = cpplist[j];
		for (j = 0; j < np; j++)
			av[na++] = plist[j];

		if (rglflag) av[na++] = "-I/usr/include/rgl";
		if (target_machine_specification != NONE_SPECIFIED) {
			if (target_machine == IRIS) {
				av[na++] = "-Ujuniper";
				av[na++] = "-DpmII";
			}
			else if (target_machine == JUNIPER) {
				av[na++] = "-Djuniper";
				av[na++] = "-UpmII";
			}
		}
		av[na++] = 0;
		if (timeflg) times(&sttime);
		if (callsys(0, cpp, av, "Preprocessing")) {
		        exfail++;
			eflag++;
		}
		if (timeflg) updatetime(&sttime,CPP);
		if (pflag || exfail) {
			cflag++;
			continue;
		}
		if (sflag || oflag 
#ifndef ALL_AS20
			|| (target_machine == IRIS)
#endif
			)
		{

			if (sflag) 
				assource = tmp3 = (outfile && one_source)?
					outfile:setsuf(clist[i], 's');

			av[0] = "ccom"; av[1] = tmp4; av[2] = oflag?tmp5:tmp3; na = 3;
			if (proflag)
				av[na++] = "-Xp";
			if (hwfpflag > 0)
				av[na++] = "-f";
#ifndef ALL_AS20
			if (target_machine == JUNIPER)
#endif
				av[na++] = "-N";
			if (target_machine == JUNIPER)
				av[na++] = "-J";
			if (verbose)
				av[na++] = "-v";
			if (suppress_warnings)
				av[na++] = "-w";
			if (dbgflag) 
				av[na++] = "-Xg";
			for (j=0;j<ncf;j++) av[na++] = cflist[j];
			av[na] = 0;
			if (callsys(0, ccom, av, "  Compiling")) {
				cflag++;
				eflag++;
				continue;
			}
		} 
		else { 
			/* pipe the input to the new assembler */
			char command[256];
			char *result;
			if (one_source && cflag && outfile)
				result = outfile;
			else if (one_source_to_executable)
				result = tmp6;
			else 
				result = setsuf(clist[i],binsuffix);
#ifdef NOTDEF
			sprintf(command,"%s%s %s %s %s <%s | %s %s -o %s",
#endif
			/* NOTE!!! MAXAS/MAXCS better not be larger than FOUR! ***/
			sprintf(command,
			"%s%s %s %s %s %s %s %s %s %s %s <%s | %s -o %s %s %s %s %s",
				((*ccom == '/') ? "" : LibDirectory),
				ccom,
				(proflag ? "-Xp" : ""),
				(dbgflag ? "-Xg" : ""),
				(hwfpflag>0)? "-f" : "",
#ifdef ALL_AS20
				"-N",
#else
				((target_machine == JUNIPER) ? "-N":""),
#endif
				((target_machine == JUNIPER) ? "-J":""),
				(ncf > 0)?cflist[0]:"",
				(ncf > 1)?cflist[1]:"",
				(ncf > 2)?cflist[2]:"",
				(ncf > 3)?cflist[3]:"",
				tmp4,
				as[target_machine_specification],
				result,
				(naf > 0)?aflist[0]:"",
				(naf > 1)?aflist[1]:"",
				(naf > 2)?aflist[2]:"",
				(naf > 3)?aflist[3]:""
				);
			if (zflag)
#ifdef ECHOCMDS
				printf("%s\n",command);
#else
				printf("  Compiling\n");
#endif
			if (timeflg) times(&sttime);
#ifndef NOCALLSYS
			if(system(command)) {
			    eflag++;
			    continue;
			    }
#endif
			if (timeflg) updatetime(&sttime,CCOM);
			continue;
		}
		if (timeflg) updatetime(&sttime,CCOM);
		if (oflag) {
			na = 0;
			av[na++] = "c2";
			if (*ooptions) 
			   av[na++] = strspl("-",ooptions);
			av[na++] = tmp5; 
			av[na++] = tmp3; 
			av[na++] = 0;
			if (timeflg) times(&sttime);
			if (callsys(0, c2, av, "  Optimizing")) {
				unlink(tmp3);
				tmp3 = assource = tmp5;
			} else
				unlink(tmp5);
			if (timeflg) updatetime(&sttime,C2);
		}
		if (sflag)
			continue;
	assemble:
		cunlink(tmp1); cunlink(tmp2); cunlink(tmp4);
		na = 0;
		av[na++] = 
				as[target_machine_specification];
#ifdef NOTDEF
		if (target_machine == IRIS)
			av[na++] = "-i";
		else  if (target_machine == JUNIPER)
			av[na++] = "-j";
#endif
		av[na++] = "-o"; 
		if (cflag && nc == 1 && outfile)
			av[na++] = outfile;
 		else av[na++] = one_source_to_executable?tmp6:setsuf(clist[i], binsuffix);
#ifdef NOTDEF
		if ((target_machine == IRIS) && (host_machine == IRIS) && lflag) {
			av[na++] = "-l";
		}
#endif
		av[na++] = assource;
		for (j=0;j<naf;j++) av[na++] = aflist[j];
		av[na] = 0;
		if (timeflg) times(&sttime);
		if (callsys(0, as[target_machine_specification], av, "  Assembling") > 1) {
			cflag++;
			eflag++;
			continue;
		}
		if (timeflg) updatetime(&sttime,AS);
	}		/* End of loop to process .[cs] files */
dofort:
	if (!exfail) {
		/* start loop to process .[fp] files */
		char *svsobjfile;
		for (i=0; i<nsvs; i++) {
			if (nsvs > 1) {
				printf("%s:\n", svslist[i]);
				fflush(stdout);
			}
			na=0;
			svsprog = (getsuf(svslist[i])=='p')?PASCAL:FORT0;
			if (pflag)
				tmp4 = setsuf(svslist[i], 'i');
			av[na++] = "cpp"; 
			if (svsprog == PASCAL) {
				av[na++] = "-P";
				av[na++] = "-p";
				av[na++] = "-C";
			}
			for (j = 0; j< ncpp; j++)
				av[na++] = cpplist[j];
			av[na++] = svslist[i]; av[na++] = exflag ? "-" : tmp4;
			for (j = 0; j < np; j++)
				av[na++] = plist[j];
			if (target_machine_specification != NONE_SPECIFIED) {
				if (target_machine == IRIS) {
					av[na++] = "-Ujuniper";
					av[na++] = "-DpmII";
				}
				else if (target_machine == JUNIPER) {
					av[na++] = "-Djuniper";
					av[na++] = "-UpmII";
				}
			}
			av[na++] = 0;
			if (timeflg) times(&sttime);
			if (callsys(0, cpp, av, "Preprocessing")) {
			        exfail++;
				eflag++;
			}
			if (timeflg) updatetime(&sttime,CPP);
			if (pflag || exfail) {
				cflag++;
				continue;
			}
			na = 0;
			av[na++] = passnames[svsprog];
			av[na++] = tmp4; 
			av[na++] = "+q";
			av[na++] = "-p";
			av[na++] = "-e";
			if ((hwfpflag<=0)) 
				av[na++] = "-f";
			else 
				av[na++] = "+f";
			if (dbgflag) av[na++] = "+d";
			if (lflag && fxref) av[na++] = "+x";
			av[na++] = strspl("-i",tmp3); 
			if (lflag) av[na++] = strspl("-l",setsuf(svslist[i],'l'));
			if (svsprog == FORT0)
				for (j=0;j<nff;j++) av[na++] = fflist[j];
			else
				for (j=0;j<npf;j++) av[na++] = pflist[j];
			av[na] = 0;

			if (callsys(0, (svsprog == FORT0)?fort0[target_machine_specification]:pascal[target_machine_specification], 
				    av, passnames[svsprog])) {
				cflag++;
				eflag++;
				continue;
			}
			
			if (timeflg) updatetime(&sttime,svsprog);
			/* at this point, the intermediate file is in tmp3 */
			cunlink(tmp1); cunlink(tmp2); cunlink(tmp4);

			/* invoke the code generator on the intermediate file */
			na = 0;
			av[na++] = passnames[FORT1];
			av[na++] = tmp3;
			if ((svsprog == FORT0)||(one_source_to_executable)||
			    (!preserve_dot_js))
				svsobjfile = tmp4;
			else 
				svsobjfile = setsuf(svslist[i],'j');

			av[na++] = svsobjfile;
			av[na] = 0;

			if (timeflg) times(&sttime);
			if (callsys(0, fort1[target_machine_specification], av, "  Fort1") > 1) {
				cflag++;
				eflag++;
				if (one_source_to_executable) cunlink(tmp6);
				cunlink(tmp3);
				continue;
			}
			if (timeflg) updatetime(&sttime,FORT1);
			/* at this point, the .j file is in tmp4 */
			cunlink(tmp3);

			/* do the prelinking, producing a .o file */
			na=0;
			av[na++] = "fort2";
			if (one_source_to_executable) {
				fortobj = tmp6;
			}
			else if ((one_source)&&cflag&&(outfile)) {
				fortobj = outfile;
			}
			else {
				fortobj = setsuf(svslist[i],binsuffix);
			}
			av[na++] = strspl("-o",fortobj);
			av[na++] = svsobjfile;
#ifdef NOTDEF
			if (dbgflag) {
				char *cptr;
				if (getsuf(fortobj))
					cptr = setsuf(fortobj,'d');
				else
					cptr = strspl(fortobj,".d");
				av[na++] = strspl("-d",cptr);
			}
#endif
			av[na++] = 0;
			/* and call the prelinker */
			if (timeflg) times(&sttime);
			if (callsys(0, fort2[target_machine_specification], av, "  Fort2") > 1) {
				cflag++;
				eflag++;
 				if (one_source_to_executable) cunlink(tmp6);
				cunlink(fortobj);
				cunlink(tmp4);
				goto nocom;
			}
			if (timeflg) updatetime(&sttime,FORT2);
			cunlink(tmp4);
			if (one_source_to_executable) {
				/* at this point, the .o file is in fortobj */
				for (j=nl;j>=0;--j)
		   			llist[j+1] = llist[j];
				nl++;
				llist[0] = fortobj;
			} 
			else {
				cunlink(tmp6);
				chmod(fortobj, 0666 ^ usermask);
				fortobj = 0;
			}
		}
	}

nocom:			/* link edit files in llist[0:nl-1] */
	if (cflag==0 && nl!=0) {
		/* if we are targetting for a JUNIPER, and we have FORTRAN or
		   Pascal, and we are generating debugging info, we have to
		   combine all of the .d files into the final .dbg file.
		*/
#ifdef NOTDEF
		if ((hasfortran||haspascal)&& (dbgflag)) {
			char *cptr;
			na = 0;
			av[na++] = cat;
			for (i=0;i<nl;i++) {
				if (getsuf(llist[i]))
					cptr = setsuf(llist[i],'d');
				else
					cptr = strspl(llist[i],".d");
				if (access(cptr,04) == 0)
					av[na++] = cptr;
			}
			if (na > 1) {
				cptr = outfile?(strspl(outfile,".dbg")):
					"a.out.dbg";
				av[na] = 0;
				callsys(cptr, cat, av, "Making dbg");	
				if (one_source_to_executable)
					cunlink(av[1]);
			}
			else 
	 			fprintf(stderr,
		"%s : Cannot generate .dbg file. - No .d files present." ,
					progstr);
		}
#endif
		na = 0;
		av[na++] = "ld";
		av[na++] = "-X";
		if (Rflag)
			av[na++] = "-r";
		if (entrypt) {
			av[na++] = "-e";
			av[na++] = entrypt;
		}
		if (Org) {
			av[na++] = "-T";
			av[na++] = Org;
		}
		if ((hwfpflag>0)) {
			av[na++] = "-T";
			av[na++] = "2000";
		}
		if (!noshareflag)
			av[na++] = "-n";
		
		/* startup */
		if (standalone) crt0 = crt0stand;
		av[na++] = (altlibpath >= 0)?
			add_libpath(crt0):crt0;


		if (outfile) {	/* else if outfile exists then */
			av[na++] = "-o";/* output to it.  Default is b.out */
			av[na++] = outfile;
		}
		for (i=0; i<nf; i++)	/* supply all flags */
			av[na++] = flist[i];
		for (i=0; i<nl; i++)	/* supply all .o arguments */
			av[na++] = llist[i];

#ifdef GL1
		if ((glflag||rglflag) &&  haspascal) {
			/* supply the pascal intcmd jumptable */
			av[na++] = (altlibpath >= 0)?
				add_libpath(pascalglue):pascalglue;
		}
		if (hasfortran) {
			/* supply the FORTRAN common block */
			av[na++] = (altlibpath >= 0)?
				add_libpath(fblkdat):fblkdat;
		}
#endif
#ifdef NOTDEF
		if (rglflag && hasfortran) {
			/* supply the C part of the 
			   fortran remote graphics library */
#ifdef GL1
			av[na++] = (altlibpath >= 0)?
				add_libpath("/usr/lib/librfgl.o"):
				"/usr/lib/librfgl.o";
#else
			av[na++] = (altlibpath >= 0)?
				add_libpath("/usr/lib/libf.o"):
				"/usr/lib/libf.o";
#endif
		}
		else 
#endif
		if (rglflag && /*!nsvs*/ !hasfortran)
			av[na++] = (altlibpath >= 0)?
				add_libpath("/usr/lib/librgl.a"):
				"/usr/lib/librgl.a";
		/* GB FIX! */
		if ((target_machine == JUNIPER) && hwfpflag)
		{
			char *cptr;
			if (proflag)
				cptr = juniperhw_p;
			else
				cptr = juniperhw;
			av[na++] = 
				(altlibpath >= 0)?(add_libpath(cptr)):cptr;

		}
		/* dont have to add the libpath, as they are -l's */
		if (hasfortran) 
			av[na++] = fortlib0[target_machine_specification];

		if (hasfortran || haspascal) {
			av[na++] = fortlib1[target_machine_specification];
			if (fortlib1 == hwfpfortlib1)
				av[na++] = swfpfortlib1[target_machine_specification];
		}

		if (standalone) 
			av[na++] = "-lstand";
		if (glflag && /*nsvs && */ hasfortran) {
			/* supply the fortran wrapper library */
			av[na++] = "-lfgl";
		}
		if (glflag) av[na++] = "-lgl"; 

/* dar edit force libm load, don't check &&(libmindex < 0)  on if. */

		if ((glflag||rglflag)) 
			av[na++] = ((hwfpflag>0))?libmhwfp[target_machine_specification]:"-lm";
		if (dbgflag) {
			av[na++] = "-lg";
		}
		if (!seenlibc) av[na++] = (proflag)?"-lc_p":"-lc";
	if (!noxflag)	       /* add -x by default unless --x given */
			av[na++] = "-x";
		av[na++] = 0;			/* argument delimiter */
		if (timeflg) times(&sttime);
		eflag |= callsys(0, ld, av, "Loading");	/* invoke ld */
		if (timeflg) updatetime(&sttime,LD);
		if (one_source_to_executable) cunlink(tmp6);
	}
	cunlink(fortobj);
	if (timeflg) reporttimes();
	dexit();
}

idexit()
{

	eflag = 100;
	dexit();
}

dexit()
{

	if (!pflag) {
		cunlink(tmp1);
		cunlink(tmp2);
		if (sflag==0)
			cunlink(tmp3);
		cunlink(tmp4);
		cunlink(tmp5);
	}
	exit(eflag);
}

error(s, x)
	char *s, *x;
{
	FILE *diag = exflag ? stderr : stdout;

	fprintf(diag, "%s: ",progstr);
	fprintf(diag, s, x);
	putc('\n', diag);
	exfail++;
	cflag++;
	eflag++;
}

getsuf(as)
char as[];
{
	register int c;
	register char *s;
	register int t;

	s = as;
	c = 0;
	while (t = *s++)
		if (t=='/')
			c = 0;
		else
			c++;
	s -= 3;
/*	if (c <= DIRSIZ && c > 2 && *s++ == '.')*/
	if (/*c <= DIRSIZ && */c > 2 && *s++ == '.')
		return (*s);
	return (0);
}

char *
setsuf(as, ch)
	char *as;
{
	register char *s, *s1;

	s = s1 = savestr(as);
	while (*s)
		if (*s++ == '/')
			s1 = s;
	s[-1] = ch;
	return (s1);
}

char *
setlongsuf(as, suff)
char *as;
char *suff;
{
	register char *s, *s1;
	register int suflen = strlen(suff);

	s = s1 = savestr(as);
	while (*s)
		if (*s++ == '/')
			s1 = s;
	s[-1] = 0;
/*	if (strlen(s1) > (DIRSIZ - suflen)) {
		s[-suflen] = 0;
		s[-(suflen-1)] = '.';
	}
*/
	return(strspl(s1,suff));
}


callsys(stdoutfile, f, v, msg)
	char *stdoutfile, *f, **v;
{
	int t, status;
	char cmd[256];
	
	if (*f!='/')
	  {
	      /*
	       * add the binary directory at the begining if not
	       * already specified, so you can have other versions
	       * in your path without screwing up.
	       */
	   strcpy( cmd, BinDirectory);
	   strcat( cmd, f);
	  }
	else strcpy( cmd, f);

	if (zflag) 
#ifdef	ECHOCMDS
	  {
	  	/*
		 * print out a trace of all commands executed
		 */
	    char **arg = v+1;
	    printf( "\t%s ", cmd);
	    while (*arg) printf( "%s ", *arg++);
	    if (stdoutfile != (char *)0)
		printf( "> %s",stdoutfile);
	    printf("\n");
	  }
#else
		printf("%s\n", msg);
#endif

#ifndef NOCALLSYS
	t = fork();
	if (t == -1) {
		printf("No more processes\n");
		return (100);
	}
	if (t == 0) {
		if (stdoutfile != (char *)0) {
			if( freopen(stdoutfile, "w", stdout) == (FILE *)0) {
				fprintf(stderr,
				   "%s: cannot open output .dbg file\n",
				   progstr);
				_exit(100);
			}
		}
		execvp( cmd, v);
		fprintf(stderr,"Can't find %s\n", cmd);
		_exit(100);
	}
	while (t != wait(&status))
		;
	if ((t=(status&0377)) != 0 && t!=14) {
		if (t!=2) {
			printf("Fatal error in %s\n", cmd);
			eflag = 8;
		}
		dexit();
	}
	return ((status>>8) & 0377);
#else
	return(0);
#endif

}

nodup(l, os)
	char **l, *os;
{
	register char *t, *s;
	register int c;

	s = os;
	if (getsuf(s) != binsuffix)
		return (1);
	while (t = *l++) {
		while (c = *s++)
			if (c != *t++)
				break;
		if (*t==0 && c==0)
			return (0);
		s = os;
	}
	return (1);
}

#define	NSAVETAB	1024
char	*savetab;
int	saveleft;

char *
savestr(cp)
	register char *cp;
{
	register int len;

	len = strlen(cp) + 1;
	if (len > saveleft) {
		saveleft = NSAVETAB;
		if (len > saveleft)
			saveleft = len;
		savetab = (char *)malloc(saveleft);
		if (savetab == 0) {
			fprintf(stderr, "%s: ran out of memory (savestr)\n",
				progstr);
			exit(1);
		}
	}
	strncpy(savetab, cp, len);
	cp = savetab;
	savetab += len;
	saveleft -= len;
	return (cp);
}

char *
strspl(left, right)
	char *left, *right;
{
	char buf[BUFSIZ];

	strcpy(buf, left);
	strcat(buf, right);
	return (savestr(buf));
}

reporttimes()
{	int totalu = 0,
    	    totals = 0,
	    i;
	
	for (i=0;i<=NPASSES;i++) {
		totals += stime[i];
		totalu += utime[i];
	}
	printf("	TIME REPORT\n\n");
	printf("total: user time = %d  system time = %d\n",totalu,totals);

	printf("PASSNAME      UTIME       %%        STIME         %%\n\n");
	for (i=0;i<=NPASSES;i++) {
		printf("%s      %-10.10d    %-6.2f    %-10.10d    %-6.2f\n",
		  passnames[i],utime[i],
		  ((totalu > 0) ? ((utime[i]/(float)totalu)*100.) : 0),
		  stime[i],
		  ((totals > 0) ? ((stime[i]/(float)totals)*100.) : 0));
	}
}

updatetime(intime,index)
	struct tms *intime;
	int index;
{
	struct tms timbuf2;
	times(&timbuf2);
	stime[index] += 
	   (timbuf2.tms_stime - intime->tms_stime) +
	   (timbuf2.tms_cstime - intime->tms_cstime);
	utime[index] +=
	   (timbuf2.tms_utime - intime->tms_utime) +
	   (timbuf2.tms_cutime - intime->tms_cutime);
/*	printf("pass # %d had stime=%6.6d utime=%6.6d",
	       index,stime[index],utime[index]);
*/
}			

setenv(oargc,newarglist)
char *newarglist[];
{
	char cputypestring[0x10];
	char *cc_opts, *getenv();
	char **curargptr;
	int  n_newargs = 0;

	if (!getversion(KVERS_CPUTYPE,cputypestring))
	{
		/* call succeeded.  the buffer is either
		   "ipII" or "pmII"
		*/
		if (cputypestring[0] == 'i') 
			target_machine = JUNIPER;
		else 
			target_machine = IRIS;
	}
	else {
		/* call failed, dont alter the default */
	}

#ifdef ENV_CFLAGS
	/* ok, see if there is a CC_OPTS environment variable set. */
	if ((cc_opts = getenv("CC_OPTS")) != (char *)0)
	{
		register char c;
		/* yes, one existed.  Get a copy */
		cc_opts = savestr(cc_opts);
		curargptr = &newarglist[oargc];
		while ((c = *cc_opts)&& (c != '-')) 
			cc_opts++;
		/* one less - for the terminator. */
		while ((*cc_opts) && (n_newargs < (MAXNEWARGS - 1))) 
		{
			/* add in the new argument */
			*curargptr++ = cc_opts;
			n_newargs++;

			/* find the end of the argument */
			while ((c = *cc_opts) && (!isspace(c)))
				cc_opts++;
			/* delimit it */
			if (*cc_opts) 
				*cc_opts++ = 0;
			/* and look for the start of the next one */
			while ((c = *cc_opts)&& (isspace(c))) 
				cc_opts++;
		}
		if (n_newargs == (MAXNEWARGS - 1))
			fprintf(stderr,
"%s: warning: CC_OPTS read from environment contained more than %d args - truncated.\n",
				progstr,(MAXNEWARGS-1));
		*curargptr = (char *)0;
		/* and update the number of arguments in argv */
		oargc += n_newargs;

	}
#endif
	return(oargc);
}




#ifdef NOTDEF
char *
add_libpath(str) 
char *str;
{
	/* alter juniper?w* strings. */
	char *cptr;
	int jstrlen;
	jstrlen = strlen(str) + 1;
	jstrlen += strlen(ldlibpath);
	cptr = (char *)malloc(jstrlen);
	if ((cptr = (char *)malloc(jstrlen)) <= 0) {
		fprintf(stderr, 
"%s: ran out of memory (savestr)\n",
			progstr);
		exit(1);
	}
	strcpy(cptr,str);
	return(cptr);
}
#endif
