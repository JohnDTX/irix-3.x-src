/* streams tty interface
 *	definitions for the streams 'line discipline'
 *
 * $Source: /d2/3.7/src/sys/streams/RCS/stty_ld.h,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:35:00 $
 */


#ifdef mips
#define u_char unchar
#endif

/* view an IOCTL message as a termio structure */
#define STERMIO(bp) ((struct termio*)(bp)->b_cont->b_rptr)


/* one of these is created for each stream
 */
struct stty_ld {
	struct termio st_termio;	/* modes and such		*/
#define st_iflag st_termio.c_iflag	/* input modes			*/
#define st_oflag st_termio.c_oflag	/* output modes			*/
#define st_cflag st_termio.c_cflag	/* 'control' modes		*/
#define st_lflag st_termio.c_lflag	/* line discipline modes	*/
#define	st_line  st_termio.c_line	/* line discipline		*/
#define	st_cc	 st_termio.c_cc		/* control chars		*/

	u_char	st_werase;		/* word erase			*/
	u_char	st_retype;		/* retype line			*/
	u_char	st_lit;			/* take next character literally*/
	u_char	st_flushc;		/* flush output			*/

	u_char	st_rrunning;		/* input busy			*/

	ushort	st_state;		/* current state		*/
	short	st_pgrp;		/* process group name		*/
	int	st_tid;			/* input timer ID		*/
	ushort	st_ocol;		/* current output 'column'	*/
	ushort	st_ncol;		/* future current column	*/
	ushort	st_xcol;		/* current 'interfering' column	*/
	ushort	st_bcol;		/* 'base' of input line		*/

	ushort st_llen;			/* bytes of typed-ahead text	*/

	queue_t *st_rq;			/* our read queue		*/
	queue_t *st_wq;			/* our write queue		*/
	mblk_t	*st_imsg, *st_ibp;	/* input line			*/
	mblk_t	*st_lmsg, *st_lbp;	/* typed-ahead canonical lines	*/
	mblk_t	*st_emsg, *st_ebp;	/* current echos		*/
};

/* states */
#define ST_LIT		0x0001		/* have seen literal character	*/
#define ST_ESC		0x0002		/* have seen backslash		*/
#define ST_TIMING	0x0004		/* timer is running		*/
#define ST_TIMED	0x0008		/* timer has already run	*/
#define ST_TABWAIT	0x0010		/* waiting to echo HT delete	*/
#define ST_BCOL_BAD	0x0020		/* base column # is wrong	*/
#define ST_BCOL_DELAY	0x0040		/* mark column # bad after this */
#define ST_ISTTY	0x0080		/* told stream head about tty	*/
#define ST_INRAW	0x0100		/* input is very raw		*/
#define ST_INPASS	0x0200		/* pass input upstream fast	*/


#define ST_MAX_LINE	256		/* maximum cooked/cannonical line */
