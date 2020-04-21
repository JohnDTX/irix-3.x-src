int
md_open(ext,file)
    char *ext,*file;
{
    extern int mdinit(),mdrblk(),mdclear();

    return disk_open(mdinit,mdrblk,mdclear,ext,file);
}
