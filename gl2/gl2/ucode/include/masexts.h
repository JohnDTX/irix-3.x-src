/* masexts.h
 *
 */

extern short state;		/* state counter			*/
extern short filestate;	/* ditto for each user code file	*/
extern short bracket;		/* bracket matching flag		*/
extern char *nexts[MAXSTATES];	/* storage for unresolved labels	*/
extern char NextAssigned[MAXSTATES];
extern char *Assigned;
extern char *procname;     /* for passing procedure name for error rept */
extern Output NextAddress;

extern Output *fieldlist[MAXFIELDS];
extern short flp;
