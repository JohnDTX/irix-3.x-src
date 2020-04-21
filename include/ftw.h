/*
 * $Source: /d2/3.7/src/include/RCS/ftw.h,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 16:11:30 $
 */

/*
 *	Codes for the third argument to the user-supplied function
 *	which is passed as the second argument to ftw
 */

#define	FTW_F	0	/* file */
#define	FTW_D	1	/* directory */
#define	FTW_DNR	2	/* directory without read permission */
#define	FTW_NS	3	/* unknown type, stat failed */
