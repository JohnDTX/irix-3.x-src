/*
 * 68010 Trap type values
 *
 * $Source: /d2/3.7/src/sys/pmII/RCS/trap.h,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:33:53 $
 */

#define	T_BUSERR	2	/* bus error fault */
#define	T_ADDRERR	3	/* address error fault */
#define	T_ILLINST	4	/* illegal instruction fault */
#define	T_DIVZERO	5	/* zero divide fault */
#define	T_CHK		6	/* CHK instruction */
#define	T_TRAPV		7	/* TRAPV instruction */
#define	T_PRIVVIO	8	/* privilege violation fault */
#define	T_TRCTRAP	9	/* trace trap */
#define	T_L1010		10	/* line 1010 emulation trap */
#define	T_L1111		11	/* line 1111 emulation trap */
#define	T_RESCHED	12	/* SOFTWARE trap for reschedule stuff */
#define	T_COPROC	13	/* coprocessor violation (68020) */
#define	T_FORMAT	14	/* format error (fault frames) */
#define	T_SPURINT	24	/* spurious interrupt trap */
/* 25-31 are interrupt levels 1-7 */
#define	T_SYSCALL	32	/* TRAP 0 (syscall trap) */
#define	T_TRAP1		33	/* TRAP 1 */
#define	T_TRAP2		34	/* TRAP 2 */
#define	T_TRAP3		35	/* TRAP 3 */
#define	T_TRAP4		36	/* TRAP 4 */
#define	T_TRAP5		37	/* TRAP 5 */
#define	T_TRAP6		38	/* TRAP 6 */
#define	T_TRAP7		39	/* TRAP 7 */
#define	T_TRAP8		40	/* TRAP 8 */
#define	T_TRAP9		41	/* TRAP 9 */
#define	T_TRAP10	42	/* TRAP 10 */
#define	T_TRAP11	43	/* TRAP 11 */
#define	T_TRAP12	44	/* TRAP 12 */
#define	T_TRAP13	45	/* TRAP 13 */
#define	T_TRAP14	46	/* TRAP 14 */
#define	T_TRAP15	47	/* TRAP 15 */
/* 48-63 are reserved */

#define	KRNL	64	/* bit or'd into trap code signifying kernel faulted */
