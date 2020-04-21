# include "pmII.h"
# define IKONBASE	0x70C0

/*
 * we don't want to boot from it.
 */
IkonInit()
{
    register char *ip;
    char c;

    ip = (char *)mbiotov(IKONBASE);

    if( !probe(ip,1) ) return;

    /* just disable interrupts */
    ip[(0x8)] = 0x12;
    ip[(0xA)] = 0;
    ip[(0xA)] = 0xFF;

    /* reset in case one was pending */
    ip[(0xC)] = 0x8;
    c = ip[(0x8)];
}
