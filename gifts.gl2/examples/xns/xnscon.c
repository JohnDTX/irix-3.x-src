#include <stdio.h>

main(ac,av)
int ac;
char **av;
{
	int sockno,fd,rval,cnt;
	char *hostname;
	char line[80];
	ac--;
					/* process arguments */
	if(ac == 0){
		printf("usage: %s hostname socket \n",*av);
		exit(1);
	}else{
		*av++;
		hostname = *av;
		ac--;*av++;
		if(ac == 0)sockno = 100;
		else{
			sockno = atoi(*av);
					/* these are good numbers to use */
			if((sockno < 100) || (sockno > 2000)){
				printf("socket(%d) out of range, 100 used\n",sockno);
				sockno = 100;
			}
		}

					/* connect to host */
	if((fd = xnsconnect(hostname,sockno)) == -1){
		printf("xnsconnect of %s on %d failed\n",hostname,sockno);
		exit(1);
	}
					/* now write to the socket */

	printf(" - connected -\n");
				/* while there is input read it */
	while((cnt = read(0,line,80))>0){
				/* some data massaging */
		printf("sending <");
		fflush(stdout);
		write(1,line,cnt-1);
		printf("\\n>\n");
		fflush(stdout);
				/* write to socket */
		if(write(fd,line,cnt) < 0){
			perror("write");
			exit(0);
		}
	}
	printf("DONE\n");
				/* clean up */
	close(fd);
	}
}
