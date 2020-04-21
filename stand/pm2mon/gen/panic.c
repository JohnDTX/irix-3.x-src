panic(str)
    char *str;
{
    printf("%s\n",str);
    delayed_reboot();
    warmboot();
}
