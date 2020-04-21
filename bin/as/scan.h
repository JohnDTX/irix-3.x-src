		/*	scan.h	1.2	82/09/15	*/
#include <stab.h>

/* stab symbol table bucket - YIP */
struct stab_sym_bkt {
         char                *ch;        /* string name   */
         unsigned char       type;       /* field "type"  */
         char                other;      /* field "other" */
         short               desc;       /* field "desc"  */
         unsigned long       value;      /* field "value" */
         short               id;         /* character count of string */
         short               tag;        /* STABFLOATING/STABFIX */
         struct sym_bkt	     *label;	 /* pointer to the label symbol */
         struct stab_sym_bkt *next_stab;
};

                                    /* YIP */
/* bootstrap problem -- no long names yet */
# define stabkt_head	sBKTHEAD
# define stabkt_tail	sBKTAIL
struct stab_sym_bkt *stabkt_head,   /* head of stab symbol bucket linked list */
                    *stabkt_tail;   /* tail of stab symbol bkcket linked list */

