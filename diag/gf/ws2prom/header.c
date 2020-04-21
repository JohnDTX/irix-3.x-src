/* header.c  -- little file to print the version number

1.0	Initial version
1.1	fewer spaces in verbose;
	tests 8 swap patterns;
	tallies microram test errors
1.2	new microcode broke polygons.  Changed circle draw to 2 red squares
1.3	fixed dummy-GE mode
 */

extern short Verbose;

header()
{
	if (Verbose) printf("   gf test V1.3");
}
