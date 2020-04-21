#
/*
 * rib.c --
 * reset ib (as super-user).
 * this program should be hidden where only
 * authorized users can execute it.
 */
# define ROOTUID	0
char *IIB = "/etc/iib";

main(rgc,rgv)
    int rgc;
    char **rgv;
{
    setuid(ROOTUID);
    rgv[0] = IIB;
    execv(IIB,rgv);
}
