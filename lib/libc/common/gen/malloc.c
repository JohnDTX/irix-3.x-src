/*
 * Memory allocator, traditional interface.
 * Brendan Eich, based on Vernon Schryver's 9/5/86, but with a new free space
 * fragmentation heuristic.
 *
 * NB:	We assume that memory is always added (initialized to zeros) to the
 *	end of some kind of data segment, and never returned to the system.
 *
 * Compilation flags:
 *	ASSERT		compile random assertions
 *	DEBUG		if kswitch is patched, switch consoles before printf
 *	HEAP_DUMP	dumps heap to console if depleted
 *	HEAP_CHECK	perform thorough consistency check on every call
 *			also do HEAP_DUMP-ing
 *	METER		measure and collect raw statistics
 *
 * $Source: /d2/3.7/src/lib/libc/common/gen/RCS/malloc.c,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 16:14:30 $
 */
#ifdef KERNEL
# ifdef SVR3
#  include "sys/debug.h"
#  include "sys/types.h"
#  include "sys/param.h"
#  include "sys/sysmacros.h"
#  include "sys/tuneable.h"
#  include "sys/systm.h"
#  include "sys/map.h"
#  include "sys/cmn_err.h"
#  include "sys/immu.h"
#  include "sys/region.h"
#  include "sys/kopt.h"
# endif
# ifdef SVR0
#  include "../h/param.h"
#  include "../vm/vm.h"
#  include "../h/proc.h"
#  include "../h/cmap.h"
#  include "machine/pte.h"
# endif
# ifndef OS_ASSERT
#  undef ASSERT
# endif
# if defined OS_DEBUG && !defined DEBUG
#  define DEBUG	1
# endif
# ifndef OS_METER
#  undef METER
# endif
#else
# include <stdio.h>
# include <sys/param.h>
# ifdef SVR3
#  define PAGESIZE NBPC
# endif
# ifdef SVR0
#  define PAGESIZE NBPG
# endif
#endif

#ifndef	NULL
# define NULL	0
#endif

typedef	unsigned int	sizeof_t;
typedef	unsigned long	word_t;		/* can hold both int and ptr */

struct heap {
	struct fragment	*base;		/* first link in arena */
	struct fragment	*limit;		/* last (guard) link in arena */
	struct fragment	*rover;		/* roving search pointer */
	struct fragment	*lowfree;	/* free fragment low-watermark */
	unsigned short	freefrags;	/* number of free fragments */
	unsigned short	busyfrags;	/* number of busy fragments */
	sizeof_t	netminsize;	/* minimum net fragment size */
	sizeof_t	minsize;	/* minimum gross fragment size */
};

#define	HEAP(netminsize, overhead) \
    { NULL, NULL, NULL, NULL, 0, 0, (netminsize), (netminsize) + (overhead) }

/*
 * Heap operations - allocate, re-shape an allocation, deallocate, and
 * check structural integrity.
 *	struct heap	*hp;
 *	sizeof_t	nbytes;
 *	char		*p;
 */
char	*heap_alloc(/* hp, nbytes */);
char	*heap_realloc(/* hp, p, nbytes */);
char	*heap_free(/* hp, p */);
void	heap_check(/* hp, p */);

struct fragment {
	struct fragment	*forw;	/* pointer to next fragment */
	struct fragment	*back;	/* previous ptr plus flags */
};

/*
 * Private heap operations.  The result parameter rem is set to
 * point to free space after the busy nbytes added by heap_addmem().
 * NB: heap_addmem() does not adjust hp->rover, but it may adjust
 * hp->lowfree if hp->lowfree pointed to the guard at the old end
 * of memory.
 *	struct fragment	**rem;
 */
void		heap_init(/* hp, nbytes */);
struct fragment	*heap_addmem(/* hp, nbytes, rem */);

/*
 * Internal routines implemented variously, depending on whether
 * KERNEL is defined and if so on kernel type.  Calls to getendofmem(),
 * which may be a macro, return the current end of the data segment.
 * Moremem() always returns the current break via *endofmem, but may
 * fail, returning NULL directly.
 *	sizeof_t nbytes;
 *	char	**endofmem;
 *	char	*getendofmem()
 */
char	*moremem(/* nbytes, endofmem */);

/*
 * Flags stored in the low two bits of a pointer.  The alignment
 * restriction implied by these flags.
 */
#define	BUSY			0x1
#define	GUARD			0x2
#define	ALIGNMENT_MODULUS	((BUSY|GUARD)+1)

/*
 * Macros for treating heaplinks as words and vice versa, and for
 * testing, setting, and clearing flag bits in links.
 */
#define	word(p)		((word_t)(p))
#define	heaplink(w)	((struct fragment *)(w))
#define	test(p, f)	(word(p) & (f))
#define	flags(p)	test(p, BUSY|GUARD)
#define	add(p, f)	heaplink(word(p) | (f))
#define	sub(p, f)	heaplink(word(p) & ~(f))

/*
 * Macros for testing and modifying a fragment's state.
 */
