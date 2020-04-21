#define CALL(str)	printf("\tjbsr\t%s\n",str)

#define COMM(str,size) 	printf("\t.comm\t%s,%d\n",str,size)

#define COMMENT3(str1,str2,str3)\
			printf("|\n|\t%s %s %s\n|\n",str1,str2,str3)

#define COMMENT2(str1,str2) \
			COMMENT3(str1,str2,"")

#define COMMENT(str1)	COMMENT3(str1,"","")

#define EXTEND(ch)	printf("\text%c\td0\n",ch)

#define FIRST_ARG_OFFSET 8

#define GLOBL(str)	printf("\t.globl\t%s\n",str)

#define INCR_ADDRESS(con) printf("\t%s\t#%d,a0\n",\
			       ((con > 0) && (con < 9))?"addql":"addl",con)

#define INCR_VALUE(con)	printf("\t%s\t#%d,d0\n",\
			       ((con > 0) && (con < 9))?"addql":"addl",con)

#define JUMP_INDIRECT	printf("\tjra\ta0@\n")

#define LABEL(str)	printf("%s:\n",str)

#define LINK(locals)	printf("\tlink\ta6,#%d\n",locals)


#define LOAD_IND_STKVAL(off,ch) \
			printf("\tmovl\ta6@(%d),a0\t|get address from stack\n"\
					   ,off);\
			printf("\tmov%c\ta0@,d0\t|get indirect value\n",ch)
			
#define LOAD_STKADDR(offset) \
			printf("\tlea\ta6@(%d),a0\t|get address of value on stack\n",offset)

#define LOAD_STKVAL_A(offset) \
			printf("\tmovl\ta6@(%d),a0\n",offset)

#define LOAD_VALUE(offset,ch) \
			printf("\tmov%c\ta6@(%d),d0\n",ch,offset)
		
#define LOAD_ZERO	printf("\tclrl\td0\n")

#define PULL_ADDRESS	printf("\tmovl\tsp@+,a0\n")

#define PUSH_ADDRESS	printf("\tmovl\ta0,sp@-\n")

#define PUSH_CONSTANT(con)\
			printf("\tpea\t%d\t\t|push a constant on the stack\n",con)

#define PUSH_INDIRECT(off) \
			printf("\tmovl\ta0@(%d),sp@-\n",off)

#define PUSH_SHORT_CONSTANT(con)\
			printf("\tmovw\t#%d,sp@-\n",con)

#define PUSH_SHORT_VALUE \
			printf("\tmovw\td0,sp@-\n")

#define PUSH_STK_ADDR(off) \
			printf("\tpea\ta6@(%d)\t|push address of value on stack\n",off)

#define PUSH_STK_VALUE(off,ch) \
			printf("\tmov%c\ta6@(%d),sp@-\t|copy value to new parameter list\n",ch,off)

#define PUSH_VALUE	printf("\tmovl\td0,sp@-\n")


#define RESTOREFREGS	printf("\tmovl\t_$a4_save,a4\t|Fortran expects these registers\n");\
			printf("\tmovl\t_$a5_save,a5\t|to be static when executing\n")

#define RESTORECREGS(off) \
			printf("\tmoveml\ta6@(%d),#0x3004\t|restore a4/5 d2\n",off)

#define RTS		printf("\trts\n")

#define SAVE_VALUE(off)	printf("\tmovl\td0,a6@(%d)\t|save d0 across calls\n",off)

#define SAVECREGS(off)	printf("|\n|\tFortran must have a4/a5 constant anytime\n");\
			printf("|\tis is executing.  It also considers d2 to be volatile\n");\
			printf("|\tacross procedure calls.  Save these C registers\n|\n");\
			printf("\tmoveml\t#0x3004,a6@(%d)\n",off)

#define SAVEFREGS	printf("\tmovl\ta4,_$a4_save\n");\
			printf("\tmovl\ta5,_$a5_save\n")

#define STABC(name)	\
	printf("\t.stabs\t\"%s\",0x%x,0,%d,0\n",name,N_ISWRP,WRAPPING_C)

#define STABF(name)	\
	printf("\t.stabs\t\"%s\",0x%x,0,%d,0\n",name,N_ISWRP,WRAPPING_FORTRAN);

#define TEXTCSEG	printf("\t.text\n")

#define UNLINK		printf("\tunlk\ta6\n")

#define UPDATE_SP(off)	printf("\t%s\t#%d,sp\n",\
			       ((off > 0) && (off < 9))?"addql":"addl",off)

