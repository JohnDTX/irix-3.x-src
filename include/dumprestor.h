/*
 * $Source: /d2/3.7/src/include/RCS/dumprestor.h,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 16:11:20 $
 */

#define NTREC   	20
#define MLEN    	16
#define MSIZ    	4096

#define TS_TAPE 	1
#define TS_INODE	2
#define TS_BITS 	3
#define TS_ADDR 	4
#define TS_END  	5
#define TS_CLRI 	6
#define MAGIC   	(int)60011
#define CHECKSUM	(int)84446
struct	spcl
{
	int	c_type;
	time_t	c_date;
	time_t	c_ddate;
	int	c_volume;
	daddr_t	c_tapea;
	ino_t	c_inumber;
	short	c_idummy;	/* pad to force long boundary */
	int	c_nrec;		/* number of records per tape */
	int	c_magic;
	int	c_checksum;
	struct	dinode	c_dinode;
	int	c_count;
	char	c_addr[BSIZE];
} spcl;

struct	idates
{
	char	id_name[16];
	char	id_incno;
	time_t	id_ddate;
};
