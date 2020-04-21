

static char sccsid[] = "@(#)newobj.c 1.14 10/22/83";

static char rcsid[] = "$Header: /d2/3.7/src/usr.bin/dbx/RCS/newobj.c,v 1.1 89/03/27 17:44:41 root Exp $";

/*
 * Object code interface, mainly for extraction of symbolic information.
 */

#include "defs.h"
#include "object.h"
#include "main.h"
#include "symbols.h"
#include "names.h"
#include "languages.h"
#include "mappings.h"
#include "lists.h"
#include "newobj.h"
#include <a.out.h>
#include <stab.h>
#include <ctype.h>

extern	unsigned	long	codestart;
DBXHEADER	dbxhead;
Symbol	mainfunc;
Symbol		*symrefs;
char		*newstringtab;

rdbinst(f, dbxfiloff)
Fileid	f;
long	dbxfiloff;
{

	char		*malloc();
	struct	Symbol	s;		
	Symbol		sp;
	Symbol		np;
	long	rem;
	long	stringoff;
	long	i;
	Name	n;
	LINEHEADER	linehdr;
	LINEADDR	lineaddr;


	initsyms();
	if (read(f, &dbxhead, sizeof(DBXHEADER)) == -1) {
		panic("cannot read dbxheader\n");
	}

	symrefs = (Symbol *) malloc(sizeof(Symbol) * (dbxhead.nsyms + 1));
	newstringtab = (char *) malloc(dbxhead.stringsize);
	stringoff = lseek(f,0,1) + (dbxhead.nsyms*sizeof(struct Symbol));
	if (lseek(f, dbxhead.nsyms * sizeof(struct Symbol), 1) <= 0) {
		panic("cannot seek to string table\n");
	}
	
	if (read(f, newstringtab, dbxhead.stringsize - 1 ) == -1) {
		panic("cannot read string table\n");
	}
	
	if (lseek(f, sizeof(DBXHEADER) + dbxfiloff, 0) <= 0) {
		fprintf(stderr, "cannot seek to dbx symbol table\n");
		exit(200);
	}
	
	for (i = 0; i < dbxhead.nsyms; i++) {
		if (read(f, &s, sizeof(s)) == -1) {
			fprintf(stderr, "cannot read dbx header\n");
			exit(100);
		} else {

			if ((long) s.name == -1) {
				symrefs[s.symnum] = newSymbol((Name) 0, 
					curlevel, s.class, s.type, s.chain); 
				s.name = nil;
			} else if (s.class == FUNC && 
					s.symvalue.funcv.ismain == 1) {
				s.name = identname("main");
			} else {
				char	*nameptr;
				nameptr = ((char *) ((long) (newstringtab) 
					+ (long) s.name));
				if (strcmp(nameptr, "/ /") == 0) {
					nameptr = "//";
				}
				if (strcmp(nameptr, "NONAME") == 0) {
					nameptr = "NONAME/D";
				}
				s.name = identname(nameptr);

			}
			if (s.class == MODULE) {
				symrefs[s.symnum] = nil;
				enterSourceModule( s.name, (Address) 0);
				sp = &s;
			} else {
				symrefs[s.symnum] = insert(s.name);
				sp = symrefs[s.symnum];
				np = sp->next_sym;
				*(sp) = s;
				sp->next_sym = np;
				sp->language = curlang;
			}
			if (s.class == FUNC && s.symvalue.funcv.ismain == 1) {
				mainfunc = sp;
			}
			if (tracesyms) {
				fprintf(stderr, "\n---------------------------\n");
				if (s.name ) {
					fprintf(stderr, "name\t%s\n", 
						Ident(s.name)); 
				} else {
					fprintf(stderr, "name\t%s\n", "nil");
				}
		fprintf(stderr, "addr=%x\n", sp);
				fprintf(stderr, "symnum=%d\n", sp->symnum);
				fprintf(stderr, "lang\t%d\n", (sp->language));
				fprintf(stderr, "level\t%d\n", sp->level);
				fprintf(stderr, "class\t%s\n", classname(sp));
				fprintf(stderr, "type\t%d\n", sp->type);
		    		fprintf(stderr, "chain\t%d\n", sp->chain);
	    			fprintf(stderr, "block\t%d\n", sp->block);
				fprintf(stderr, "next %x\n", sp->next_sym);
			}
		}
	}
	for (i=1; i <= dbxhead.nsyms; i++) {
		if(symrefs[i]) {
			sp = symrefs[i];
			if (sp->type) {
				sp->type = symrefs[(long) sp->type];
			} else {
				sp->type = t_nil;
			}
			if (sp->class == COMMON) {
				Symbol	cp;
				sp->symvalue.common.offset = (long) 
				symrefs[(long) sp->symvalue.common.chain];
				sp->symvalue.common.chain = 
				(Symbol)sp->symvalue.common.offset;
				cp = sp;

				while (cp = cp->symvalue.common.chain) {
					cp->symvalue.common.chain = 
					symrefs[(long)cp->symvalue.common.chain];
				}
					
			}
			sp->chain = symrefs[(long) sp->chain];
			if (sp->class == FUNC || sp->class == PROC) {
				sp->block = curmodule;
			} else {
				sp->block = symrefs[(long) sp->block];
			}
		}
	}
	mainfunc->block = program;
}

