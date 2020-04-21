char _Origin_[] = "Unisoft Systems";

#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <stdio.h>

/*
 *	sumtree [directories]
 *		sum all files in the tree
 *	parameter
 *		directory - root directory of tree to be sum
 */

#define ifdir(s)	(((s).st_mode&S_IFMT)==S_IFDIR)
#define ifreg(s)	(((s).st_mode&S_IFMT)==S_IFREG)

/*********	START OF CODE	*********/
main(argc, argv)
char **argv;
{
	register char *p;

	if (argc < 2)
		check(".");
	else while (--argc)
		check(*++argv);
	exit(0);
}

check(file)
char *file;
{
	struct stat sb;

	if (stat(file, &sb) < 0) {
		fprintf(stderr, "cannot stat %s\n", file);
		return;
	}
	if (ifdir(sb))
		search(file);
	else if (ifreg(sb))
		print(file);
	else
		fprintf(stderr, "special file   %s\n", file);
}

search(d)
char *d;
{
	char file[512];
	DIR *fp;
	register struct dirent *b;

	if ((fp = opendir(d)) == 0) {
		fprintf(stderr, "cannot open directory %s\n", d);
		return;
	}
	while ((b = readdir(fp)) != 0) {
		if (b->d_ino && !dotdot(b->d_name)) {
			sprintf(file, "%s/%s", d, b->d_name);
			check(file);
		}
	}
	closedir(fp);
}

dotdot(s)
char *s;
{
	if (*s++ != '.')
		return (0);
	if (*s == 0 || (*s++ == '.' && *s == 0))
		return (1);
	return (0);
}

print(file)
char *file;
{
	long cnt;
	unsigned int sum;
	int fp;
	char cbuf[BUFSIZ];
	register int i, j;

	if ((fp = open(file, 0)) < 0) {
		fprintf(stderr, "cannot open %s\n", file);
		return;
	}
	sum = 0;
	cnt = 0;
	while ((j = read(fp, cbuf, BUFSIZ)) > 0) {
		cnt += j;
		for (i = 0; i < j; i++) {
			if (sum&01)
				sum = (sum>>1) + 0x8000;
			else
				sum >>= 1;
			sum += cbuf[i] & 0xFF;
			sum &= 0xFFFF;
		}
	}
	printf("%.5u %8ld %s\n", sum, cnt, file);
	close(fp);
}