#define	isfree(p)	(flags((p)->back) == 0)
#define	isbusy(p)	test((p)->back, BUSY)
#define	isguard(p)	test((p)->back, GUARD)
#define	SETBUSY(p)	((p)->back = add((p)->back, BUSY))
#define	SETGUARD(p)	((p)->back = add((p)->back, GUARD))
#define	CLRBUSY(p)	((p)->back = sub((p)->back, BUSY))
#define	CLRGUARD(p)	((p)->back = sub((p)->back, GUARD))

/*
 * Inquiry and connection operations for fragments.
 */
#define	nextfrag(p)	((p)->forw)
#define	prevfrag(p)	heaplink(word((p)->back) & ~(BUSY|GUARD))
#define	LINKTOFREE(p, q) \
	((p)->forw = (q), (q)->back = (p))
#define	LINKTOFLAGGED(p, q, f) \
	((p)->forw = (q), (q)->back = heaplink(word(p) | (f)))
#define	LINKTOBUSY(p, q) \
	LINKTOFLAGGED(p, q, BUSY)
#define	LINKTOGUARD(p, q) \
	LINKTOFLAGGED(p, q, GUARD)
#define	LINK(p, q) \
	LINKTOFLAGGED(p, q, flags((q)->back))

/*
 * Convenient address arithmetic macros.
 */
#define	address(p)		((char *) (p))
#define	advance(p, nbytes)	heaplink(address(p) + (nbytes))
#define	distance(p, q)		(address(q) - address(p))

/*
 * Rounding macros - n is the variable being rounded up, m is the
 * power-of-2 rounding modulus, and o is an offset to add.
 */
#define	ROUNDUPWITHOFFSET(n, m, o) \
	((n) += ((m) - 1 + (o)), (n) &= ~((m) - 1))
#define	ROUNDUP(n, m) \
	ROUNDUPWITHOFFSET(n, m, 0)
#define	ALIGN(n) \
	ROUNDUP(n, ALIGNMENT_MODULUS)

/*
 * Adjust a request size to be properly minimal and aligned.
 */
#define	ADJUST(n, hp) \
    (((n) < hp->netminsize) ? (n) = (hp)->netminsize : ALIGN(n))

#ifdef METER
/*
 * Heap metering structure.
 */
struct heap_meter {
	unsigned long	mallocs;	/* calls to malloc */
	unsigned long	frees;		/* calls to free */
	unsigned long	reallocs;	/* calls to realloc */
	unsigned long	callocs;	/* calls to calloc */
	unsigned long	busybytes;	/* gross bytes in use */
	unsigned long	lowstarts;	/* searches which began from lwm */
	unsigned long	searches;	/* scans for an allocation request */
	unsigned long	steps;		/* links traversed while scanning */
	unsigned long	wraps;		/* wraps around end of arena */
	unsigned long	lwmsteps;	/* upward low-watermark adjustments */
	unsigned long	lwmdrops;	/* downward low-watermark adjustment */
	unsigned long	forwcoals;	/* forward free blocks coalesced */
	unsigned long	backcoals;	/* backward free blocks coalesced */
	unsigned long	realgrows;	/* reallocs which grew in place */
	unsigned long	realhards;	/* " which required alloc-copy-free */
	unsigned long	moremems;	/* memory expansion calls */
};
#endif

/*
 * Traditional heap interface.
 */
#ifdef KERNEL

static struct heap kern_heap = HEAP(8, sizeof(struct fragment));

#ifdef METER
static struct heap_meter kheap_meter;

#undef METER
#define	METER(x)	(kheap_meter.x)
#else
#define METER(x)
#endif

#ifdef SVR3
sema_t	heapsem;		/* single thread heap */
sema_t	heapsync;		/* to sleep on to wait for more memory */

/* lock kernel heap, blocking other requests */
#define	LOCK_KHEAP()	psema(&heapsem, PRIBIO)

/* release kernel heap for other users */
#define	UNLOCK_KHEAP()	vsema(&heapsem)

/* release kernel heap and sleep for more memory */
#define	UNLOCKSLEEP_KHEAP()	vpsema(&heapsem, &heapsync, PRIBIO)

/* wake up any sleepers if memory becomes available */
#define	WAKEUP_KHEAP()	cvsema(&heapsync)
#endif

#ifdef SVR0
/* locking flags */
static char	kheap_busy, kheap_wanted;

/* lock kernel heap, blocking other requests */
#define	LOCK_KHEAP() { \
	register int s; \
	s = spl7(); \
	while (kheap_busy) { \
		kheap_wanted = 1; \
		sleep((caddr_t)&kheap_busy, PRIBIO); \
	} \
	kheap_busy = 1; \
	splx(s); \
}

/* release kernel heap for other users */
#define	UNLOCK_KHEAP() { \
	if (kheap_wanted) { \
		kheap_wanted = 0; \
		wakeup((caddr_t)&kheap_busy); \
	} \
	kheap_busy = 0; \
}

static char	kheap_depleted;
#endif

