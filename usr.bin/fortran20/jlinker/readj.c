#include "defs.h"
#include <stdio.h>
#include "symbols.h"
#include "readj.h"
#include "getbytes.h"


BLOCKSTRS	blockstrs[] = {
			"HEAD1BLOCK",		0x80,
			"END1BLOCK",		0x81,
			"ENTRY1BLOCK",		0x82,
			"EXTERN1BLOCKFF",	0x83,
			"START1BLOCK",		0x84,
			"CODE1BLOCK",		0x85,
			"RELOC1BLOCK",		0x86,
			"EXTERN1BLOCKFT",	0x89,
			"FDDEF1BLOCKF",		0x8a,
			"FDREF1BLOCKF",		0x8c,
			"ENTRY1BLOCKT",		0xb0,
			"EXTERN1BLOCKTF",	0xb1,
			"FDFREL1BLOCK",		0xb3,
			"FDDEF1BLOCKT",		0xb4,
			"FDINIT1BLOCK",		0xb5,
			"FDREF1BLOCKT",		0xb6,
			"FDREL1BLOCK",		0xb7,
			"VARTYP1BLOCK",		0xa0,
			"BPOINT1BLOCK",		0xa1
} ;

char	*langstrs[] = {
			"PASCAL",
			"FORTRAN",
			"BASIC",
			"C",
};	


short	vflag;		/* verbose */
short	dbflag;		/* debug */
short	dbinfo;
Symbol	curfunc;
Symbol	curparam;
Symbol	curblock;
short	curlang;
int	curlevel = 1;
long	nesting;
Symbol	curmodule;
FILE	*ofilep;
FILE	*tfilep;
DBXHEADER	dbxhead;
Symbol	curcomm;
char	*tfile = "/tmp/jlxXXXXXX";

tstc(ofilestr, jfilestr)
char	*ofilestr;
char	*jfilestr;
{
	char	*jname;
	char	*oname;
	FILE	*jfilep;
	long	hdrloc;
	char	*mktemp();

	symbols_init();
	jname = malloc(jfilestr[0] + 1);
	strncpy(jname, jfilestr+1, jfilestr[0]);
	oname = malloc(ofilestr[0] + 1);
	strncpy(oname, ofilestr+1, ofilestr[0]);

	if ((jfilep = fopen(jname, "r")) == NULL) {

		fprintf(stderr, "cannot open %s\n", jname);
		exit(100);
	} else {
		readjfile(jfilep);
	}
	if (dbinfo) {
		tfile = mktemp(tfile);
		if ((tfilep = fopen(tfile, "w")) == NULL) {
			fprintf(stderr, "cannot open %s\n", tfile);
			exit(100);
		}
	
		dbxhead.nfiles = 1;
		symbol_dump(curfunc, tfilep);
		names_dump(tfilep);
		line_dump(tfilep);
		fclose(tfilep);
		if ((tfilep = fopen(tfile, "r")) == NULL) {
			fprintf(stderr, "cannot open %s\n", tfile);
			exit(100);
		}
		if ((ofilep = fopen(oname, "a+")) == NULL) {
			fprintf(stderr, "cannot open %s\n", oname);
			exit(100);
		}
		if (fseek(ofilep, 0, 2) != 0) {
			fprintf(stderr, "cannot seek back to dbx header\n");
			exit (100);
		}
		if (fwrite(&dbxhead, sizeof(DBXHEADER), 1, ofilep) != 1) {
			fprintf(stderr, "cannot write dbx header\n");
			exit (100);
		}
		cattmp(tfilep, ofilep);
		fclose(ofilep);
		fclose(tfilep);
		unlink(tfile);
	}
}


readjfile(jfilep)
FILE	*jfilep;
{

	int	blocktype;
	int	size;
	int	nextblock;

	while((blocktype = getc(jfilep)) != 0) {
		if ((blocktype < HEAD1BLOCK) || (blocktype > FDREL1BLOCK)) {
			fprintf(stderr, "Illegal blocktype %x at %o\n", 
				blocktype, ftell(jfilep));
			exit(500);
		}
		get3byts(jfilep, &size);
		if (dbflag) {
			fprintf(stderr, "	size=%d\n", size);
		}
		nextblock = size + ftell(jfilep) - 4;
		if ((nextblock % 2) != 0) {
			nextblock++;
		}
		switch (blocktype) {

		case VARTYP1BLOCK:
			vartypblck(jfilep, size);
			dbinfo = 1;
			break;

		case BPOINT1BLOCK:
			bpoint1blk(jfilep, size);
			dbinfo = 1;
			break;
		case HEAD1BLOCK:
		case END1BLOCK:
		case ENTRY1BLOCK:
		case EXTERN1BLOCKFF:
		case START1BLOCK:
		case CODE1BLOCK:
		case RELOC1BLOCK:
		case EXTERN1BLOCKFT:
		case FDDEF1BLOCKF:
		case FDREF1BLOCKF:
		case ENTRY1BLOCKT:
		case EXTERN1BLOCKTF:
		case FDFREL1BLOCK:
		case FDDEF1BLOCKT:
		case FDINIT1BLOCK:
		case FDREF1BLOCKT:
		case FDREL1BLOCK:
			if (dbflag) {
/*
				fprintf(stderr, "%s at %o not yet supported\n", 
					(char *) blocknm(blocktype), ftell(jfilep) - 1);
*/
				break;
			}
		}
		if (fseek(jfilep, nextblock, 0) != 0) {
/*
			fprintf(stderr, "cannot seek past %s\n",
				(char *) blocknm(blocktype));
*/
			exit(300);
		}
	}
}

