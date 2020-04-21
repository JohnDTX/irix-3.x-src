/*
 * Structure of the system call function table
 */
struct	sysent {
	int	(*sy_call)();		/* handler */
};
extern	struct sysent sysent[];