/*
 * Given locked kernel heap, try to allocate.  On failure sleep till
 * something is freed (see heap_free below).
 */
char *
kheap_alloc(hp, nbytes)
	register struct heap *hp;
	sizeof_t nbytes;
{
	register char *p;

	while ((p = heap_alloc(hp, nbytes)) == NULL) {
#ifdef SVR3
		UNLOCKSLEEP_KHEAP();
		LOCK_KHEAP();
#endif
#ifdef SVR0
		UNLOCK_KHEAP();
		kheap_depleted = 1;
		sleep((caddr_t) hp, PRIBIO);
		LOCK_KHEAP();
#endif
	}
	return p;
}

/*
 * XXX would like both SVR3 and SVR0 to use
 *	p = kmem_new(type)
 *	p = kmem_newvec(type, len)
 *	p = kmem_realloc(p, newsize)
 *	kmem_delete(p)
 * and smuggle type into heap for HEAP_DUMP, rather than file/line.
 */
#ifdef SVR3
# ifdef HEAP_DUMP
#  define malloc(nbytes) \
	  _kern_malloc(file, line, nbytes) char *file; int line;
#  define calloc(number, size) \
	  _kern_calloc(file, line, number, size) char *file; int line;
# else
#  define malloc(nbytes)	kern_malloc(nbytes)
#  define calloc(number, size)	kern_calloc(number, size)
# endif
# define realloc(p, nbytes)	kern_realloc(p, nbytes)
# define free(p)		kern_free(p)
#endif

#if defined SVR0 && defined HEAP_DUMP
# undef malloc
# define malloc(nbytes) \
	 _malloc(file, line, nbytes) char *file; int line;
# undef calloc
# define calloc(number, size) \
	 _calloc(file, line, number, size) char *file; int line;
short	heap_dump = 0;	/* XXX patchable */
#endif

char *
malloc(nbytes)
	sizeof_t nbytes;
{
	register char *p;

	METER(mallocs++);
#ifdef HEAP_DUMP
	nbytes += sizeof(char *);
#endif
	LOCK_KHEAP();
	p = kheap_alloc(&kern_heap, nbytes);
	UNLOCK_KHEAP();
#ifdef HEAP_DUMP
	if (p != NULL) {
		int i = (line >= 1000) ? 4 : (line >= 100) ? 3
			: (line >= 10) ? 2 : 1;
#ifdef SVR3
		register int len = strlen(file);

		/*
		 * SVR3 source files are compiled in local directories.
		 * Overwrite at most 4 chars at the end of file's storage with
		 * line's digits.
		 */
		*(char **)p = file;
		p += sizeof(char *);
		if (i < len) {
			file += len;
			while (--i >= 0) {
				*--file = '0' + line % 10;
				line /= 10;
			}
		}
#endif
#ifdef SVR0
		/*
		 * SVR0 source files are compiled relative to a peer config
		 * directory, so their names begin "../*".  Overwrite at most
		 * 4 chars at the front of file's storage with line's digits.
		 */
		*(char **)p = file;
		p += sizeof(char *);
		file += i;
		while (--i >= 0) {
			*--file = '0' + line % 10;
			line /= 10;
		}
		if (heap_dump) {
			print_heap_contents(&kern_heap);
			heap_dump = 0;
		}
#endif
	}
#endif
	return p;
}

char *
realloc(p, nbytes)
	char *p;
	sizeof_t nbytes;
{
	METER(reallocs++);
#ifdef HEAP_DUMP
	nbytes += sizeof(char *);
	p -= sizeof(char *);
#endif
	LOCK_KHEAP();
	p = heap_realloc(&kern_heap, p, nbytes);
	UNLOCK_KHEAP();
#ifdef HEAP_DUMP
	if (p != NULL) {
		p += sizeof(char *);
	}
#endif
	return p;
}

void
free(p)
	char *p;
{
	METER(frees++);
#ifdef HEAP_DUMP
	p -= sizeof(char *);
#endif
	LOCK_KHEAP();
	heap_free(&kern_heap, p);
	UNLOCK_KHEAP();
}

char *
calloc(number, size)
	int number;
	register sizeof_t size;
{
	register char *p;

	METER(callocs++);
	size *= number;
#ifdef HEAP_DUMP
	p = _malloc(file, line, size);
#else
	p = malloc(size);
#endif
	if (p != NULL) {
		bzero(p, size);
	}
	return p;
}

#ifdef SVR3
/* we must undefine this so as to call the "map allocate" malloc() */
#undef malloc
#undef realloc
#undef free
#undef calloc

/* this is defined in master.d/kernel */
extern long	kheap_maxbytes;

struct kheap_vm {
	char		*base;		/* kernel heap virtual base */
	char		*limit;		/* end of virtual region */
	char		*endofmem;	/* end of physical pages in region */
	unsigned long	pgbase;		/* first kernel heap page number */
};

static struct kheap_vm kheap_vm;
int kheap_initialized = 0;	/* used by semas to know when can alloc */

/*
 * Initialize the dynamic memory allocation system.
 */
