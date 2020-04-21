/*
 * $Source: /d2/3.7/src/usr.bin/edge/common/RCS/dbxshm.h,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:45:51 $
 */
#define SHMSIG "###"

#define	DBX_SHMKEY	101
#define	MIDDLE	0
#define TOP	1

typedef	struct	{
	int	lineno;
	int	value;
	char	sbuf[1000];
	char	filename[512];
} DBXSHM;

extern	DBXSHM	*shmptr;
extern	int	shm_id;

typedef struct {
	int	sp_ptynum;
	int	sp_masterfd;
	char	*sp_mastername;
	char	*sp_slavename;
	int	sp_slavefd;
} SYNCPTY;

extern	SYNCPTY	syncptyin;
extern	SYNCPTY	syncptyout;
extern	int	noread;
extern	int	cur_line;
extern	char	*cur_file;
