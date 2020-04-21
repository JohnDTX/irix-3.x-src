/* endstate.c  --- end-of-state checking
 */

#include <stdio.h>
#include "mas.h"
#include "fbcparams.h"

extern FILE *symf;
extern Output ccsel,i1,i2,seqop,DIsrc,seqtype,microconst;

endstate()
{

    if (bracket==0) {
	printf("brackets mismatch");
	fprintf(symf,"brackets mismatch");
	printstate();
    }
    bracket = 0;

/*  user stuff here   */

    if (Is(i1)) Assert(i2);
    Default(i1);
    Default(i2);

    if (Is(microconst))
	if (!Eq(DIsrc,UCONST) && !Eq(DIsrc,UCOUNT)) {
	    printf("       warning: use LOADDI(UCONST) w/ MICROCONST. ");
	    fprintf(symf,"     warning: use LOADDI(UCONST) w/ MICROCONST. ");
	    printstate();
	    LOADDI(UCONST);
    }
    if (Eq(DIsrc,UCONST)) {
	if ( !Is(ccsel) && !Is(seqop)) {
		COND(IFFALSE); SEQ(JUMP);
	}
	else if (Eq(seqtype,BRANCH)||Eq(seqtype,SPECIAL)) {
		if (!Eq(ccsel,7)) {
			printf("constant procedure without COND(IFFALSE)");
			fprintf(symf,"constant procedure w/o COND(IFFALSE)");
			printstate();
		}
	}
    }

    if (Eq(seqop,11) && !Eq(ccsel,7)) {
	printf("CJPP without COND(IFFALSE)");
	fprintf(symf,"CJPP without COND(IFFALSE)");
	printstate();
    }

    if (Eq(seqtype,BRANCH) && (NextAssigned[state]==0)) {	/* cheating */
		printf("Missing branch address");
		fprintf(symf,"Missing branch address");
		printstate();
	}

    if (!Is(seqop)) SEQ(CONT);		/* supply default */

/* check DI / SEQop compatibility */

    if (Is(DIsrc)) switch (Val(DIsrc)) {
	case UCONST:		if (Eq(seqtype,NONBRANCH)
				    || Eq(seqtype,SPECIAL)) DIerr(); break;
	case UCOUNT:		if (!Eq(seqtype,COUNTER)) DIerr(); break;
	case INLJUST:		if (!Eq(seqtype,NONBRANCH)) DIerr(); break;
	case INRJUST:		if (!Eq(seqtype,NONBRANCH)) DIerr(); break;
	case OUTPUTREG:		if (!Eq(seqop,11)) DIerr(); break;
	case OUTPUTCOUNT:	if (!Eq(seqtype,COUNTER)) DIerr(); break;
	case OUTPUTADDRESS:	if (!Eq(seqop,2)) DIerr(); break;
	case BPCDATA:		if (!Eq(seqop,11)) DIerr(); break;
	case MULTIBUS:		if (!Eq(seqop,2)) DIerr(); break;
    }
    else if (Eq(seqtype,COUNTER) || Eq(seqtype,SPECIAL)) {
	printf("       warning: unspecified DI bus use with special SEQ op,");
	fprintf(symf," warning: unspecified DI bus use with special SEQ op,");
	printstate();
    }

#ifdef DEBUG
    printf("state %d: NA assigned = %d outfield = %d nexts = %s\n",state,
		NextAssigned[state],NextAddress.outfield[state],
		nexts[state]);
#endif
}


DIerr()
{
	printf("Illegal DI bus selection / seq. op.");
	printf("\n    DIsrc=%d  seqop=%d  seqtype=%d ",Val(DIsrc),Val(seqop),
			Val(seqtype));
	fprintf(symf,"Illegal DI bus selection / seq. op.");
	fprintf(symf,"\n    DIsrc=%d  seqop=%d  seqtype=%d ",Val(DIsrc),
			Val(seqop),Val(seqtype));
	printstate();
}
