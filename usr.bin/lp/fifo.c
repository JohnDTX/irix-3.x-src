/* enqueue(cmd, args) -- writes command and args for scheduler on FIFO */

#include	"lp.h"

SCCSID("@(#)fifo.c	3.1")

static FILE *fifo = NULL;

static int
sig14()
{
	signal(SIGALRM, sig14);
}

enqueue(cmd, args)
char cmd;
char *args;
{
	int i = 0;
	unsigned alarm();

	if(eaccess(FIFO, ACC_W) == 0) {
		if(fifo == NULL) {
			sig14();
			alarm(ALTIME);
			fifo = fopen(FIFO, "w");
			alarm(0);
		}
		if(fifo != NULL) {
			alarm(ALTIME);
			i = fprintf(fifo, "%c %s\n", cmd, args);
			alarm(0);
			if(i != 0)
				fflush(fifo);
			else
				fifo = NULL;
		}
	}
	else
		fifo = NULL;

	return(fifo != NULL);
}