void
init_malloc()
{
	if (kheap_initialized) {
		return;		/* XXX can't happen? */
	}

	/*
	 * Allocate page table slots from sptmap for later use.
	 */
	kheap_vm.pgbase = kvalloc(btoc(kheap_maxbytes), VM_NOSLEEP);
	if (kheap_vm.pgbase == 0) {
		cmn_err(CE_PANIC, "not enough vm for malloc");
	}
	kheap_vm.base = kheap_vm.endofmem = address(kheap_vm.pgbase);
	kheap_vm.limit = kheap_vm.base + kheap_maxbytes;
	if (showconfig) {
		printf("malloc virtual region: 0x%x - 0x%x\n",
		    kheap_vm.base, kheap_vm.limit - 1);
	}
	initnmutex(&heapsem, 1, "heapsem");
	initnsync(&heapsync, 0, "heapsync");
	kheap_initialized = 1;
}

#define	getendofmem()	(kheap_vm.endofmem)

static char *
moremem(nbytes, endofmem)
	register sizeof_t nbytes;
	char **endofmem;
{
	extern sema_t mem_lock;
	register char *p, *q;
	register int npages;
	register struct kheap_vm *vm = &kheap_vm;

	METER(moremems++);
	p = q = vm->endofmem;
	npages = btoc(nbytes);
	while (--npages >= 0) {

		/* compute address of new region to allocate */
		if (q >= vm->limit) {
			cmn_err(CE_WARN, "out of dynamic memory, nbytes=%d\n",
			    nbytes);
			p = NULL;	/* set for NULL return */
			break;
		}

		/* assign a physical page */
		availmemlock();
		--availsmem, --availrmem;
		if (availsmem < tune.t_minasmem
		    || availrmem < tune.t_minarmem) {
			availsmem++, availrmem++;
			availmemunlock();
			cmn_err(CE_WARN, "out of physical memory, nbytes=%d\n",
			    nbytes);
			p = NULL;
			break;
		}
		availmemunlock();

		psema(&mem_lock, PZERO);	/* lock memall */
		if (kpalloc(q, 1, 0)) {
			availmemlock();
			availrmem++; availsmem++;
			availmemunlock();
			vsema(&mem_lock);
			cmn_err(CE_WARN, "out of physical memory, nbytes=%d\n",
			    nbytes);
			p = NULL;
			break;
		}
		vsema(&mem_lock);

		q += ctob(1);
	}
	*endofmem = vm->endofmem = q;
#ifdef HEAP_DUMP
	if (p == NULL) {
		print_heap_contents(&kern_heap);
	}
#endif
	return p;
}
#endif	/* SVR3 */

#ifdef SVR0
/*
 * Dynamic memory is addressed virtually through a private page table.
 * Up to a half-meg of memory may be mapped.
 */
#define	NPTES	128			/* max of 128 pages of heap space */

struct kheap_vm {
	char		*base;		/* kernel heap virtual base */
	char		*limit;		/* end of virtual region */
	char		*endofmem;	/* end of physically mapped region */
	unsigned short	nextpte;	/* next pt entry to allocate */
	struct pte	pt[NPTES];	/* kernel heap page table */
};

static struct kheap_vm kheap_vm;

/* kheap_pte is exported to parity checking code in machdep.c */
struct pte *kheap_pte = &kheap_vm.pt[0];

/*
 * Initialize the kernel's dynamic memory allocation system.
 */
init_malloc(vstart, vend)
	char *vstart, *vend;
{
	kheap_vm.base = kheap_vm.endofmem = vstart;
	kheap_vm.limit = vend;
#ifdef HEAP_CHECK
	printf("kernel heap virtual region from %x to %x\n",
	    vstart, vend);
	printf("kernel heap meter at %x\n", &kheap_meter);
#endif
}

#define	getendofmem()	(kheap_vm.endofmem)

static char *
moremem(nbytes, endofmem)
	register sizeof_t nbytes;
	char **endofmem;
{
	register char *p, *q;
	register int reqsize, npages;
	register struct kheap_vm *vm = &kheap_vm;

	METER(moremems++);
	reqsize = nbytes;
	ROUNDUP(nbytes, NBPG);
	p = q = vm->endofmem;

	npages = nbytes >> PGSHIFT;
	while (--npages >= 0) {
		register struct pte *pte;

		/*
		 * Compute address of new region to allocate.  Make sure
		 * we can allocate there, and if we can, get memory from
		 * the vm system and make it accessible by the kernel.
		 */
		if (vm->nextpte >= NPTES || q >= vm->limit) {
			printf("kernel: out of dynamic memory, nbytes=%d\n",
			    reqsize);
#ifdef HEAP_DUMP
			print_heap_contents(&kern_heap);
#endif
			debug("kernel heap depleted");	
			p = NULL;	/* set for NULL return */
			break;
		}
		pte = &vm->pt[vm->nextpte++];
		(void) vmemall(pte, 1, (struct proc *)NULL, CSYS);
		ptaccess(pte, (struct pte *) q, 1);
		q += NBPG;
	}
	*endofmem = vm->endofmem = q;
	return p;
}
#endif	/* SVR0 */

