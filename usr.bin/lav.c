char _Origin_[] = "UC Berkeley";
static	char	*Sccsid = "@(#)$Header: /d2/3.7/src/usr.bin/RCS/lav.c,v 1.1 89/03/27 17:40:21 root Exp $";
/*
 * $Log:	lav.c,v $
 * Revision 1.1  89/03/27  17:40:21  root
 * Initial check-in for 3.7
 * 
 * Revision 1.2  85/03/11  13:26:34  bob
 * Changed to work with vmunix.
 * 
 */

#include <stdio.h>
#include <nlist.h>

#define fxtod(i)	(((double)vec[i])/1024.0)

/*
 * Namelist for getting load average info out of kernel
 */
struct nlist    nl[] = {
        { "_avenrun" },
        { "" },
};
int kmem;

/*
 * print the load average on a falco
 */
main(argc, argv)
int argc;
char *argv[];
{
        long vec[3];

    /* get address of _avenrun variable in kernel */

        nlist("/vmunix", nl);
        if (nl[0].n_type==0) {
                fprintf(stderr, "No namelist\n");
                exit(1);
        }
        if ((kmem = open("/dev/kmem", 0)) == -1) {
                fprintf(stderr, "Can't open /dev/kmem\n");
                exit(1);
        }

    /* print out load average */

        loadav(vec);
        printf("lav: %.2f %.2f %.2f\n", fxtod(0), fxtod(1), fxtod(2));
}

loadav(v)
long v[3];
{
        lseek(kmem, (long)nl[0].n_value, 0);
        read(kmem, v, 3*sizeof(long));
}
