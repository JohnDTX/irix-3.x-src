main()
{
    unsigned short k;

    ginit();
    while(1) {
	color(k++ % 7);
	recti(rand()%1024,rand()%768,rand()%1024,rand()%768);
    }
}
