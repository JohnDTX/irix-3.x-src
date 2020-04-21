#include "gl.h"
#include "device.h"

main()
{
	short namebuffer[50];
	long numpicked;
	short type, val, i, j, k;

	ginit();

	qdevice(MOUSE1);
	qdevice(MOUSE2);

	makeobj(1);
		color(RED);
		loadname(1);
		rectfi(20,20,100,100);
		loadname(2);
		pushname(3);
		circi(50,500,50);
		loadname(4);
		circi(50,530,60);
	closeobj();

	color(BLACK);
	clear();
	callobj(1);

	while (1) {
		type = qread(&val);
		if (val == 0)
			continue;
		switch (type) {
		case MOUSE1:
			gexit();
			exit(0);
		case MOUSE2:
			pick(namebuffer, 50);
			ortho2(-0.5, XMAXSCREEN + 0.5, -0.5, YMAXSCREEN + 0.5);
			callobj(1);
			numpicked = endpick(namebuffer);
			printf("hits: %d; ",numpicked);
			j = 0;
			for (i = 0; i < numpicked; i++) {
				printf(" ");
				k = namebuffer[j++];
				printf("%d ", k);
				for (;k; k--)
					printf("%d ", namebuffer[j++]);
				printf("|");
			}
			printf("\n");
		}
	}
}
