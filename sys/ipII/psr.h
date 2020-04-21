/*
 * $Source: /d2/3.7/src/sys/ipII/RCS/psr.h,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:30:58 $
 */
/*
** bit definitions for the 68020 status register.
*/
#define	SR_C	0x1		/* carry bit			*/
#define	SR_OVFL	0x2		/* overflow bit			*/
#define	SR_Z	0x4		/* zero bit			*/
#define	SR_NEG	0x8		/* negative bit			*/
#define	SR_EXTD	0x10		/* extend bit			*/
#define	SR_IPM	0x700		/* interrupt priority mask	*/
#define	SR_MI	0x1000		/* master/interrupt state	*/
#define	SR_SU	0x2000		/* supervisor/use state		*/
#define	SR_TMSK	0x3000		/* trace enable bits		*/

#define	SR_IPM0	0x0		/* interrupt priority mask 0	*/
#define	SR_IPM1	0x100		/* interrupt priority mask 1	*/
#define	SR_IPM2	0x200		/* interrupt priority mask 2	*/
#define	SR_IPM3	0x300		/* interrupt priority mask 3	*/
#define	SR_IPM4	0x400		/* interrupt priority mask 4	*/
#define	SR_IPM5	0x500		/* interrupt priority mask 5	*/
#define	SR_IPM6	0x600		/* interrupt priority mask 6	*/
#define	SR_IPM7	0x700		/* interrupt priority mask 7	*/

#define	SR_TANY		0x8000	/* trace any instruction	*/
#define	SR_TFLOW	0x4000	/* trace flow of control only	*/

/*
** for compatability
*/
#define	PS_C	SR_C		/* carry bit */
#define	PS_V	SR_OVFL		/* overflow bit */
#define	PS_Z	SR_Z		/* zero bit */
#define	PS_N	SR_NEQ		/* negative bit */
#define	PS_X	SR_EXTD		/* extend bit */
#define	PS_IPL	SR_IPM		/* interrupt priority level */
#define	PS_SUP	SR_SU		/* supervisor mode */
#define	PS_T	SR_TANY		/* trace enable bit */
