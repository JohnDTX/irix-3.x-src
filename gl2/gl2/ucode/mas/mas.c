/* mas.c
 *	the important routines for microcode assembler
 *
 *  define CARRAY to spit out initialized ucode C array
 */

#include <stdio.h>
#include "masdefs.h"

extern Output NextAddress;	/* initialized by user		*/
short nstates = 0;	/* state counter			*/
extern short declstate;		/* from defines.c		*/
short state;		/* state pointer			*/
short filestate;	/* ditto for each user code file	*/
short filecounter = 0;	/* no. of new files			*/
char filename[80];	/* name of current c file		*/
short bracket = 0;	/* bracket matching flag: 1=inside	*/
char *nexts[MAXSTATES];	/* storage for unresolved labels	*/
char *Assigned;		/* default for nexts[state] if const() used	*/
char NextAssigned[MAXSTATES];	/* whether NextAddress assigned per state */
char *procname;		/* for passing procedure name for error rept */

unsigned short _ram[MAXSTATES][SHORTSPERSTATE];
Output *fieldlist[MAXFIELDS];
short flp = 0;

#ifdef VAX
unsigned char _startb[] = {
	7,1, 0,0, 0,0, 0,0, 8,0x80, 0,0, 0,0, 0,0,
	0xe,0, 0,0, 0,0, 0,0, 0,0, 0,0, 0,0, 0,0
};		/* unsigned short ???[4096*4] */

unsigned char _endb[] = {
	0,0, 0,0, 0,0, 0,0, 0x23,0, 0,0, 0,0, 0,0,
/*	0x75,0x63, 0x6f,0x64, 0x65,0*/
	0x5f,0x75,0x63,0x6f,0x64,0x65,0,0
};		/* ucode[][] */
#endif VAX
#ifdef UNIX
unsigned char _startb[] = {
	0,0, 1,7, 0,0, 0,0, 0,0, 0x80,0, 0,0, 0,0,
	0,0, 0,0x18, 0,0, 0,0, 0,0, 0,0, 0,0, 0,0,
};
unsigned char _endb[] = {
	0,0, 0,4, 7,0, 0,0, 0,0, 0,0,
	0,0, 0,0xb, 4,0, 0,0, 0,0, 0,0, 0,0, 0,0x10,
	0x5f,0x75,0x63,0x6f,0x64,0x65, 0,0x2e, 0x4c,0x4c, 0x30,0
};
#endif UNIX

FILE *outf,*symf;


fsm()
{
    printversion();
    if ((symf = fopen("ucode.sym","w")) == NULL)
	printf("Can't open ucode.sym for output\n");
    
    state = filestate = 0;
    fprintf(symf,"\n/*\n");
}


endfsm()
{
	short i;
	register Output *op;
	register unsigned short mask;
	register short j;
	short word, bit, nbits;
	extern short nscratchsyms;
	register unsigned char *pram;

/* resolve labels	*/

    for (i=0; i<MAXSTATES; i++)
	{
	    if (NextAssigned[i] ==0)
		{
			NextAddress.outfield[i] = i;	/* default	*/
		}
	    else
		{
		    if (nexts[i] != Assigned)	/* i.e. to a const */
			    NextAddress.outfield[i] = find(nexts[i],i);
		}
	}
/* generate the output file */

    fprintf(symf,"\n%d (%x hex) states programmed\n",nstates,nstates);

/* output the scratch symbol table */

    printsymtab(nscratchsyms);

    for (i=0; i<flp; i++)
	{
	    op = fieldlist[i];
	    if ((nbits = op->msb - op->lsb) > 16) {
		 printf("%s is too wide\n",op->name);
		 fprintf(symf,"%s is too wide\n",op->name);
	    }
	    word = op->lsb / 16;	/* word containing the lsb */
	    bit = op->lsb - (word<<4);	/* bit posn in word	*/
	    mask = ~(   ( (1 << (nbits+1)) -1) <<bit   );
			/* all bits not influenced by this field */
#ifdef DEBUG
	    printf("%s  mask=%04x word= %d lsb= %x\n",op->name,mask,word,bit);
#endif
	    for (j=0; j < MAXSTATES; j++)
		_ram[j][word] = (_ram[j][word] & mask)
				 | (op->outfield[j] << bit);
	}

/* spit it out	*/

#ifdef CARRAY
    printf("*/\n\nunsigned short ucode[%d][4] = {",MAXSTATES);
    for (j=1; j<=MAXSTATES-1; j++)
	{
	    printf("\n{ 0x%04x,\t0x%04x,\t0x%04x,\t0x%04x }\t/* %x */",
		_ram[j][0], _ram[j][1], _ram[j][2] ,_ram[j][3],j-1);
	    if (j<MAXSTATES-1) putchar(',');
	} 
    printf("\n};\n");
#endif

    if ((outf = fopen("ucode.o","w")) == NULL)
	printf("Can't open ucode.o for output\n");

    for (i=0; i<sizeof(_startb); i++)
	putc(_startb[i],outf);

#if VAX & LOCAL
    for (pram = (unsigned char *)&_ram[1][0], j=0; j<(MAXSTATES-1)*4; j++) {
	mask = *pram++;
	putc(*pram++,outf);
	putc(mask,outf);
    }
#else /* UNIX on ws -- or VAX REMOTE */
    for (pram = (unsigned char *)&_ram[1][0], j=0; j<(MAXSTATES-1)*4; j++) {
	putc(*pram++,outf);
	putc(*pram++,outf);
    }
#endif

    for (i=0; i<sizeof(_endb); i++)
	putc(_endb[i],outf);

    fclose(outf);
}


