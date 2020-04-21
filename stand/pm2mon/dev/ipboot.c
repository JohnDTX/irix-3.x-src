int
ip_open(ext,file)
    char *ext,*file;
{
    extern int ipinit(),iprblk(),dummy();

    return disk_open(ipinit,iprblk,dummy,ext,file);
}
