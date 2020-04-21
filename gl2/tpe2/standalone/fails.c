short buf[20];

main()
{
    int i;

    buf[0] = 8; buf[1] = 1;
    ginit();
    while (1) gewrite(buf,2);
}
