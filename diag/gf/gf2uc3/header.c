/* header.c  -- little file to print the version number

1.0	Initial version
1.1	Initial repairs
	added "fb"
	added "T" token flag test
1.2	added "if" (like "is" but does initmap() )
1.3	redone "gt" tests
1.4	added "B", "U" breakpoints
1.5	added gepa tests
1.6	SGEdata no longer hangs; gB can use interrupts (Si)

 */

header()
{
	printf("GF2 CONSOLE V1.6\n\n   changes:\n");

	printf("use 'S' instead of cs; Si for cT test!\n");
	printf("'gj' 'ga' for GA testing\n");
	printf("'gt' tests in order from 0\n");
	printf("'T' for token test\n");

	printf("\n\n");
}
