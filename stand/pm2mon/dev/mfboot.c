int
mf_open(ext,file)
    char *ext,*file;
{
    extern int mfinit(),mfrblk(),dummy();

    return disk_open(mfinit,mfrblk,dummy,ext,file);
}
