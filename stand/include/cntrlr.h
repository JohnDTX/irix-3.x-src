/*
* $Source: /d2/3.7/src/stand/include/RCS/cntrlr.h,v $
* $Revision: 1.1 $
* $Date: 89/03/27 17:13:35 $
*/

#define	MAX(a,b)	(((a)>(b))?a:b)
#define MIN(a,b)	(((a)<(b))?a:b)

#define	INITED		0x1
#define	INUSE		0x2
#define	READING		0x4
#define	WRTING		0x8
