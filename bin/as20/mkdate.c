#include <stdio.h>
#   include <time.h>
#ifndef VERSION
#define VERSION "NONE"
#endif

main()
{
    struct tm *t;
    long clock;
    char name[100];
    int namelen;

    printf("char *id = \"as20 ");
    DoVersionNumber();
    printf(" of ");
    clock = time(0);
    t = localtime(&clock);
    printf("%d/%d/%d ", t->tm_mon + 1, t->tm_mday, t->tm_year % 100);
    printf("%d:%02d", t->tm_hour, t->tm_min);
    printf("\\n\";\n");
    exit(0);
}

DoVersionNumber()
{
    printf("version %s",VERSION);
}