#else	/* user-level */

static struct heap malloc_heap = HEAP(8, sizeof(struct fragment));

#ifdef METER
static struct heap_meter malloc_meter;

#undef METER
#define	METER(x)	(malloc_meter.x)
#else
#define METER(x)
#endif

char *
malloc(nbytes)
	sizeof_t nbytes;
{
	return heap_alloc(&malloc_heap, nbytes);
}

char *
realloc(p, nbytes)
	char *p;
	sizeof_t nbytes;
{
	return heap_realloc(&malloc_heap, p, nbytes);
}

void
free(p)
	char *p;
{
	heap_free(&malloc_heap, p);
}

char *
calloc(number, size)
	int number;
	register sizeof_t size;
{
	register char *p;

	size *= number;
	p = malloc(size);
	if (p != NULL) {
		bzero(p, size);
	}
	return p;
}

void
cfree(p, number, size)
	char *p;
	int number;
	sizeof_t size;
{
	heap_free(&malloc_heap, p);
}

void
malloc_check(p)
	char *p;
{
#ifdef HEAP_CHECK
	heap_check(&malloc_heap, p);
#endif
}

/*
 * Interface to data segment adjustment system call.
 */
char *sbrk();

#define	getendofmem()	sbrk(0)

static char *
moremem(nbytes, endofmem)
	register sizeof_t nbytes;
	char **endofmem;
{
	register char *newmem;
	register int pageresid;
	extern char *sbrk();

	METER(moremems++);
	/*
	 * Get current break.  Adjust count upward to align to next
	 * page boundary.  Make sure that we allocate at least nbytes
	 * worth of memory.
	 */
	newmem = sbrk(0);
	pageresid = PAGESIZE - ((long) newmem & (PAGESIZE - 1));
	if (nbytes < pageresid) {
		nbytes = pageresid;
	} else {
		nbytes = pageresid +
		    ((nbytes + PAGESIZE - 1) / PAGESIZE) * PAGESIZE;
	}

	newmem = sbrk(nbytes);
	*endofmem = sbrk(0);
	if (newmem == (char *) -1)
		return NULL;
	return newmem;
}

#endif	/* kernel/user */

/*
 * Cause a controlled crash.
 */
#ifdef KERNEL
# define ABORT()	panic("kernel heap")
#else
# define ABORT()	abort()
#endif

/*
 * Initialize debugging/assertion-checking output.
 */
#ifdef KERNEL
# if defined DEBUG && defined SVR0
#  define setprint()	forceconsole()
# else
#  define setprint()
# endif
#else
# define setprint()	setbuf(stdout, (char *) NULL)
#endif

/*
 * When compiled with -DHEAP_CHECK, check the heap's consistency at
 * strategic times.
 */
#ifdef HEAP_CHECK
# define check(h,p)	heap_check(h,p)

/*
 * Check heap consistency thoroughly.
 */
void
heap_check(h, p)
	register struct heap *h;
	register char *p;
{
	register struct fragment *target, *q, *r;
	char found_rover = 0, found_lowfree = 0, found_target = 0;

	target = (p == NULL) ? h->rover : heaplink(p) - 1;
	q = NULL;
	r = h->base;
	setprint();

	do {
		register int f;

		if (r < h->base || h->limit < r) {
			printf("heap check: bad link %x after %x\n", r, q);
			goto abort;
		}
		q = r;
		if (q == h->rover)
			found_rover++;
		if (q == h->lowfree)
			found_lowfree++;
		if (q == target)
			found_target++;

		r = nextfrag(q);
		f = flags(q->back);
		if (f == 0 || f == BUSY) {
			if (r < q) {
				printf(
				"heap check: missing guard at inversion %x\n",
				    q);
				goto abort;
			}
			if (distance(q, r) < h->minsize) {
				printf("heap check: fragment %x too small\n",
				    q);
				goto abort;
			}
		} else if (f != GUARD) {
			printf("heap check: bad flag %x at link %x\n", q, f);
			goto abort;
		}

		if (q != prevfrag(r)) {
			printf("heap check: bad back link from %x to %x\n",
			    r, q);
			goto abort;
		}
	} while (r > q);

	if (q != h->limit || r != h->base || !found_rover || !found_lowfree
	    || !found_target) {
		printf("heap check: bad chain from %x to %x", r, q);
		if (found_rover == 0)
			printf(", missing rover %x", h->rover);
		if (found_lowfree == 0)
			printf(", missing lowfree %x", h->lowfree);
		if (p != NULL && found_target == 0)
			printf(", missing fragment %x", target);
		printf("\n");
		goto abort;
	}
	return;
abort:
	ABORT();
}
#else	/* HEAP_CHECK */
# define check(h,p)
#endif	/* HEAP_CHECK */

#ifdef HEAP_DUMP
/*
 * Both kernel implementations support heap object tagging and dumping
 * when HEAP_DUMP is defined.
 */
