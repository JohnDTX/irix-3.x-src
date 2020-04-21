/*
 * Definitions for "pipe" filesystem.
 *
 * $Source: /d2/3.7/src/sys/com/RCS/pipe_inode.h,v $
 * $Date: 89/03/27 17:26:48 $
 * $Revision: 1.1 $
 */
#include "../com/com_inode.h"
#include "../h/poll.h"

/*
 * Common inode information for named pipes under the efs.  The
 * implementation for pipes and fifos is shared by all the filesystems
 * that use the com filesystem.
 */
struct pipe_inode {
	struct	com_inode pi_com;	/* base class state */
	struct pipe_buffer {		/* read and write buffer state */
		short	pb_flags;	/* state flags */
		short	pb_ptr;		/* read index into buffer */
		short	pb_ref;		/* number of processes referencing */
		struct pollqueue pb_pq;	/* queue of polling processes */
	} pi_rb, pi_wb;
	struct	buf *pi_bp;		/* buffer holding data */
};

#define	pipe_fsptr(ip)	((struct pipe_inode *) (ip)->i_fsptr)

/* pipe buffer flags */
#define	PBWAIT	0x1	/* process(es) waiting on a buffer */

#define	PIPE_SIZE	(10 * 1024)
#define	PIPE_MAGIC	0xF1F0F1F0

int	pipe_create();
void	pipe_openi();
void	pipe_closei();
void	pipe_readi();
void	pipe_writei();
