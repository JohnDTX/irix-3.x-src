/* masdefs.h -- definitions for all .c files
 *
 */

#define MAXSTATES	4097	/* 0 doesn't count  */
#define SHORTSPERSTATE	4
#define MAXFIELDS	128

typedef short Param;	/* for enumerated types passed to user procs. */

typedef struct {	/* lower & upper bounds of enumerated type */
	short lb;	/* unused for pseudo-output  */
	short ub;
} Paramtype;

typedef struct {
	char name[25];		/* string for error reporting	*/
	short lsb;		/* position of lsb in output	*/
	short msb;		/* ditto msb			*/
	short defaultval;	/* assigned by Default() in endstate()	*/
	short outfield[MAXSTATES];	/* all values for this out field */
	char assigned[MAXSTATES];  /* flag -- whether fld already set	*/
} Output;

#define _NS	newstate();
				/* left-bracket for state	*/
#define _ES	endstate();
				/* right-bracket for state	*/
#define Setfield(fld,val)	setfield(&fld,val)

#define declare(adr)	_declare("adr");
#define blok(siz)	_blok(siz);
#define external	_external();
#define sarray(adr,ofs)	NextAssigned[state] = 1;	\
			nexts[state] = Assigned;	\
			NextAddress.outfield[state] = \
				scratchlookup("adr") + (ofs);
#define scratch(adr)	sarray(adr,0)


#define reloc(adr)	state = (adr);

#define label(lab)	install("lab",state); printf("%x\t%s\n",state,"lab");

#define next(lab)	NextAssigned[state] = 1; \
			nexts[state] = "lab";
	/* user routine for specifying next address */

#define const(val)	NextAssigned[state] = 1; \
			NextAddress.outfield[state] = val; \
			nexts[state] = Assigned; \
	/* user routine for specifying constant in next address field */

/* macros for endstate.c
 */
#define Default(field)	if (field.assigned[state]==0) \
				field.outfield[state] = field.defaultval;
#define Assert(field)	if (field.assigned[state]==0) \
				printass("field");
#define Exclude(a,b)	if ((a) && (b)) printexcl();

#define Is(a)		(a.assigned[state])
#define Val(a)		(a.outfield[state])
#define Eq(a,b)		(Val(a) ==(b))
#define Lt(a,b)		(Val(a) < (b))
#define Le(a,b)		(Val(a) <=(b))

#define Procname(name)	procname = "name";

#define Generate(fld)	fieldlist[flp++] = &fld;
