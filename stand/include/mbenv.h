/*
* $Source: /d2/3.7/src/stand/include/RCS/mbenv.h,v $
* $Revision: 1.1 $
* $Date: 89/03/27 17:13:46 $
*/

extern char	*MBioVA;     /* virtual address of the multibus i/o space     */
extern char	*MBmemVA;    /* virtual address of the multibus accessable    */
			     /*  memory					      */
extern char	*MBmallocVA; /* virtual address of area given to mbmalloc()   */
extern long	MBmallocSZ;  /* size of the area given to mbmalloc()	      */
extern long	MBmemphys;   /* multibus memory space			      */
extern long	MBiophys;    /* multibus io space			      */
extern char	*MBmappedVA; /* virtual address of a multibus mapped area */
extern long	MBmapSZ;     /* maximum size of multibus mapped area	*/
extern char	*MBmappedphys;	/* multibus address of mapped area	*/
extern long	MBmappedstart; /* start of dynamic mapped area	*/

/*
** some nice macros for conversions
*/
/*	map multibus IO to virtual	*/
#define	mbiotov(x)	((long)(x)+(long)MBioVA)
/*	map virtual to multibus memory	*/
#define	mbvtop(x)  \
		( (long)(x) >= (long)MBmemVA ? \
			((long)(x)-(long)MBmemVA+(long)MBmemphys): \
			( (long)(x)&(0xfff) + (long)MBmappedphys ) )
/*	map multibus memory to virtual */
#define mbptov(x) \
		( (long)(x) < (long)MBmappedphys ? \
			((long)(x)+(long)MBmemVA-(long)MBmemphys): \
			( (long)MBmappedVA + (long)(x) - (long)MBmappedphys ) )

#define	CONF_DEAD	1
#define	CONF_FAULTED	2
#define	CONF_ALIVE	3

#define MBIO_VBASE	SEG_MBIO
