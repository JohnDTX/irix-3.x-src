/*
 *
 *     sort.c - This program does a quick sort of its input, 
 *		writing the sorted list to the named output
 *		file.  Each sort element comprises an entire line
 *		in both the input and the output.  This program
 *		uses the qsort() package of the C library.
 *
 *	usage:
 *		sort [-i] -o ofile ifile
 *
 *	   where
 *		ofile	is the name of the output file.
 *		ifile	is the name of the input file.
 *		-i	is used if the case of characters
 *			   is to be ignored (e.g. 'A' == 'a').
 *
 *
 */

/* ~@
#include <stdio.h>

/* The maximum length of an input record ~@
#define MAXRECLEN 0x50

/* The maximum number of input records ~@
#define MAXRECS 0x100

/* The length of a read buffer ~@
#define BUFLEN  0x400

/* macros for case-insensitive comparisons ~@
#define iscap(c)  (((c)>='A')&&((c)<='Z'))
#define lowercase(c)	(c-'A'+'a')

@~ */

/* the input and output file names */
char *outputfile, *inputfile;

/* the status of the ignore-case option. (nonzero -> ignore case.)  */
int ignorecase = 0;

/* an array of input records */
char *rec[MAXRECS];

main(argc,argv)
char **argv;
int argc;
{
	/* a dummy counter.  Used as index into argv[] */
	int i=1;
	/* curarg is the current command-line argument */
	char *curarg;
	/* file pointers for the input and output files */
	FILE *ip,*op;
	/* the number of records being processed */
	int nrecs;
	/* our routine to compare two records */
	int compare_recs();


	/* process the arguments.  legal switches are:

		-o <filename> 	output filename.
		-i		ignore case

	   the only other legal argument is the input filename.

	   Example:
		sort -i input -o output

	   would sort input onto output and ignore case during
	   the sort.

	*/

	while (i<argc)
	{
#if BUGLEV>=1
		/* get the current arg */
		curarg = argv[i];
#endif
		/* is it a switch? */
		if ( *curarg == '-')
		{
			/* yes.  process the switch */
			switch (curarg[1])
			{
			case 'i':
				/* ignoring case. */
				ignorecase++;
#if BUGLEV<3
				/* bump the counter */
				i++;
#endif
				break;

			case 'o':
				/* the output file name follows */
#if BUGLEV>=2
				/* increment past the switch.  The
				   next argument is the name of the output
				   file
				*/
				i++;
#endif
				if (i<argc) 
					/* get the output filename */
					outputfile = argv[i];
				else 
					/* ran out of arguments */
					fatal(
					   "output filename must follow -o");
				break;

			default:
				/* no other switches are recognized */
				fatal("unrecognized switch (%s)",curarg);
			}
		}
		else  {
			/* the argument isn't a switch.  It must be
			   the name of the input file 
			*/
			inputfile = curarg;
		}
		/* and bump our counter past the switch or filename */
		i++;

	}

	/* open the files */
	if ((ip = fopen(inputfile,"r")) == NULL) {
		fatal("cant open input file %s",inputfile);
	}
	if ((op = fopen(outputfile,"w")) == NULL) {
		fatal("cant open output file %s",outputfile);
	}

	/* read the input file. */
	nrecs = readrecs(ip);

	/* do the sort */
	printf("\nsorting...");

	/* qsort takes arguments:
		the array of sort records (rec)
		the number of records (nrecs)
		the size of an element (char *)
		the address of a routine to pass pointers to two
			elements to compare them (compare_recs)
	*/
	qsort(rec,nrecs,sizeof(char *),compare_recs);

	/* write out the result */
	for (i=0;i<nrecs;i++) 
		fputs(rec[i],op);

	printf("\n%d records sorted from input file %s onto output file %s\n",
		nrecs,inputfile,outputfile);

	return(0);
	
}


int
readrecs(ip)
FILE *ip;
{
	/* read the input file and fill in the array of
	   records to be sorted.  

	*/
	int nextrecno=0;
	int len;
	char *curbuf,*curbufend;
	char *fgets();

	do {

		/* allocate a new buffer */
		if ((curbuf = (char *)malloc(BUFLEN)) == NULL) 
			fatal("out of string space");
		
		curbufend = &curbuf[BUFLEN - MAXRECLEN];

		while (curbuf < curbufend) {

			/* have we read too many records? */
			if (nextrecno == MAXRECS) 
				fatal("too many records in input. Max %d.",
					MAXRECS);

			/* get the next record */
			if ((curbuf = fgets(curbuf,MAXRECLEN,ip)) == NULL) 
				/* finished. */
				break;

			/* get the length. */
			len = strlen(curbuf);

			/* if fgets truncated the record, abort */
			if (len >= MAXRECLEN) 
				fatal("record #%d too long.  Max %d chars.\n",
					(nextrecno + 1),MAXRECLEN);

			rec[nextrecno++] = curbuf;
			curbuf += (len+1);
		}

	} while (curbuf != NULL) ;
	
	return(nextrecno);
}

compare_recs(rec0,rec1)
char **rec0, **rec1;
{
	/* compare_recs() is called by qsort().  qsort() passes
	   it pointers to two records (a record for us is a char *,
	   so a pointer to it is a char **), and expects it to 
	   return an integer value 0 if equal, > 0 if the first record
	   is 'greater' than the second, < 0 if the first record is 'less'
	   than the second.
	*/

	/* buffers for lowercasing records.  Used if ignoring case. */
	char tempbuf0[MAXRECLEN],tempbuf1[MAXRECLEN];
	if (ignorecase) {
		/* lowercase the records, and place the result in 
		   the temporary buffers.
		*/
		lower(tempbuf0,*rec0);
		lower(tempbuf1,*rec1);
		return(strcmp(tempbuf0,tempbuf1));
	}
	else
#if BUGLEV<4
		return(strcmp(rec0,rec1));
#else
		return(strcmp(*rec0,*rec1));
#endif
}

lower(result,input)
char *result,*input;
{
	/* lowercase the character string input, placing the
	   result in the string result.
	*/
	char c;
#if BUGLEV<=4
	while (*input) {
		if (iscap(c = *input++))
#else
	while (c = *input++) {
		if (iscap(c))
#endif
			*result++ = lowercase(c);
		else
			*result++ = c;
	}
	*result = 0;
}

/* our general fatal error message reporter */
fatal(fmt,a,b,c,d,e)
char *fmt;
{

	fprintf(stderr,"sort: ");
	fprintf(stderr,fmt,a,b,c,d,e);
	fputc('\n',stderr);
	exit(-1);
}
