# ifndef _PM2MAP_

# define _PM2MAP_


/* pages per mem chunk (1/2 megabyte) */
# define MEM_CHUNK_PAGES	0x80
# define MEM_CHUNK_SIZE		ptoa(MEM_CHUNK_PAGES)
# define MAX_MEM_CHUNKS		31


/*
 * MAPPING SCHEME:
 *
	the upper N_STACKBSS_PAGES of the last half meg are
	given to the stack.
	the next lower N_MBMEM_PAGES are given to multibus-
	addressible memory.
	the highest of these is given to the disk controller.
	the next N_MBMA_PAGES lower are given to mbmalloc.

	Memory is mapped so that logical (virtual) space is 
	contiguous.

	Note that in the naming scheme used below,
	_FIRST_PAGES are inclusive but _LAST_PAGES are not.
 *
 */

/* pm2 multibus map offsets */
# define MBMAPOFFSET	0x100000
# define MBMBOXOFFSET	0x000000
# define MBMEMOFFSET	0x010000
# define MBNOMEMOFFSET	0x0F0000

/* physical layout of special prom half-meg */
# define CHUNK_STACKBSS_PAGE	(MEM_CHUNK_PAGES-1)
# define CHUNK_FIRST_STACK_PAGE	(CHUNK_STACKBSS_PAGE+1-N_STACKBSS_PAGES)
# define CHUNK_WUB_PAGE		(CHUNK_FIRST_STACK_PAGE-N_WUB_PAGES)
# define CHUNK_FIRST_MBMA_PAGE	(CHUNK_WUB_PAGE-N_MBMA_PAGES)
# define CHUNK_FIRST_MBMEM_PAGE	(CHUNK_FIRST_STACK_PAGE-N_MBMEM_PAGES)
# define CHUNK_FIRST_SPECIAL_PAGE	CHUNK_FIRST_MBMEM_PAGE

/* VIRTUAL LAYOUT */
# define VIRT_LAST_PAGE		(VIRT_FIRST_PAGE+N_VIRT_PAGES)
# define VIRT_FIRST_PAGE	0x00
# define N_VIRT_PAGES		0xF80

/* virtual address of MB io window */
# define VIRT_LAST_MBIO_PAGE	VIRT_LAST_PAGE
# define VIRT_FIRST_MBIO_PAGE	(VIRT_LAST_MBIO_PAGE-N_MBIO_PAGES)
# define N_MBIO_PAGES		0x10

/* virtual address of stack+bss */
# define VIRT_LAST_STACK_PAGE	VIRT_FIRST_MBIO_PAGE
# define VIRT_STACKBSS_PAGE	(VIRT_LAST_STACK_PAGE-1)
# define VIRT_FIRST_STACK_PAGE	(VIRT_LAST_STACK_PAGE-N_STACKBSS_PAGES)
# define N_STACKBSS_PAGES	3

/* virtual address of MB mem window */
# define VIRT_LAST_MBMEM_PAGE	VIRT_FIRST_STACK_PAGE
# define N_WUB_PAGES		1
# define VIRT_WUB_PAGE		(VIRT_LAST_MBMEM_PAGE-N_WUB_PAGES)
# define VIRT_FIRST_MBMEM_PAGE	(VIRT_LAST_MBMEM_PAGE-N_MBMEM_PAGES)

/* virtual address of MB malloc area */
# define VIRT_LAST_MBMA_PAGE	VIRT_WUB_PAGE
# define VIRT_FIRST_MBMA_PAGE	(VIRT_LAST_MBMA_PAGE-N_MBMA_PAGES)
# define VIRT_FIRST_SPECIAL_PAGE	VIRT_FIRST_MBMEM_PAGE
# define N_MBMA_PAGES		0x12
# define N_MBMEM_PAGES		0x70

/* virtual address of magic chunk */
# define VIRT_LAST_MAGIC_PAGE	(VIRT_LAST_PAGE-MEM_CHUNK_PAGES)
# define VIRT_FIRST_MAGIC_PAGE	(VIRT_LAST_MAGIC_PAGE-N_MAGIC_PAGES)
# define N_MAGIC_PAGES		MEM_CHUNK_PAGES


/* MBIO address of MB io window */
# define MBIO_LAST_PAGE		(MBIO_FIRST_PAGE+N_MBIO_PAGES)
# define MBIO_FIRST_PAGE	0x00

/* MBMEM address of MB mem window */
# define MBMEM_LAST_PAGE	0x80
# define MBMEM_FIRST_PAGE	(MBMEM_LAST_PAGE-N_MBMEM_PAGES)


/* virtual address of MB io window */
extern char *MBioVA;

/* virtual address of MB mem window */
extern char *MBMemVA;

/* virtual address of mbmalloc area */
extern char *MBMemArea;

/* size (remaining) of mbmalloc area */
extern long MBMemSize;

/* size of multibus i/o space */
extern long MBioSize;


/* map virtual->mbmem */
# define vtop(va)		((long)(va)-(long)MBMemVA)

/* map virtual->mbio */
# define vtombio(va)		((long)(va)-(long)MBioVA)

/* map mbmem->virt */
# define ptov(pa)		((long)(pa)+(long)MBMemVA)

/* map mbio->virt */
# define mbiotov(pa)		((long)(pa)+(long)MBioVA)


# endif  _PM2MAP_
