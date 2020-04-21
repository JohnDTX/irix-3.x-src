
/* BYTE 0 defines */
#define NOCARTRIDGE	0x40
#define NOTONLINE	0x20
#define WRITEPROTECTED	0x10
#define ENDOFTAPE	0x08
#define DATAERROR	0x04
#define BOTNOTFOUND	0x02
#define FILEFOUND	0x01
/* BYTE 1 defines */
#define	ILLEGALCMD	0x40
#define	NODATAFOUND	0x20
#define	MAXRETRIES	0x10
#define	BOT		0x08
#define	RESVERED0	0x04
#define	RESVERED1	0x02
#define	RESETOCCURRED	0x01

/* Tape flags */
#define TAPE_DEAD	0x80
#define TAPE_INITED	0x01
#define TAPE_INREWIND	0x02
#define TAPE_INSPACE	0x04
#define FILE_MARK_FOUND	0x08