vartypblck(jfilep, size)
FILE	*jfilep;
int	size;
{
	PROCINFO	procdesc;
	char	*filename;
	Name	identname();

	filename = getnm(jfilep);
	srcf.filename = (long) identname(filename);
	getproc(jfilep, &procdesc);
	if (vflag) {
		dumpproc(&procdesc);
		psym(curfunc);
	}
	gettypes(jfilep);
	getvars(jfilep);
	if (vflag) {
		fortran_printdecl(curfunc);
	}
}

getproc(jfilep, procptr)
FILE	*jfilep;
PROCINFO	*procptr;
{
	long	offset;
	char	*getnm();
	long	ftell();	
	Name	fname;
	short	i;
	int	j;

	if (fread((char *) procptr, 5, 1, jfilep) != 1) {
		fprintf(stderr, "cannot read procedure information\n");
		exit(600);
	}
	i = getshort(jfilep);
	procptr->pr_linknm = getnm(jfilep);
	i = getshort(jfilep);
	j = getc(jfilep);
	procptr->pr_outlinknm = getnm(jfilep);
	fname = identname(procptr->pr_linknm);
	curparam = curfunc = curblock = insert(fname);
	curfunc->language = procptr->pr_lang;
	if (procptr->pr_retlev == SVSPROC) {
		curfunc->class = PROC;
	} else if (procptr->pr_retlev == SVSMAIN) { 
		curfunc->class = FUNC;
		curfunc->symvalue.funcv.ismain = 1;
	} else {
		curfunc->class = FUNC;
	}
	curfunc->storage = EXT;
	curfunc->level = curlevel; 
	curfunc->type = (Symbol) (0);
	curfunc->block = program;
	curfunc->symvalue.funcv.src = 1;
}
	
	
gettypes(jfilep)
FILE	*jfilep;
{

	short	typeno = 1;
	int	typekind;
	Symbol	newtype;
	Symbol	getforarr();
	int	i;
	
	if (vflag) {
		fprintf(stderr, "TYPES:\n");
	}
	while (typeno = getshort(jfilep)) {
		if (vflag) {
			fprintf(stderr, "typeno=%d:", typeno);
		}
		switch(typekind = getc(jfilep)) {
		case SVSFORTARR:
			typetable[typeno] = getforarr(jfilep);
			if (vflag) {
				dump_array(typetable[typeno]);
			}
			break;
		case 9:
			i = getshort(jfilep);
			break;
		default:
			break;
		}
	}

	if (typeno != 0) {
		fprintf(stderr, "cannot read type information\n");
		exit(600);
	}
	
}

Symbol
getforarr(jfilep)
FILE	*jfilep;
{

	SVSFORARR	*arryptr;
	SVSFORDIM	*dimptr;
	int	numdims;
	SVSFORDIM	*odimptr;
	Symbol	newarry;
	Symbol	makedimens();

	arryptr = (SVSFORARR *) malloc(sizeof(SVSFORARR));
	arryptr->numdims = getc(jfilep);
	arryptr->typeno = getshort(jfilep);
	
	arryptr->svsdimp = (SVSFORDIM *)
				malloc(arryptr->numdims * sizeof(SVSFORDIM));
	dimptr = arryptr->svsdimp;
	odimptr = nil;
	for (dimptr = &(arryptr->svsdimp[0]); 
		dimptr < &(arryptr->svsdimp[arryptr->numdims]); dimptr++) {
		dimptr->nextdim = odimptr;
		dimptr->flags = getc(jfilep);
		dimptr->lobound = getw(jfilep);
		dimptr->hibound = getw(jfilep);
		dimptr->elemsize = getw(jfilep);
		odimptr = dimptr;
	}
	newarry = makedimens(odimptr, arryptr->typeno);
	if (vflag) {
		dumpforarr(arryptr);
	}
	return(newarry);
}
	


