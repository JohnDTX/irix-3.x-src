/*  ps.h - header for pseudo-op processing */

/*  to use a common lookup routine, all hash chain entries
    must have the comparison name as the first element and
    the 'next pointer' as the second element */

typedef struct pstab_s {
		/* the ascii string naming the pseudo-op..
		   for entry into a hash table
		*/
		char *name;
		/* for linking within a bucket in the
		   hash table
		*/
		struct pstab_s *next;
		/* pointer to function to 
		   process the pseudo-op 
		*/
		int (*psfunc)();
		/* two arguments to pass to the function */
		int arg0,arg1;
		} pstab_t ;

/* the pseudo-ops are hashed into a table with NPSBUCKETS buckets */
#define NPSBUCKETS 10
pstab_t *ps_bucket[NPSBUCKETS];

/*  NOTE!! this is replicated in ps.h ***/
struct op_immabs
{
	/* set if this symbolic datum cant be left as relocatable
	   in the output file.  Only positive symbols are relocatable.
	*/
	short isnrexpr,isunsigned;
	struct loc_s * sym;
	/* if the sym structure is not null, the
		  address is an offset
	*/
	long addr;
} ;

/* pointer to alternate string, for data entered by a pseudo-op
   which is too big to fit in the code array.
*/
char *psstr;
