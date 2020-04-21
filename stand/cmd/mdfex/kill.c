/*
** kill.c	- Copyright (C) Silicon Graphics - Mountain View, CA 94043
**		- Author: chase
**		- Date: April 1985
**		- Any use, copy or alteration is strictly prohibited
**		- and gosh darn awfully -- morally inexcusable
**		- unless authorized by SGI.
**
** $Author: root $
** $State: Exp $
** $Source: /d2/3.7/src/stand/cmd/mdfex/RCS/kill.c,v $
** $Revision: 1.1 $
** $Date: 89/03/27 17:12:50 $
*/

#include <sys/types.h>
#include <sys/dklabel.h>
#include "disk.h"
#include "dsdreg.h"
#include "fex.h"

kill()
{
	register i;
	register int ns, maxlba, lba;
	int data;
	register char *cp;

	screenclear();
	printf("Kill all data on disk unit %d\n", dunit);
	printf("\t*** First  Pass: Write all ones\n");
	printf("\t*** Second Pass: Write all zeros\n");
	printf("\t*** Third  Pass: Write all alternating ones\n");
	printf("\t*** Fourth Pass: Write all alternating zeros\n");
	drivep = & drives[dunit];

	if(!isinited[dunit]) {
		if(initl())
			return;
	} else if(config())
		return;
	printf("Type 'go<return>' to start...");
	cp = getline();
	if(!uleq(cp, "go")) {
		printf(" Aborted.\n");
		return;
	}
	if(!quiet)
		printf("\nStarting kill...\n");

	ns = SEC;
	maxlba = CYL*HD*ns;
	data = 0xffffffff;
    	for(lba = 0; lba < maxlba; lba += ns) {
		filll((long *)BUF0, data, (int)(ns*(secsize/4)));
		if (wdata(ns, lba, (char *)BUF0)) {
			printf("Write Err @"); pb1(lba, 1, 0);
			printf("\n");
			return;
		}
		if((i = nwgch()) != -1) {
			if (i == 'q') {
				printf("Aborted\n");
				return;
			}
		}
		if((lba % 10) == 0) QP1("%4d ", lba);
	}
	printf("\nFirst Pass complete!\n");

	data = 0;
    	for(lba = 0; lba < maxlba; lba += ns) {
		filll((long *)BUF0, data, (int)(ns*(secsize/4)));
		if (wdata(ns, lba, (char *)BUF0)) {
			printf("Write Err @"); pb1(lba, 1, 0);
			printf("\n");
			return;
		}
		if((i = nwgch()) != -1) {
			if (i == 'q') {
				printf("Aborted\n");
				return;
			}
		}
		if((lba % 10) == 0) QP1("%4d ", lba);
	}
	printf("\nSecond Pass complete!\n");

	data = 0x55555555;
    	for(lba = 0; lba < maxlba; lba += ns) {
		filll((long *)BUF0, data, (int)(ns*(secsize/4)));
		if (wdata(ns, lba, (char *)BUF0)) {
			printf("Write Err @"); pb1(lba, 1, 0);
			printf("\n");
			return;
		}
		if((i = nwgch()) != -1) {
			if (i == 'q') {
				printf("Aborted\n");
				return;
			}
		}
		if((lba % 10) == 0) QP1("%4d ", lba);
	}
	printf("\nThird Pass complete!\n");

	data = 0xAAAAAAAA;
    	for(lba = 0; lba < maxlba; lba += ns) {
		filll((long *)BUF0, data, (int)(ns*(secsize/4)));
		if (wdata(ns, lba, (char *)BUF0)) {
			printf("Write Err @"); pb1(lba, 1, 0);
			printf("\n");
			return;
		}
		if((i = nwgch()) != -1) {
			if (i == 'q') {
				printf("Aborted\n");
				return;
			}
		}
		if((lba % 10) == 0) QP1("%4d ", lba);
	}
	printf("\nFourth Pass complete!\n");

	printf("Kill of drive %d complete!\n", dunit);
	dontupdate = 1;
}
