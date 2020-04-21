# include "common.h"
/*
 * prom / standalone test for auto reboot feature.
 */

main()
{
    int argc; char **argv;
    printf("booted as: %s\n",_commdat->bootstr);
    printf("autoboot from: ");
    readargs(argc,&argv);
    strcpy(_commdat->bootstr,argv[0]);
    _commdat->reboot = MAGIC_REBOOT_VALUE;
    warmboot();
}
