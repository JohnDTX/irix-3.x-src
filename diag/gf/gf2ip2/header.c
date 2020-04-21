/* header.c  -- little file to print the version number

1.0	Initial version
1.1	Lots of fixes
1.2	separated out more gl2 sources -- fixed b tests
1.3	fixed operation with GE passers
1.4	repaired bustests.  Why doesn't gtb work?
1.5     fixed hanging on 't' with passers
1.6	improved gm; added gV for Multibus testvectors (like gv)
1.7	^C escape from macro loops; "rs" prints exp'd values; gm fixes
1.8     ip2 version
 */

header()
{
	printf("GF2/IP2/UC4/DC4 CONSOLE V1.8\n\n   changes:\n");

	printf("BUGZ:  't' and 'bt' produce sporadic errors.\n");
	printf("EPROM version 1.4 kluge: >emb 3500000b   change to 0f\n");
	printf("use ^C to escape macro loops!\n");

	printf("\n\n");
}
