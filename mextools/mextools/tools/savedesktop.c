/*
 *	savedesktop - 
 *		Save a few color map entries in ~/.desktop . These colors 
 *		will be mapped for you by makemap.
 *
 *				Paul Haeberli - 1984
 *
 */
main()
{
    noport();
    winopen("savedesktop");
    savecolors();
    gexit();
}
