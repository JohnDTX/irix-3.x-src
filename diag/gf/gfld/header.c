/* header.c  -- little file to print the version number

4.7:  includes SD, id, gB, cT
4.8:  fixed GE interrupts
4.9:  automatic default delaycount setting by i commands
	ii turns on interrupts for G commands
	Gb (was G)
	Gd
4.10: minor repair
4.11: fixed "rl" and "rd"
5.0:  fixed after massive rot; added ft<n> tests
5.1:  more rot fixes. 't' still draws funny rect.
5.2:  fixed "bt"
5.3:  fixed "Gb"
 */

header()
{
	printf("GE/FBC CONSOLE V5.3\n   changes:\n");
	printf("fixed  Gb,  bt.\n");
	printf("ft<n> where n selects test pattern.\n");
}
