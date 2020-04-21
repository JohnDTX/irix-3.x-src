#ifndef	__TYPES__

#define	__TYPES__

/* generic unsigned data types */
typedef	unsigned short	ushort;
typedef	unsigned int	uint;
typedef	unsigned long	ulong;

/* system data types */
typedef	struct { int r[1]; } *	physadr;
typedef	long		daddr_t;
typedef	long		swblk_t;
typedef	char *		caddr_t;
typedef	ushort		ino_t;
typedef short		cnt_t;
typedef	long		time_t;
typedef	int		label_t[13];
typedef	short		dev_t;
typedef	long		off_t;
typedef	long		paddr_t;
typedef	long		key_t;

/* these are for berkeley compatibility */
typedef	unsigned char	u_char;
typedef	unsigned short	u_short;
typedef	unsigned int	u_int;
typedef	unsigned long	u_long;
typedef	short		size_t;

#ifdef HEAP_CHECK
# undef HEAP_DUMP
# define HEAP_DUMP	1
#endif
#ifdef HEAP_DUMP
# define malloc(size)		_malloc(__FILE__, __LINE__, size)
# define calloc(number, size)	_calloc(__FILE__, __LINE__, number, size)
#endif

#endif	__TYPES__
