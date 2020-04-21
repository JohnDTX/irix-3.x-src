/* remap.c -- map Multibus memory for ALPHA boards */

#ifdef DEVEL
/* just FBC installed at 1e8000	*/
#define LOWADR	0xc8000
#define HIGHADR	0xe0000
#else
/* Alpha FBC, DC2 installed at 0x180000, 0x104000 */
#define LOWADR	0x4000
#define HIGHADR	0x20000
#endif

#include "pcmap.h"

#define ADR_TO_PAGE(n)	(((n)&0xfffff)>>11)

remap ()
{
int i;

printf("mapping Multibus\n");
for (i = LOWADR; i < HIGHADR; i+=PAGESIZE)
    SETPAGEMAP(0x200 + ADR_TO_PAGE(i), PGSPC_MBMEM + ADR_TO_PAGE(i) );
}
