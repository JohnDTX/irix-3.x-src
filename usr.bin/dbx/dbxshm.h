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