#define	HASHSIZE	101

static struct hashtab {
	char		*key;
	unsigned int	count;
} hashtab[HASHSIZE];

static
print_heap_contents(h)
	register struct heap *h;
{
	register struct fragment *f;
	register struct hashtab *t;

	/* initialize hash table */
	for (t = &hashtab[0]; t < &hashtab[HASHSIZE]; t++) {
		t->key = NULL;
		t->count = 0;
	}

	/* build hash table */
	for (f = h->base; f < h->limit; f = nextfrag(f)) {
		if (isbusy(f)) {
			register char *k;
			register struct hashtab *t0;

			k = *(char **)(f + 1);
			t = t0 = &hashtab[(((int) k) >> 2) % HASHSIZE];
			for (;;) {
				if (t->key == NULL) {
					t->key = k;	/* first instance */
					t->count = 1;
					break;
				}
				if (t->key == k) {
					t->count++;	/* another instance */
					break;
				}
				if (--t < t0) {		/* probe linearly */
					if (t < &hashtab[0]) {
						t = &hashtab[HASHSIZE - 1];
					}
				} else if (t == t0) {
					break;
				}
			}
		}
	}

	/* dump hash table */
	for (t = &hashtab[0]; t < &hashtab[HASHSIZE]; t++) {
		if (t->key)
			printf("%s %d\n", t->key, t->count);
	}
}
#endif	/* HEAP_DUMP */

/*
 * If compiled for testing, assert certain tautologies.
 */
#ifdef ASSERT
# define assert(e,h)	if (e); else heap_botch(h,"e",__FILE__,__LINE__)

void
heap_botch(h, e, f, l)
	register struct heap *h;
	char *e, *f;
	int l;
{
	setprint();
	printf("heap botch: %s, file \"%s\", line %d\n", e, f, l);
	printf(
	    "base=%x limit=%x rover=%x lowfree=%x freefrags=%d busyfrags=%d\n",
	    h->base, h->limit, h->rover, h->lowfree, h->freefrags,
	    h->busyfrags);
	ABORT();
}
#else
# define assert(e,h)
#endif

/*
 * Given register variables p and q, adjust hp->lowfree so that it
 * points to a free fragment.  Leave p pointing at this fragment.
 */
#define	HEAP_SETLOWFREE(hp, p, q) { \
	(p) = (hp)->lowfree; \
	while (!isfree(p)) { \
		METER(lwmsteps++); \
		(q) = nextfrag(p); \
		if ((q) < (p)) { \
			break; \
		} \
		(p) = (q); \
	} \
	(hp)->lowfree = (p); \
}

/*
 * Fragmentation heuristic parameter: if the number of free fragments
 * times this factor is greater than or equal to the number of busy
 * fragments, then compact the heap by starting searches at lowfree.
 */
#if defined KERNEL && defined SVR3
extern unsigned short	kheap_fragfact;	/* in master.d/kernel */
#define	heap_fragfact	kheap_fragfact
#else
static unsigned short	heap_fragfact = 4;
#endif

/*
 * Allocate nbytes of memory from hp.
 */
char *
heap_alloc(hp, nbytes)
	register struct heap *hp;
	register sizeof_t nbytes;
{
	register struct fragment *this, *next;
	register struct fragment *start, *end;
	auto struct fragment *rem;

	ADJUST(nbytes, hp);
	if (hp->base == NULL) {
		heap_init(hp, nbytes);
	}
	check(hp, NULL);

	if (hp->freefrags * heap_fragfact >= hp->busyfrags) {
		METER(lowstarts++);
		HEAP_SETLOWFREE(hp, start, next);
	} else {
		start = hp->rover;
	}

	this = start;
	end = hp->limit;
	METER(searches++);
	for (;;) {
		next = nextfrag(this);

		/*
		 * Check whether this is free.  If it is both free and
		 * big enough for the request, stop searching.
		 */
		if (isfree(this)) {
			rem = advance(this, nbytes + sizeof *this);
			assert(this <= rem, hp);
			if (rem <= next) {
				register sizeof_t remsize;

				/*
				 * ``split'' operation, with rover/lowfree
				 * adjustment after loop.
				 */
				remsize = distance(rem, next);
				if (remsize >= hp->minsize) {
					LINKTOFREE(this, rem);
					LINK(rem, next);
				} else {
					--hp->freefrags;
					rem = next;
				}
				assert(this < rem && rem <= next, hp);
				break;
			}
		}

		/*
		 * Either this is not free or is both free and too
		 * small for the request.  Keep searching if there are
		 * blocks not yet visited.
		 */
		if (this < end) {
			assert(this < next, hp);
			METER(steps++);
			this = next;
			continue;
		}

		/*
		 * If we've searched from start through end.  If start is
		 * less than end, we still have to search from lowfree up
		 * to start.  The low watermark may be too low, so move it
		 * up if there were allocations after it was last set.
		 */
		if (start < end) {
			METER(wraps++);
			HEAP_SETLOWFREE(hp, this, next);
			end = start;
			continue;
		}

		/*
		 * We've searched everywhere and cannot find enough memory.
		 * Get some more and link it into the arena.
		 */
		this = heap_addmem(hp, nbytes, &rem);
		if (this == NULL) {
			return NULL;
		}
		assert(this < rem && rem <= hp->limit, hp);
		break;
	}

	METER(busybytes += nbytes + sizeof *this);
	SETBUSY(this);
	hp->busyfrags++;
	if (isfree(rem)) {
		if (this == hp->lowfree) {
			hp->lowfree = rem;
		}
		hp->rover = rem;
	}
	return address(this + 1);
}

