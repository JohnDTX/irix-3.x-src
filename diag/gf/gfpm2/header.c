/* header.c */
/*
4.7:  added gB<lowGE> <highGE> <preamblewords> - better vector testing
	works with gT
	added cT
4.8:  Q command for GE confidence test that runs w/o dumb terminal
4.9:  fixed GE interrupt problem; optimized gB even more.
4.10: expanded G tests (boxes, dots)
	ii command to init with interrupts on
4.11: minor tweak to "gf" command
4.12: fixed "rl"... remade in /oh4/hdwr/diag/gf/ws/ directory
*/

header()
{
    printf("GE/FBC CONSOLE V4.12\n\n    changes: ");

    printf("   'rl' fixed\n");
    printf("   NOTE: use gd2 to check level 4 interrupts\n");

    putchar('\n');
}
