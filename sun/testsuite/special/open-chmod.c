/*
 *  tests operation on open file which has been chmod'd to 0.
 *  steps taken:
 *	1.  create file
 *	2.  open for read/write
 *	3.  chmod 0
 *	4.  write data
 *	5.  rewind
 *	6.  read data back
 *
 * #define DEBUG for debug output
 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <errno.h>
extern errno;
#define TBUFSIZ 100
char wbuf[TBUFSIZ], rbuf[TBUFSIZ];
char buf[BUFSIZ];
#define TMSG "This is a test message written to the chmod'd file\n"

main(argc, argv)
int argc;
char *argv[];
{
	int fd, ret;
	char *tname = "nfstestXXXXXX";
	int errcount = 0;
	struct stat sbuf;

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
	printf("testfile before chmod:\n  ");
	sprintf(buf, "ls -l %s", tname);
	system(buf);
#endif DEBUG
	ret = chmod(tname, 0);
	printf("%s open; chmod ret = %d\n", tname, ret);
	if (ret)
		pxit(1, " chmod");
#ifdef DEBUG
	printf("testfile after chmod:\n  ");
	system(buf);
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
	if ((ret = fstat(fd, &sbuf)) < 0)
		pxit(1, " fstat");
	if (sbuf.st_size != TBUFSIZ) {
		fprintf(stderr, "fstat size %d; expected %d\n",
			sbuf.st_size, TBUFSIZ);
		pxit(0, "");
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

	printf("testfile after write/read:\n  ");
	system(buf);
#endif DEBUG
	if (unlink(tname) < 0) {
		fprintf(stderr, "can't unlink %s", tname);
		pxit(1, " unlink");
	}

	if (close(fd))
		pxit(1, "error on close");

	if (errcount == 0)
		printf("test completed successfully.\n");
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
