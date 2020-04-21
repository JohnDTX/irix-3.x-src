# define FSBShift		10
# define FSBSize		(1<<FSBShift)
# define FSBMask		(FSBSize-1)
# define AddrsPerInode		13
# define DirectAddrsPerInode	10
# define IndirectAddrsPerInode	3
# define AddrShift		4
# define AddrsPerFSB		(1<<AddrsPerFSBShift)
# define AddrsPerFSBShift	(FSBShift-AddrShift)
# define AddrsPerFSBMask	(AddrsPerFSB-1)
# define InodeShift		6
# define InodesPerFSBShift	(FSBShift-InodeShift)
# define InodesPerFSB		(1<<InodesPerFSBShift)


struct bell_toyfs
{
    struct filsys filsys;
};

struct bell_toyinode
{
    struct dinode inode;
    daddr_t daddrs[AddrsPerInode];
};