/*
 * Reshape allocation p to be nbytes long and to contain the busy bytes
 * of data starting at p.
 */
char *
heap_realloc(hp, p, newsize)
	struct heap *hp;
	register char *p;
	register sizeof_t newsize;
{
	register struct fragment *this, *next, *rem;
	register sizeof_t oldsize, remsize, growby;

	this = heaplink(p) - 1;
	assert(hp->base <= this && this < hp->limit, hp);
#ifndef KERNEL
	/*
	 * Hack for backward-compatible realloc to allow compaction via
	 * free(p); free(lowp); realloc(p, newsize).
	 */
	if (isfree(this)) {
		register struct fragment *prev;

		/*
		 * We must find the free fragment which contains this (it may
		 * be this, but heap_free coalesces, so we must search).
		 */
		for (prev = hp->base; next = nextfrag(prev), next <= this;
		     prev = next) {
			continue;
		}
		/*
		 * Now prev describes the enclosing free fragment.  If prev
		 * and this are the same, we're done: just mark this busy.
		 * Otherwise make sure prev is far enough away from this, and
		 * then split the enclosing free piece into a piece at prev
		 * and a busy piece at this.
		 */
		assert(distance(prev, next) >= hp->minsize, hp);
		if (prev == this) {
			assert(!isfree(next), hp);
			SETBUSY(this);
		} else {
			assert(distance(this, next) >= hp->minsize, hp);
			while (distance(prev, this) < hp->minsize) {
				prev = prevfrag(prev);
				assert(prev < this, hp);
			}
			LINKTOBUSY(prev, this);
			LINK(this, next);
		}
	} else {
		assert(isbusy(this), hp);
		next = nextfrag(this);
	}
	check(hp, p);	/* must come after above hack, of course */
#else
	check(hp, p);
	assert(isbusy(this), hp);
	next = nextfrag(this);
#endif

	oldsize = distance(p, next);
	ADJUST(newsize, hp);

	/*
	 * If not changing size, just return p.
	 */
	if (newsize == oldsize) {
		return p;
	}

	/*
	 * If shrinking, consider freeing the remainder and return p.
	 * We call free here in order to coalesce forward and to update
	 * hp's roving pointers.
	 */
	if (newsize < oldsize) {
		rem = advance(p, newsize);
		if (distance(rem, next) >= hp->minsize) {
			hp->busyfrags++;
			LINKTOBUSY(this, rem);
			LINK(rem, next);
			heap_free(hp, address(rem + 1));
		}
		return p;
	}

	/*
	 * If growing, check whether the next fragment is free and big
	 * enough for the growth, which we call the remainder.  Take as
	 * much of the remainder as is needed, consider freeing the
	 * unneeded part, update hp's roving pointers, and return p.
	 */
	rem = next;
	next = nextfrag(next);
	remsize = distance(rem, next);
	growby = newsize - oldsize;
	if (isfree(rem) && growby <= remsize) {
		register struct fragment *newrem;

		assert(isbusy(next) || isguard(next), hp);
		METER(busybytes += growby);
		METER(realgrows++);
		remsize -= growby;	/* the new remaining size */
		/*
		 * ``split'' operation, with ad-hoc in-line free code
		 * modified to adjust rover and lowfree.
		 */
		if (remsize >= hp->minsize) {
			newrem = advance(rem, growby);
			LINKTOFREE(this, newrem);
			LINK(newrem, next);
		} else {
			--hp->freefrags;
			LINK(this, next);
			newrem = next;
		}
		if (rem == hp->rover) {
			hp->rover = newrem;
		}
		if (rem == hp->lowfree) {
			hp->lowfree = newrem;
		}
		return p;
	}

	/*
	 * Otherwise reallocate the hard way.
	 */
	{
		register char *newp;
	
		METER(realhards++);
#ifdef KERNEL
		newp = kheap_alloc(hp, newsize);
#else
		newp = heap_alloc(hp, newsize);
#endif
		if (newp != NULL) {
			bcopy(p, newp, oldsize);
		}
		heap_free(hp, p);
		return newp;
	}
}

/*
 * Return busy space at p to hp, coalescing if possible.  Update hp's
 * low-watermark and search pointer.
 */