newstate()
{
	if (declstate > 0) {
		_declare("");
		declstate = 0;
	}
	if (state >= MAXSTATES)
		printf("Too many states for this machine.\n");
	else {
		++nstates;
		++state;
	}
	++filestate;
	if (bracket==1)
	    {
		fprintf(symf,"brackets mismatch,");
		printf("brackets mismatch,");
		printstate();
	    }
	bracket = 1;
}


Typecheck(param,paramtype)
    short param;		/* see if this number...	*/
    Paramtype paramtype;	/* is a member of this type	*/
{
    if (((unsigned short)param < (unsigned short)paramtype.lb)
	 || ((unsigned short)param > (unsigned short)paramtype.ub))
	{
	    fprintf(symf,"%s: param out of range,",procname);
	    fprintf(symf,"\nlb:%x ub:%x val:%x ",paramtype.lb,
							paramtype.ub,param);
	    printf("%s: param out of range,",procname);
	    printf("\nlb:%x ub:%x val:%x ",paramtype.lb,paramtype.ub,param);
	    printstate();
	}
}


printstate()
{
	fprintf(symf," state 0x%x, %d in file\n",state-1,filestate);
	printf(" state 0x%x, %d in ",state-1,filestate);
	printf(filename);
	putchar('\n');
}


setfield(field,val)
    register Output *field;
    short val;
{
#ifdef DEBUG
printf("setfield: \n");
#endif
    if ((field->assigned[state]) && (field->outfield[state]!=val))
	{
	    fprintf(symf,"%s: output %s mult assigned,",procname,field->name);
	    printf("%s: output %s mult assigned,",procname,field->name);
	    printstate();
	    return(0);
	}
    if (val >= (2 << (field->msb - field->lsb)))
	{
	    fprintf(symf,"%s: output %s value out of range,",
							procname,field->name);
	    printf("%s: output %s value out of range,",procname,field->name);
	    printstate();
	    return(0);
	}
#ifdef DEBUG
printf("   about to setfield to %d\n",val);
#endif
    field->assigned[state] = 1;
    field->outfield[state] = val;
}


printass(str)
    char *str;
{
    fprintf(symf,"output %s not assigned,",str);
    printf("output %s not assigned,",str);
    printstate();
}


printexcl()
{
    fprintf(symf,"output assignment conflict,");
    printf("output assignment conflict,");
    printstate();
}


find(name,index)	/* hash-search for name, return state number	*/
    char *name;
    short index;
{
    short i;

    if ((i = ilookup(name)) <0)
	{
	    fprintf(symf,"undefined label %s state %x\n",name,index-1);
	    printf("undefined label %s state %x\n",name,index-1);
	}
    return(i);
}


newfile(fname)
    char *fname;
{
    fprintf(symf,"%s\n",fname);
    filestate = 0;
    ++filecounter;
    strcpy(filename,fname);
}

_Label(lab)
    char *lab;
{
    install(lab,state);
    fprintf(symf,"%x\t%s\n",state,lab);
}
