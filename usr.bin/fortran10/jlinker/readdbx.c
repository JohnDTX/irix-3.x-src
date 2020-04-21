
#include "defs.h"
#include <stdio.h>
#include "symbols.h"
#include "readj.h"
#include "getbytes.h"

char	*langstrs[] = {
			"PASCAL",
			"FORTRAN",
			"BASIC",
			"C",
};	

private String clname[] = {
    "bad use", "constant", "type", "variable", "array", "@dynarray",
    "@subarray", "fileptr", "record", "field",
    "procedure", "function", "funcvar",
    "ref", "pointer", "file", "set", "range", "label", "withptr",
    "scalar", "string", "program", "improper", "variant",
    "procparam", "funcparam", "module", "tag", "common", "extref", "typeref"
};

DBXHEADER	dbxhead;

main(argc, argv)
int	argc;
char	*argv[];
{

	int	i;
	int	j;
	FILE	*dfilep;
	
	setbuf(stdout, NULL);
	for (i=1; i<argc; i++) {
		if (argv[i][0] == '-') {
			j = 1;
			while (argv[i][j] != '\0') {
				switch(argv[i][j++]) {
				default:
					break;
				}
			}
		} else if ((dfilep = fopen(argv[i], "r")) == NULL) {
			fprintf(stderr, "cannot open %s\n", argv[i]);
			exit(100);
		} else {
			readdfile(dfilep);
		}
	}
}
			
readdfile(dfilep)
FILE	*dfilep;
{

	struct	Symbol	s;
	String	classname();
	int	nsyms;
	char	*stringtab;
	LINEHEADER	linehdr;
	LINEADDR	lineaddr;

	if (fread(&dbxhead, sizeof(DBXHEADER), 1, dfilep) != 1) {
		fprintf(stderr, "cannot read dbx header\n");
		exit(100);
	}
	if (fseek(dfilep, dbxhead.nsyms * sizeof(struct Symbol), 1) != 0) {
		fprintf(stderr, "cannot seek to string table\n");
		exit(200);
	}
	
	stringtab = (char *) malloc(dbxhead.stringsize);
	if (fread(stringtab, dbxhead.stringsize - 1, 1, dfilep) != 1) {
		fprintf(stderr, "cannot read dbx header\n");
		exit(100);
	}

	if (fseek(dfilep, sizeof(DBXHEADER), 0) != 0) {
		fprintf(stderr, "cannot seek to string table\n");
		exit(200);
	}
	
	while (dbxhead.nsyms--) {
		if (fread(&s, sizeof(s), 1, dfilep) != 1) {
			fprintf(stderr, "cannot read dbx header\n");
			exit(100);
		} else {
			int	ltmp;
			printf("\n---------------------------\n");
			ltmp = (int) s.name;
			if (ltmp > 0) {
				printf("name\t%s\n", (int) stringtab + ltmp);
			} else {
				printf("name\t%s\n", "nil");
			}
			printf("symnum=%d\n", s.symnum);
			printf("lang\t%s\n", language_name(s.language));
			printf("level\t%d\n", s.level);
			printf("class\t%s\n", classname(&s));
			printf("type\t%d\n", s.type);
		    	printf("chain\t%d\n", s.chain);
			printf("storage=%x\n", s.storage);
	    		printf("block\t%d\n", s.block);
			printf("offset=%x\n", s.symvalue.offset);
		}

	}
	fseek(dfilep, dbxhead.stringsize, 1);
/*
	while (dbxhead.nlinetabs--) {
		if (fread(&linehdr, sizeof(LINEHEADER), 1, dfilep) != 1) {
			fprintf(stderr, " cannot read line header\n");
			exit(100);
		}
		printf("Function:%d nlines:%d\n", linehdr.funcnum,
				 linehdr.nlines);
		while (linehdr.nlines--) {
			if (fread(&lineaddr, sizeof(LINEADDR), 1, dfilep) 
					!= 1) {
				fprintf(stderr, " cannot read line/addrss\n");
				exit(100);
			}
			printf("\tline:%d offset:%x\n", lineaddr.lineno,
					lineaddr.addroff);
		}
	}
				
			
*/

}

public String classname(s)
Symbol s;
{
    return clname[ord(s->class)];
}


#if All
/*
 * Straight dump of symbol information.
 */

public psym(s)
Symbol s;
{
    printf("name\t%s at %x\n", symname(s), s);
    printf("lang\t%s\n", language_name(s->language));
    printf("level\t%d\n", s->level);
    printf("class\t%s\n", classname(s));
	printf("symnum=%d\n", s->symnum);
    printf("type\t0x%x", s->type);
    if (s->type != nil and s->type->name != nil) {
	printf(" (%s)", symname(s->type));
    }
    printf("\nchain\t0x%x", s->chain);
    if (s->chain != nil and s->chain->name != nil) {
	printf(" (%s)", symname(s->chain));
    }
    printf("\nblock\t0x%x", s->block);
    if (s->block != nil and s->block->name != nil) {
	printf(" (");
	printname(stdout, s->block);
	putchar(')');
    }
    putchar('\n');
    switch (s->class) {
	case TYPE:
	    printf("type:size\t\n" /*,size(s)*/);
	    break;

	case VAR:
	case REF:
	    switch (s->storage) {
		case INREG:
		    printf("reg\t%d\n", s->symvalue.offset);
		    break;

		case STK:
		    printf("offset\t%d\n", s->symvalue.offset);
		    break;

		case EXT:
		    printf("address\t0x%x\n", s->symvalue.offset);
		    break;
	    }
	    printf("size\t%d\n" /*,size(s)*/);
	    break;

	case RECORD:
	case VARNT:
	    printf("size\t%d\n", s->symvalue.offset);
	    break;

	case FIELD:
	    printf("offset\t%d\n", s->symvalue.field.offset);
	    printf("size\t%d\n", s->symvalue.field.length);
	    break;

	case PROG:
	case PROC:
	case FUNC:
	    printf("address\t0x%x\n", s->symvalue.funcv.beginaddr);
	    if (isinline(s)) {
		printf("inline procedure\n");
	    }
	    if (nosource(s)) {
		printf("does not have source information\n");
	    } else {
		printf("has source information\n");
	    }
	    break;

	case RANGE:
	    prangetype(s->symvalue.rangev.lowertype);
	    printf("lower\t%d\n", s->symvalue.rangev.lower);
	    prangetype(s->symvalue.rangev.uppertype);
	    printf("upper\t%d\n", s->symvalue.rangev.upper);
	    break;

	default:
	    /* do nothing */
	    break;
    }
}

private prangetype(r)
Rangetype r;
{
    switch (r) {
	case R_CONST:
	    printf("CONST");
	    break;

	case R_ARG:
	    printf("ARG");
	    break;

	case R_TEMP:
	    printf("TEMP");
	    break;

	case R_ADJUST:
	    printf("ADJUST");
	    break;
    }
}

/*
 * Print out the name of a symbol.
 */

public printname(f, s)
File f;
Symbol s;
{
    if (s == nil) {
	fprintf(f, "(noname)");
    } else if (s == program) {
	fprintf(f, ".");
/*
    } else if (isredirected() or isambiguous(s)) {
	printwhich(f, s);
*/
    } else {
	fprintf(f, "%s", symname(s));
    }
}

#endif
public String language_name(lang)
short	lang;
{
    return (lang == nil) ? "(nil)" : langstrs[lang];
}

