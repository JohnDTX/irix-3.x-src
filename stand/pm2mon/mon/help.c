help_help(n)
    int n;
{
    colprint("h(elp) [CMD]","print help info [for CMD if present]");
}



extern char z[];


colprint(a,b)
    char *a,*b;
{
    printf("  %-24s - %s",a,b);
    newline();
}
scolprint(a,b)
    char *a,*b;
{
    printf("    %-22s - %s",a,b);
    newline();
}
noteprint(a,b)
    char *a,*b;
{
    printf("%-8s%s",z,a);
    newline();
}