char *
heap_free(hp, p)
	register struct heap *hp;
	char *p;
{
	register struct fragment *this, *prev, *next;

	check(hp, p);
	this = heaplink(p) - 1;
	assert(hp->base <= this && this < hp->limit, hp);
	assert(isbusy(this), hp);

	prev = prevfrag(this);
	next = nextfrag(this);
	assert(distance(this, next) >= hp->minsize, hp);
	METER(busybytes -= distance(this, next));

#ifndef KERNEL
	/* MUST clear busy because of realloc-free hack */
	CLRBUSY(this);
#endif
	if (isfree(prev)) {
		METER(backcoals++);
		--hp->freefrags;
		if (isfree(next)) {
			METER(forwcoals++);
			--hp->freefrags;
			next = nextfrag(next);
		}
		LINK(prev, next);
		this = prev;
	} else {
		if (isfree(next)) {
			METER(forwcoals++);
			--hp->freefrags;
			next = nextfrag(next);
			LINK(this, next);
		}
#ifdef KERNEL
		CLRBUSY(this);	/* see above */
#endif
	}

	if (this < hp->lowfree) {
		METER(lwmdrops++);
		hp->lowfree = this;
	}
	hp->rover = this;
	hp->freefrags++;
	--hp->busyfrags;
#ifdef KERNEL
#ifdef SVR3
	WAKEUP_KHEAP();
#endif
#ifdef SVR0
	if (kheap_depleted) {
		kheap_depleted = 0;
		wakeup((caddr_t) hp);
	}
#endif
#endif
}

static void
heap_init(hp, nbytes)
	register struct heap *hp;
	register sizeof_t nbytes;
{
	register struct fragment *base, *limit;
	auto char *endofmem;

	nbytes += 2 * sizeof *base;
	base = heaplink(moremem(nbytes, &endofmem));
	assert(base, hp);
	assert(address(base) + nbytes <= endofmem, hp);
	limit = heaplink(endofmem) - 1;
	LINKTOGUARD(base, limit);
	LINKTOFREE(limit, base);
	hp->base = hp->rover = hp->lowfree = base;
	hp->limit = limit;
	hp->freefrags = 1;
	hp->busyfrags = 0;
	if (hp->minsize % ALIGNMENT_MODULUS != 0) {
		ALIGN(hp->minsize);
	}
}

static struct fragment *
heap_addmem(hp, nbytes, rem)
	register struct heap *hp;
	register sizeof_t nbytes;
	struct fragment **rem;
{
	register struct fragment *new, *newnext, *newlimit;
	auto char *endofmem;

	/* 
	 * Find the current data segment limit.  Here is where code to
	 * handle other than a monotonically increasing data segment break
	 * would go.  The assertion catches such breaks.
	 */
	new = heaplink(getendofmem());
	assert(new > hp->limit, hp);

	nbytes += sizeof *new;	/* add in heaplink overhead */

	/*
	 * If the heap limit guards the end of memory, check for a free
	 * fragment just before it.  If there is one, use it as the
	 * beginning of new space; otherwise use the heap limit itself.
	 * If the heap limit guards a break in the data segment which this
	 * heap did not create, link the heap limit to the new space,
	 * guarding the hole.
	 */
	assert(isguard(hp->limit), hp);
	if (hp->limit + 1 == new) {
		register struct fragment *prev = prevfrag(hp->limit);

		if (isfree(prev)) {
			assert(distance(prev,new)-sizeof(*hp->limit) < nbytes,
			    hp);
			if (moremem(nbytes - distance(prev, new)
				    + sizeof *newlimit, &endofmem) == NULL) {
				new = NULL;
			} else {
				/* we are taking a free frag */
				--hp->freefrags;
				new = prev;
			}
		} else {
			if (moremem(nbytes, &endofmem) == NULL) {
				new = NULL;
			} else {
				new = hp->limit;
				CLRGUARD(new);
			}
		}
	} else {
		new = heaplink(moremem(nbytes + sizeof *newlimit, &endofmem));
		if (new != NULL)
			LINKTOFREE(hp->limit, new);
	}

	/*
	 * Set newlimit to point to a new guard at the end of memory.
	 * Set newnext to point to free space after the requested piece.
	 * If newnext is big enough, link it into hp as a free piece.
	 * Return in *rem the next heap fragment after new.
	 */
	newlimit = heaplink(endofmem) - 1;
	if (new != NULL) {
		assert(nbytes+sizeof(*newlimit) <= endofmem-address(new), hp);
		newnext = advance(new, nbytes);
		/*
		 * ``split'' operation w/o rover update (done in heap_alloc).
		 */
		if (distance(newnext, newlimit) >= hp->minsize) {
			/* count newnext as a free frag */
			hp->freefrags++;
			LINKTOFREE(new, newnext);
			LINKTOGUARD(newnext, newlimit);
			*rem = newnext;
		} else {
			LINKTOGUARD(new, newlimit);
			*rem = newlimit;
		}
	}

	LINK(newlimit, hp->base);	/* update heap limit and lowfree */
	if (hp->lowfree == hp->limit) {
		hp->lowfree = *rem;
	}
	hp->limit = newlimit;
	return new;
}
