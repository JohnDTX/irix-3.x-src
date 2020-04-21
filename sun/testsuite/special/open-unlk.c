/*
 *  tests operation on open file which has been unlinked.
 *  steps taken:
 *	1.  create file
 *	2.  open for read/write
 *	3.  unlink file
 *	4.  write data
 *	5.  rewind
 *	6.  read data back
 *
 * #define DEBUG to get debug output 
 */

#include <stdio.h>
#include <sys/file.h>
#include <errno.h>
extern errno;
#define TBUFSIZ 100
char wbuf[TBUFSIZ], rbuf[TBUFSIZ];
#define TMSG "This is a test message written to the unlinked file\n"

main(argc, argv)
int argc;
char *argv[];
{
	int fd, ret;
	char *tname = "nfstestXXXXXX";
	int errcount = 0;

	setbuf(stdout, NULL);
	mktemp(tname);
#ifdef O_RDWR
	if ((fd = open(tname, O_CREAT|O_TRUNC|O_RDWR, 0777)) < 0) {
		fprintf(stderr, "can't create %s: ", tname);
		pxit(1, "open");
	}
#else
	if ((fd = creat(tname, 0777)) < 0) {
		fprintf(stderr, "can't create %s: ", tname);
		pxit(1, "creat");
	}
	close(fd);
	if ((fd = open(tname, 2)) < 0) {
		fprintf(stderr, "can't reopen %s: ", tname);
		unlink(tname);
		pxit(1, "open");
	}
#endif O_RDWR
#ifdef DEBUG
	printf("nfsjunk files before unlink:\n  ");
	system("ls -l .nfsjunk*");	/* Sun implementation-specific */
#endif DEBUG
	ret = unlink(tname);
	printf("%s open; unlink ret = %d\n", tname, ret);
	if (ret)
		pxit(1, " unlink");
#ifdef DEBUG
	printf("nfsjunk files after unlink:\n  ");
	system("ls -l .nfsjunk*");
#endif DEBUG
	strcpy(wbuf, TMSG);
	if ((ret = write(fd, wbuf, TBUFSIZ)) != TBUFSIZ) {
		fprintf(stderr, "write ret %d; expected %d\n", ret, TBUFSIZ);
		pxit(ret < 0, " write");
	}
	if ((ret = lseek(fd, 0, 0)) != 0) {
		fprintf(stderr, "lseek ret %d; expected 0\n", ret);
		pxit(ret < 0, " lseek");
	}
	if ((ret = read(fd, rbuf, TBUFSIZ)) != TBUFSIZ) {
		fprintf(stderr, "read ret %d; expected %d\n", ret, TBUFSIZ);
		pxit(ret < 0, " read");
	}
	if (strcmp(wbuf, rbuf) != NULL) {
		errcount++;
		printf("read data not same as written data\n");
		printf(" written: '%s'\n read:    '%s'\n", wbuf, rbuf);
	}
#ifdef DEBUG
	else {
		printf("data compare ok\n");
	}
#endif DEBUG

	if (unlink(tname) == 0) {
		errcount++;
		printf("Error: second unlink succeeded!??\n");
	} else if (errno != ENOENT) {
		errcount++;
		perror("unexpected error on second unlink");
	}

	if (ret = close(fd)) {
		errcount++;
		perror("error on close");
	}

#ifdef DEBUG
	printf("nfsjunk files after close:\n  ");
	system("ls -l .nfsjunk*");
#endif DEBUG

	if ((ret = close(fd)) == 0) {
		errcount++;
		fprintf(stderr, "second close didn't return error!??\n");
	}

	if (errcount == 0)
		printf("Test completed successfully.\n");
	else
		printf("Test failed with %d error%s.\n", errcount,
			errcount == 1? "" : "s");
	exit(errcount);
}

pxit(pflg, s)
int pflg;
char *s;
{
	if (pflg)
		perror(s);
	fprintf(stderr, "Test failed.\n");
	exit(1);
}
