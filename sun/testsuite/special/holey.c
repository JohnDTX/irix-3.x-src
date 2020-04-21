/*
 * test read/write of holey files
 *
 * writes data blocks (of size datasz) into files, with seeks of seeksz
 * between writes.  This creates a file with "holes" in it.  On reads,
 * these holes should be read as zeroes.  There is a bug in some
 * implementations which cause a premature EOF if a seek crosses
 * a block boundary (when run on an NFS client).
 */
#include <sys/param.h>
#ifndef major
#include <sys/types.h>
#endif
#include <sys/stat.h>
#include <stdio.h>
#include <sys/file.h>

#define BUFSZ   8192
#define FILESZ	70000

#ifdef works_ok
#define DATASZ	4321
#define HOLESZ	9012
#else fails
#define DATASZ	2048
#define HOLESZ	2048
#endif

#define FILENM  "holeyfile"
int Debug = 0;
char *Prog;

main(argc, argv)
int argc;
char *argv[];
{
	int fd, i, tot, ct, sz, bytes, ret;
	char *filenm = FILENM;
	char buf[BUFSZ];
	extern int errno;
	int filesz = FILESZ;
	int datasz = DATASZ;
	int holesz = HOLESZ;

	Prog = argv[0];
	if (argc > 1 && !strcmp(argv[1], "-d")) {
		Debug = 1;
		argc--;
		argv++;
	}
	if (argc > 5 || (argc > 1 && !strcmp(argv[1], "-h"))) {
		fprintf(stderr, "\
usage: %s [filename filesize datasize holesize]\n", Prog);
		exit(1);
	}
	if (argc > 1 && strcmp(argv[1], "-"))
		filenm =       argv[1];
	if (argc > 2 && strcmp(argv[2], "-"))
		filesz =  atoi(argv[2]);
	if (argc > 3 && strcmp(argv[3], "-"))
		datasz =  atoi(argv[3]);
	if (argc > 4 && strcmp(argv[4], "-"))
		holesz =  atoi(argv[4]);

	umask(0);
	if (datasz > BUFSZ) {
		fprintf(stderr, "%s (fail): datasize (%d) greater than maximum (%d)\n",
			Prog, datasz, BUFSZ);
		exit(1);
	}
	if ((fd = creat(filenm, 0666)) < 0) {
		sprintf(buf, "%s (fail): creat of %s", Prog, filenm);
		perror(buf);
		exit(1);
	}
	if (close(fd)) {
		sprintf(buf, "%s (fail): close of %s after creat", Prog, filenm);
		perror(buf);
		exit(1);
	}
	if ((fd = open(filenm, 2)) < 0) {
		sprintf(buf, "%s (fail): open of %s", Prog, filenm);
		perror(buf);
		exit(1);
	}
	for (i=0; i < BUFSZ / sizeof(int); i++)
		((int *)buf)[i] = i;

	for (sz = filesz; sz; ) {
		if (datasz || sz == 1) {
			bytes = MIN(sz, datasz);
			if (bytes == 0)
				bytes = 1;
			if ((ret = write(fd, buf, bytes)) != bytes) {
				fprintf(stderr, "write ret %d (expect %d)\n",
					ret, bytes);
				if (errno) perror("write");
				exit(1);
			}
			sz -= bytes;
		}
		if (sz && holesz) {
			bytes = MIN(sz - 1, holesz);
			if (lseek(fd, bytes, L_INCR) == -1) {
				perror("lseek (write)");
				exit(1);
			}
			sz -= bytes;
		}
	}
	if (lseek(fd, 0, L_SET) == -1) {
		perror("lseek (rewind)");
		exit(1);
	}

	for (sz = filesz; sz; ) {
		if (datasz || sz == 1) {
			bytes = MIN(sz, datasz);
			if (bytes == 0)
				bytes = 1;
			sz -= bytes;
			for ( ; bytes; bytes -= ret) {
				if (Debug) 
					fprintf(stderr, "\
--data read: offset %d, sz = %d, bytes = %d\n", filesz - sz - bytes, sz, bytes);
				if ((ret = read(fd, buf, bytes)) <= 0) {
					fprintf(stderr, "\
fail: read (data) offset %d, sz = %d, bytes = %d (ret = %d), datasz = %d\n",
filesz - sz - bytes, sz, bytes, ret, datasz);
					if (ret < 0)
						perror("read");
					exit(1);
				}
				ct = bytes - (bytes % sizeof(int));
				if (Debug)
					fprintf(stderr, "\
  ret = %d, ct = %d\n", ret, ct);
				for (i=0; i < ct / sizeof(int); i++) {
					if (((int *)buf)[i] != i) {
						fprintf(stderr, "\
fail: bad data in %s\n", filenm);
						exit(1);
					}
				}
			}
		}
		if (sz && holesz) {
			tot   = MIN(holesz, sz - 1);
			sz -= tot;
			for (ct = 0; tot; tot -= ret, ct += ret) {
				bytes = MIN(tot, BUFSZ);
				if (Debug)
					fprintf(stderr, "\
++hole read: offset %d, sz = %d, tot = %d, bytes = %d\n",
					filesz - sz - tot, sz, tot, bytes);
				if ((ret = read(fd, buf, bytes)) <= 0) {
					fprintf(stderr, "\
fail: read (hole) offset %d, sz = %d, bytes = %d (ret %d), holesz = %d\n",
filesz - sz - tot, sz, bytes, ret, holesz);
					if (ret < 0)
						perror("read");
					exit(1);
				}
				if (Debug)
					fprintf(stderr, "  ret = %d\n", ret);
				for (i = 0; i < ret; i++) {
					if (buf[i] != '\0') {
						fprintf(stderr, "\
fail: non-zero data read back from hole (offset %d)\n", filesz - sz + ct + i);
						exit(1);
					}
				}
			}
		}
	}
	printf("\nHoley file test ok\n");
	exit(0);
}
