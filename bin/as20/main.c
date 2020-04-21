
/*
	MC68020 assembler - Silicon Graphics, INC.

	as20 is a virtual memory assembler for use on a paging system.  
	A single pass is made over the source code, allowing input to 
	be piped to it from the compiler.  Instructions from the
	source are parsed, and a binary representation 
	placed in memory.  Linked lists are managed 
	to indicate positions in the preliminary code stream which need
	patching, and these positions are patched once the symbol table has
	been fully assembled.

	Author: Greg Boyd, SGI.  This assembler was created from scratch.  No
	parts of it previously appeared in any other projects.  Parts of it
	were eventually used in the the ongoing mc68020 C compiler project.

	Begun: December, 1984.
	First Alpha Release: January, 1985.
	Usable for most applications: February, 1985.
	Completed:
*/

#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <fcntl.h>
#include "globals.h"
#include "scan.h"


#define MAXFILES 10
char *inputfile[MAXFILES] = {0};
char *outfile = "";
char *listfile = "";

int dolisting=0;	/* unimplemented */
int nipfiles=0;
int debug=0;		/* available only if compiled with -DDEBUG.  Activated
			   by the -d switch */
int sym_debug=0;	/* available only if compiled with -DDEBUG.  Activated
			   by the -s switch.  Dumps symbols and reloc info
			   at output time.
			*/
int isstdin=1,		/* input source.  Can be pipe, file, or stdin. */
    ispipe=0,
    isterm=0;

int Errors=0;
int stabsinsource = 0;	/* set if stabs seen */
int output = (-1);
#ifndef M68010_ONLY
/* default is SET */
int is68020 = 1;	/* if NOT set, 68020 instructions and opcodes 
			   cause warning/error messages */
#else
int is68020 = 0;	/* if NOT set, 68020 instructions and opcodes 
			   cause warning/error messages */
#endif
FILE *input = (FILE *)0;

/* id is created by mkdate and is in date.c */
extern char *id;

#define giveversion() fprintf(stderr,"%s",id);

main(argc,argv)
char **argv;
int argc;
{
	int i=1;
	int ipfileno,ftype;

	/* process arguments.  legal switches are:

		-o 	output filename follows.
		-d	debugging turned on.  Causes voluminous amounts
			of output.
		-s	symbol debugging turned on.  
		-i	IRIS (m68010) set is68020 to 0.
		-l	listing filename follows.  Currently no-op.
		-v	just give version and exit.

	   other arguments are filenames, which are in effect catenated.
	*/
	int justgiveversion=0;
	char machinechar;

	machinechar = argv[0][(strlen(argv[0]) - 2)];

	if (machinechar == '1') {
		is68020 = 0;
	}
	else if (machinechar == '2') {
		is68020 = 1;
	}
	while (i<argc)
	{
		if (argv[i][0] == '-')
		{
			switch (argv[i][1])
			{
			case 'l':
				/* listing file name follows */
				if (i <= argc)
				{
					listfile = argv[i]; dolisting++;
				}
				else
					fatal(0,
					   "listing filename must follow -l");
				break;

			case 'o':
				/* past switch */
				i++;
				if (i<=argc)
					outfile = argv[i];
				else 
					fatal((tokentype *)0,
						"output filename must follow -o");
				break;

			case 's': 
				sym_debug++;
				break;

			case 'd':
				debug++;
				break;

			case 'i':
				is68020 = 0;
				break;

			case 'j':
				is68020 = 1;
				break;

			case 'v':
				justgiveversion = 1;
				break;

			default:
				fatal((tokentype *)0,
					"unrecognized switch",argv[i]);
			}
			i++;
		}
		else
			inputfile[nipfiles++] = argv[i++];
	}

	if (justgiveversion)
	{
		giveversion();
		exit(-1);
	}

	/* pass through the files */
	/* open the output file and initialize */
	if ((output = open(outfile,(O_WRONLY|O_CREAT|O_TRUNC),0666)) < 0)
		fatal((tokentype *)0,"cant open output file %s",outfile);

	ipfileno=0;

	if (nipfiles == 0)
	{
		struct stat statbuf;
		/* check if the input is from a pipe or terminal */
		if (!fstat(fileno(stdin),&statbuf))
		{
			/* stat succeeded. input is stdin */
			isstdin++;
			/* is this a pipe? */
			ftype = statbuf.st_mode & S_IFMT;
			if (ftype == S_IFIFO)
				ispipe++;
			else if (ftype == S_IFCHR)
			{
				isterm++;
				giveversion();
			}
			else 
				fatal((tokentype *)0,
				   "<stdin> must be open to terminal or pipe");
			nipfiles++;
		}
	}
	else isstdin = 0;

	/* check that a listing is possible if selected */
	if ((dolisting ) && (ispipe|isterm))
	{
		/* a listing, when implemented, cannot be made from a pipe,
		   as we have to make a separate pass.
		*/
		warning(0,"cannot produce listing from piped or terminal input");
		dolisting=0;
	}

	initialize();
	while (ipfileno < nipfiles)
	{
		
		input = (FILE *)0;
		if (isstdin)
			input = stdin;
		else 
			input = fopen(inputfile[ipfileno],"r") ;

		if (input != (FILE *)0)
		{
			/* open successful -- process the file */

			init_file();
			while (do_statement()) ;
			last_statementno = linenum;
			fclose(input);
		}
		else
			error((tokentype *)0,
				"cannot open file %s",inputfile[ipfileno]);

		ipfileno++;
	}

	/* all the files are processed.  The internal representation
	   should be complete.  Generate the code.
	*/

	if (dolisting)
	{
		listing = fopen(listfile,"w");
		if (listing == (FILE *)0)
		{
			error (0,"cant open listing file");
			dolisting = 0;
		}
	}

	/* remove active temporary labels */
	delete_usertemps();
	/* generate the text and data segments */
	generate_code();
	/* generate the symbol table, string table, 
	   and relocation information */
	generate_symbols();
	/* write the header */
	generate_header();

#ifdef DEBUG	
	if(debug)
	{
		dump_symtab();
	}
	fprintf(stderr,"final break value: 0x%x\n",(int)sbrk(0));
#endif
	flush_errors();
	closeall();

	/* if there were errors (as opposed to just warnings), remove
	   the .o file
	*/
	if (Errors) unlink(outfile);
	exit(Errors?(-1):0);
}

initialize() 
{
	init_alloc();
	init_ps();
	init_sym();
	init_inst();
	init_statement();
}


init_file()
{
	/* set the current token and the lookahead token to NULL.
	   set the line pointers up so that get_token will refresh the
	   buffer
	*/
	init_scan();
	linenum = 0;
	compound_line = 0;

}

abort()
{
	flush_errors();
	closeall();

	unlink(outfile);
	exit(-1);
}
