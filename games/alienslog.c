char _Origin_[] = "UniSoft Systems";

#include <stdio.h>
#include <pwd.h>

#define ALIENSLOG "/usr/games/lib/alienslog"

main()
{
	int scores[8];
	register int fp, g;
	struct passwd *getpwuid();

	if ((fp = open(ALIENSLOG, 0)) < 0) {
		fprintf(stderr, "cannot open %s\n", ALIENSLOG);
		exit(1);
	}
	if (read(fp, scores, sizeof(scores)) != sizeof(scores)) {
		fprintf(stderr, "cannot read %s\n", ALIENSLOG);
		exit(1);
	}
	for (g = 1; g <= 4; g++)
		printf("game %d %8.8s %6d\n", g,
			getpwuid(scores[(g*2)-1])->pw_name, scores[(g*2)-2]);
	exit(0);
}
