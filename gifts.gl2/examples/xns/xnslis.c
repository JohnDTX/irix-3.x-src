main(ac,av)
int ac;
char **av;
{
	int sockno,fd,cnt;
	char buf[80];
	ac--;
	if(ac == 0){
		printf("usage: %s socket\n",*av);
		exit(1);
	}else{
		*av++;
		sockno = atoi(*av);
				/* these are good numbers to use */
		if((sockno < 100) || (sockno > 2000)){
			printf(" socket(%d) out if range, 100 used\n",sockno);
			sockno = 100;
		}
	}

				/* listen till someone tries to connect. */
	if((fd = xnslisten(sockno)) == -1){
		printf("xnslisten on %d failed\n",sockno);
		exit(1);
	}
	printf(" - connection established - \n");
				/* now read from the socket */
	while((cnt = read(fd,buf,80)) > 0){
		write(1,buf,cnt);
	}
	printf("DONE\n");
				/* clean up */
	close(fd);
}