getvars(jfilep)
FILE	*jfilep;
{
	char	namelen;
	short	vartype;
	char	*varname;
	int	varloc = -1;
	int	varclass;
	char	*lname = (char *) 0;
	short	shortaddr;
	long	longaddr;
	Name	symnme;
	Symbol	varsym;
	Name	commname;
	Symbol	commsym;
	char	*tmpname;

	if (vflag) {
		fprintf(stderr, "VARIABLES\n");
	}
	while ((fread((char *) &namelen, sizeof(namelen), 1, jfilep) == 1)
			&& (namelen != (char) 0)) {
		if (fseek(jfilep, -1, 1) != 0) {
			fprintf(stderr, "cannot seek\n");
			exit(600);
		}
		varname = getnm(jfilep);
		vartype = getshort(jfilep);
		varclass = getc(jfilep);
		symnme = identname(varname, true);
		varsym = makesymbol(symnme, 0, VAR, gettype(vartype), nil);
		switch (varclass % 16) {
		case 1: /* Stack parms */
			varsym->class = REF;
			curparam->chain = varsym;
			curparam = varsym;

		case 0: /* Stack locals */
			varsym->storage = STK;
			varsym->symvalue.offset = varloc = getshort(jfilep);
			varsym->block = curfunc;
			break;

		case 3:	/* Address Register locals */
			if (vartype < 0 && (vartype > -100)) {
				varsym->class = REF;
			}
			varsym->storage = INREG;
			varsym->symvalue.offset = (varclass >> 4) + 1;
			varsym->block = curfunc;
			break;

		case 2:	/* Numeric Register locals */
			varsym->storage = INREG;
			varsym->symvalue.offset = (varclass >> 4);
			varsym->block = curfunc;
			break;

		case 4:	/* Common */
			shortaddr = getshort(jfilep);
			lname = getnm(jfilep);
			if (lname[0] != '/') {
				tmpname = (char *) 
					malloc(strlen(lname) + 3);
				strcpy(tmpname, lname);
				lname = strcat(tmpname, "/D");
			}
			commname = identname(lname);
			if ((commsym = lookup(commname)) == NULL) {
				commsym = makesymbol(commname, 1, COMMON, NULL,
						NULL);
				commsym->block = 0;
				if (vflag) {
					fprintf(stderr, "\tCOMMON:\n");
					psym(commsym);
				}
				commsym->symvalue.common.chain = 
					(Symbol) varsym->symnum;
			} else {
				(commsym->type)->symvalue.common.chain =
					(long) varsym->symnum;
			}
			commsym->type = varsym;
			curcomm = varsym;
			varsym->storage = EXT;
			varsym->block = curfunc;
			varsym->symvalue.common.offset = varloc = getw(jfilep);
			varsym->symvalue.common.chain = 0;
			break;
		case 8: /* Register Parameter */
			varsym->class = REF;
			varsym->storage = INREG;
			varsym->symvalue.offset = (varclass >> 4) + 1;
			varsym->block = curfunc;
			curparam->chain = varsym;
			curparam = varsym;
			break;
		default:
			break;

		}
		if (vflag) {
			fprintf(stderr, "_________________________________________________\n");
			if (varclass == 4) {
				if (lname) {
					fprintf(stderr, " lname=%s", lname);
				}
				fprintf(stderr, " shortaddr=%d longaddr=%d\n",
					shortaddr, longaddr);
			}
			fprintf(stderr, "\n");
			psym(varsym);
			fortran_printdecl(varsym);
		}
	}

	if (namelen != 0) {
		fprintf(stderr, "cannot read variable information\n");
		exit(600);
	}
}

	



dumpproc(procptr)
PROCINFO	*procptr;
{

	fprintf(stderr, "FUNCTION:");
	fprintf(stderr, "link name = %s\n", procptr->pr_linknm);
	fprintf(stderr, "\touter link name = %s\n", procptr->pr_outlinknm);
	fprintf(stderr, "\tlanguage=%s\n", langstrs[procptr->pr_lang]);
	fprintf(stderr, "\tlevel=%d\n", procptr->pr_level);
}
	

char *
blocknm(blocktype)
int	blocktype;
{

	int	i;

	for (i = 0; i < NBLKNMS; i++) {
		if (blockstrs[i].blockid == blocktype) {
			return(blockstrs[i].blockname);
		}
	}
	fprintf(stderr, "illegal block type %x\n", blocktype);
	return((char *) 0);
}

dumpforarr(arryptr)
SVSFORARR	*arryptr;
{
	SVSFORDIM	*dimptr;

	
	fprintf(stderr, "\tFOR ARRAY:dims=%d type=%d\n", arryptr->numdims,
		 arryptr->typeno);
	dimptr = arryptr->svsdimp;
	for (dimptr = &(arryptr->svsdimp[0]); 
		dimptr < &(arryptr->svsdimp[arryptr->numdims]); dimptr++) {
		fprintf(stderr, "	dim:lo=%d hi=%d esize=%d\n",
			dimptr->lobound, dimptr->hibound, dimptr->elemsize);
	}
}




cattmp(infd, outfd)
FILE	*infd;
FILE	*outfd;
{

	long	bytes_read;
	char	buffer[BUFSIZ];

	while ((bytes_read = fread(buffer, 1, BUFSIZ, infd)) == BUFSIZ) {
		fwrite(buffer, 1, BUFSIZ, outfd);
	}
	if (bytes_read) {
		fwrite(buffer, 1, bytes_read, outfd);
	}
}

