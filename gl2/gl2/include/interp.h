#ifndef INTERPDEF
#define INTERPDEF
/**************************************************************************
 *									  *
 * 		 Copyright (C) 1984, Silicon Graphics, Inc.		  *
 *									  *
 *  These coded instructions, statements, and computer programs  contain  *
 *  unpublished  proprietary  information of Silicon Graphics, Inc., and  *
 *  are protected by Federal copyright law.  They  may  not be disclosed  *
 *  to  third  parties  or copied or duplicated in any form, in whole or  *
 *  in part, without the prior written consent of Silicon Graphics, Inc.  *
 *									  *
 **************************************************************************/

/********************* Interpreter MACROS	*************************/

#define movb(x,y)	asm("movb x,y")
#define movw(x,y)	asm("movw x,y")
#define movl(x,y)	asm("movl x,y")
#define addl(x,y)	asm("addl x,y")
#define bra(x)		asm("bra x")
#define jra(x)		asm("jra x")
#define jmp(x)		asm("jmp x")
#define clrl(x)		asm("clrl x")
#define rts()		asm("rts")

#define INTERP_PC a4
#define INTERP_A0 a0
#define DECLARE_INTERP_REGS \
    register unsigned short *GE; \
    register long *PC;	/* be real carefull here: GE==a5, PC==a4 */ \
    register windowstate *WS	/* and WS==a3		*/


#define thread movl(a4@+,a0); jmp(a0@)

#define GL_INTERP_LABEL(rootname,charname,size) \
	asm (".text"); \
	asm (".long	charname"); \
	asm (".long	-1"); \
	asm (".long	size"); \
	asm (".globl rootname"); \
	asm ("rootname:")

#define INTERP_LABEL(name,size) \
	GL_INTERP_LABEL(_i_/**/name,_/**/name/**/_n,size)

#define INTERP_ROOT(rootname)		\
	INTERP_ROOT_2F(rootname/**/2);	INTERP_ROOT_3F(rootname);	\
	INTERP_ROOT_2I(rootname/**/2i);	INTERP_ROOT_3I(rootname/**/i);	\
	INTERP_ROOT_2S(rootname/**/2s);	INTERP_ROOT_3S(rootname/**/s)
	
#define INTERP_ROOT_0(name)		\
INTERP_LABEL(name,2);			\
	im_/**/name ();			\
	thread

#define INTERP_ROOT_1S(name)		\
INTERP_LABEL(name,3);			\
	im_/**/name (*(short *)PC++);	\
	thread

#define INTERP_ROOT_1I(name)		\
INTERP_LABEL(name,4);			\
	im_/**/name (*PC++);		\
	thread

#define INTERP_ROOT_1F(name)		\
INTERP_LABEL(name,4);			\
	im_/**/name (*(float *)PC++);		\
	thread

#define INTERP_ROOT_2S(name)		\
INTERP_LABEL(name,4);			\
	im_/**/name (*(short *)PC++, *(short *)PC++);	\
	thread

#define INTERP_ROOT_2I(name)		\
INTERP_LABEL(name,6);			\
	im_/**/name (*PC++, *PC++);	\
	thread

#define INTERP_ROOT_2F(name)		\
INTERP_LABEL(name,6);			\
	im_/**/name (*(float *)PC++, *(float *)PC++);	\
	thread

#define INTERP_ROOT_3S(name)		\
INTERP_LABEL(name,5);			\
	im_/**/name (*(short *)PC++,*(short *)PC++,*(short *)PC++);	\
	thread

#define INTERP_ROOT_3I(name)		\
INTERP_LABEL(name,8);			\
	im_/**/name (*PC++, *PC++, *PC++);	\
	thread

#define INTERP_ROOT_3F(name)		\
INTERP_LABEL(name,8);			\
	im_/**/name (*(float *)PC++,*(float *)PC++,*(float *)PC++);	\
	thread

#define INTERP_ROOT_4S(name)		\
INTERP_LABEL(name,6);			\
	im_/**/name (*(short *)PC++, *(short *)PC++,		\
			*(short *)PC++, *(short *)PC++);	\
	thread

#define INTERP_ROOT_4I(name)		\
INTERP_LABEL(name,10);			\
	im_/**/name (*PC++, *PC++, *PC++, *PC++);	\
	thread

#define INTERP_ROOT_4F(name)		\
INTERP_LABEL(name,10);			\
	im_/**/name (*(float *)PC++, *(float *)PC++,		\
			*(float *)PC++, *(float *)PC++);	\
	thread

#define INTERP_ROOT_MATRIX(name)	\
INTERP_LABEL(name,34);			\
	im_/**/name (PC);		\
	PC += 16;			\
	thread

#endif INTERPDEF
