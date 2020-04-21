/*
 * $Source: /d2/3.7/src/include/RCS/scnhdr.h,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 16:11:51 $
 */

struct scnhdr {
	char		s_name[8];	/* section name */
	long		s_paddr;	/* physical address */
	long		s_vaddr;	/* virtual address */
	long		s_size;		/* section size */
	long		s_scnptr;	/* file ptr to raw data for section */
	long		s_relptr;	/* file ptr to relocation */
	long		s_lnnoptr;	/* file ptr to line numbers */
	unsigned short	s_nreloc;	/* number of relocation entries */
	unsigned short	s_nlnno;	/* number of line number entries */
	long		s_flags;	/* flags */
	};

#define	SCNHDR	struct scnhdr
#define	SCNHSZ	sizeof(SCNHDR)




/*
 * Define constants for names of "special" sections
 */

#define _TEXT ".text"
#define _DATA ".data"
#define _BSS  ".bss"
#define _TV   ".tv"




/*
 * The low 4 bits of s_flags is used as a section "type"
 */

#define STYP_REG	0x00		/* "regular" section: */
					/*	allocated, relocated, loaded */
#define STYP_DSECT	0x01		/* "dummy" section: */
					/*	not allocated, relocated,
						not loaded */
#define STYP_NOLOAD	0x02		/* "noload" section: */
					/*	allocated, relocated,
						 not loaded */
#define STYP_GROUP	0x04		/* "grouped" section: */
					/*	formed of input sections */
#define STYP_PAD	0x08		/* "padding" section: */
					/*	not allocated, not relocated,
						 loaded */
#define STYP_COPY	0x10		/* "copy" section: */
					/*	for decision function used
						by field update;  not
						allocated, not relocated,
						loaded;  reloc & lineno
						entries processed normally */

/*
 *  In a minimal file or an update file, a new function
 *  (as compared with a replaced function) is indicated by S_NEWFCN
 */

#define S_NEWFCN  0x10

/*
 * In 3b Update Files (output of ogen), sections which appear in SHARED
 * segments of the Pfile will have the S_SHRSEG flag set by ogen, to inform
 * dufr that updating 1 copy of the proc. will update all process invocations.
 */

#define S_SHRSEG	0x20
